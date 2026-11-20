#include "AuthService.h"
#include <QCryptographicHash>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>

namespace {
constexpr int kIterations = 150000;
QByteArray saltBytes() {
    QByteArray salt;
    salt.reserve(16);
    for (int i = 0; i < 16; ++i)
        salt.append(static_cast<char>(QRandomGenerator::system()->bounded(256)));
    return salt;
}
QByteArray keyFor(const QString &password, const QByteArray &salt) {
    return QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256,
                password.toUtf8(), salt, kIterations, 32);
}
bool equalDigest(const QByteArray &a, const QByteArray &b) {
    if (a.size() != b.size()) return false;
    unsigned int mismatch = 0;
    for (qsizetype i = 0; i < a.size(); ++i)
        mismatch |= static_cast<unsigned char>(a.at(i)) ^ static_cast<unsigned char>(b.at(i));
    return mismatch == 0;
}
}

bool AuthService::registerUser(const QString &username, const QString &email,
                               const QString &password, QString *error) {
    QString name = username.trimmed();
    QString mail = email.trimmed();
    static const QRegularExpression mailPattern(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");
    if (name.size() < 3 || name.size() > 50 || !mailPattern.match(mail).hasMatch() ||
        mail.size() > 100 || password.size() < 8) {
        if (error) *error = "Ten >= 3 ky tu, email hop le, mat khau >= 8 ky tu.";
        return false;
    }
    const QByteArray salt = saltBytes();
    const QByteArray key = keyFor(password, salt);
    QSqlQuery q;
    q.prepare(R"(INSERT INTO "USER"(UserName,UserEmail,PasswordHash,PasswordSalt,Role,Status)
                 VALUES(?,?,?,?,'USER','ACTIVE'))");
    q.addBindValue(name);
    q.addBindValue(mail);
    q.addBindValue(QString::fromLatin1(key.toHex()));
    q.addBindValue(QString::fromLatin1(salt.toHex()));
    if (!q.exec()) {
        if (error) *error = "Khong tao duoc tai khoan (ten/email co the da ton tai): " + q.lastError().text();
        return false;
    }
    return true;
}

Session AuthService::login(const QString &username, const QString &password, QString *error) {
    Session s;
    QSqlQuery q;
    q.prepare(R"(SELECT UserID,UserName,UserEmail,PasswordHash,PasswordSalt,Role,Status
                 FROM "USER" WHERE UserName=? OR UserEmail=? LIMIT 1)");
    q.addBindValue(username.trimmed());
    q.addBindValue(username.trimmed());
    if (!q.exec() || !q.next()) {
        if (error) *error = "Sai ten dang nhap hoac mat khau.";
        return s;
    }
    const QByteArray salt = QByteArray::fromHex(q.value(4).toByteArray());
    const QByteArray known = QByteArray::fromHex(q.value(3).toByteArray());
    const QByteArray attempted = keyFor(password, salt);
    if (!equalDigest(known, attempted)) {
        if (error) *error = "Sai ten dang nhap hoac mat khau.";
        return s;
    }
    if (q.value(6).toString() != "ACTIVE") {
        if (error) *error = "Tai khoan da bi khoa. Lien he quan tri vien.";
        return s;
    }
    s.userId = q.value(0).toInt();
    s.username = q.value(1).toString();
    s.email = q.value(2).toString();
    s.role = q.value(5).toString();
    return s;
}

bool AuthService::bootstrapAdminFromEnvironment(QString *error) {
    QSqlQuery q;
    if (!q.exec(R"(SELECT COUNT(*) FROM "USER" WHERE Role='ADMIN')") || !q.next()) {
        if (error) *error = "Khong the kiem tra Admin.";
        return false;
    }
    if (q.value(0).toInt() > 0) return true;
    const QString username = qEnvironmentVariable("PHONEBOOK_ADMIN_USER");
    const QString email = qEnvironmentVariable("PHONEBOOK_ADMIN_EMAIL");
    const QString password = qEnvironmentVariable("PHONEBOOK_ADMIN_PASSWORD");
    if (username.isEmpty() && email.isEmpty() && password.isEmpty()) return true;
    if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
        if (error) *error = "Can dat du 3 bien PHONEBOOK_ADMIN_USER, PHONEBOOK_ADMIN_EMAIL, PHONEBOOK_ADMIN_PASSWORD.";
        return false;
    }
    if (!registerUser(username, email, password, error)) return false;
    QSqlQuery update;
    update.prepare(R"(UPDATE "USER" SET Role='ADMIN' WHERE UserName=?)");
    update.addBindValue(username);
    if (!update.exec()) {
        if (error) *error = update.lastError().text();
        return false;
    }
    return true;
}
