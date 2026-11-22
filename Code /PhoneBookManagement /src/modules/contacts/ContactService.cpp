#include "ContactService.h"
#include <QDate>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>

namespace {
Contact fromQuery(const QSqlQuery &q) {
    Contact c;
    c.id = q.value(0).toInt();
    c.userId = q.value(1).toInt();
    c.fullName = q.value(2).toString();
    c.personalPhone = q.value(3).toString();
    c.workPhone = q.value(4).toString();
    c.personalEmail = q.value(5).toString();
    c.workEmail = q.value(6).toString();
    c.address = q.value(7).toString();
    c.gender = q.value(8).toString();
    c.dateOfBirth = q.value(9).toString();
    c.company = q.value(10).toString();
    c.occupation = q.value(11).toString();
    c.notes = q.value(12).toString();
    return c;
}
const QString cols = "ContactID, UserID, FullName, PersonalPhone, WorkPhone, "
    "PersonalEmail, WorkEmail, Address, Gender, DateOfBirth, Company, Occupation, Notes";
void bindContact(QSqlQuery &q, const Contact &c) {
    q.addBindValue(c.fullName.trimmed());
    q.addBindValue(c.personalPhone.trimmed());
    q.addBindValue(c.workPhone.trimmed());
    q.addBindValue(c.personalEmail.trimmed());
    q.addBindValue(c.workEmail.trimmed());
    q.addBindValue(c.address.trimmed());
    q.addBindValue(c.gender.trimmed());
    q.addBindValue(c.dateOfBirth.trimmed());
    q.addBindValue(c.company.trimmed());
    q.addBindValue(c.occupation.trimmed());
    q.addBindValue(c.notes.trimmed());
}
// Non-empty text match, ignoring case and outer spaces.
bool sameText(const QString &a, const QString &b) {
    return !a.trimmed().isEmpty() && a.trimmed().compare(b.trimmed(), Qt::CaseInsensitive) == 0;
}
// True if the two contacts share any phone number or any email.
bool sharesPhoneOrEmail(const Contact &a, const Contact &b) {
    for (const QString &x : {a.personalPhone, a.workPhone})
        for (const QString &y : {b.personalPhone, b.workPhone})
            if (sameText(x, y)) return true;
    for (const QString &x : {a.personalEmail, a.workEmail})
        for (const QString &y : {b.personalEmail, b.workEmail})
            if (sameText(x, y)) return true;
    return false;
}
}

bool ContactService::validate(const Contact &c, QString *error) {
    if (c.userId <= 0 || c.fullName.trimmed().isEmpty() || c.fullName.size() > 200) {
        if (error) *error = "Can co UserID hop le va FullName (toi da 200 ky tu).";
        return false;
    }
    if (c.personalPhone.size() > 20 || c.workPhone.size() > 20 ||
        c.personalEmail.size() > 100 || c.workEmail.size() > 100) {
        if (error) *error = "So dien thoai hoac email qua dai so voi Database Schema.";
        return false;
    }
    if (c.personalPhone.trimmed().isEmpty() && c.workPhone.trimmed().isEmpty() &&
        c.personalEmail.trimmed().isEmpty() && c.workEmail.trimmed().isEmpty()) {
        if (error) *error = "Enter at least one phone number or email.";
        return false;
    }
    static const QRegularExpression emailRx(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");
    for (const QString &mail : {c.personalEmail, c.workEmail}) {
        if (!mail.isEmpty() && !emailRx.match(mail).hasMatch()) {
            if (error) *error = "Email lien he khong hop le.";
            return false;
        }
    }
    if (!c.dateOfBirth.isEmpty() && !QDate::fromString(c.dateOfBirth, Qt::ISODate).isValid()) {
        if (error) *error = "Ngay sinh phai co dang YYYY-MM-DD.";
        return false;
    }
    return true;
}

QVector<Contact> ContactService::list(int userId, const QString &search) {
    QVector<Contact> result;
    if (userId <= 0) return result;
    QSqlQuery q;
    q.prepare("SELECT " + cols + " FROM CONTACT WHERE UserID=? "
              "AND (FullName LIKE ? OR PersonalPhone LIKE ? OR PersonalEmail LIKE ?) "
              "ORDER BY FullName COLLATE NOCASE, ContactID");
    const QString pattern = "%" + search.trimmed() + "%";
    q.addBindValue(userId);
    q.addBindValue(pattern);
    q.addBindValue(pattern);
    q.addBindValue(pattern);
    if (!q.exec()) return result;
    while (q.next()) result.append(fromQuery(q));
    return result;
}

bool ContactService::get(int userId, int contactId, Contact *out) {
    if (!out || userId <= 0 || contactId <= 0) return false;
    QSqlQuery q;
    q.prepare("SELECT " + cols + " FROM CONTACT WHERE UserID=? AND ContactID=?");
    q.addBindValue(userId);
    q.addBindValue(contactId);
    if (!q.exec() || !q.next()) return false;
    *out = fromQuery(q);
    return true;
}

bool ContactService::ownedBy(int userId, int contactId) {
    QSqlQuery q;
    q.prepare("SELECT 1 FROM CONTACT WHERE UserID=? AND ContactID=?");
    q.addBindValue(userId);
    q.addBindValue(contactId);
    return q.exec() && q.next();
}

ContactService::Duplicate ContactService::checkDuplicate(const Contact &c) {
    Duplicate result = Duplicate::None;
    for (const Contact &other : list(c.userId)) {
        if (other.id == c.id || !sharesPhoneOrEmail(c, other)) continue;
        if (sameText(c.fullName, other.fullName)) return Duplicate::SameContact;
        result = Duplicate::SharedInfo;
    }
    return result;
}

bool ContactService::add(Contact c, QString *error) {
    if (!validate(c, error)) return false;
    if (checkDuplicate(c) == Duplicate::SameContact) {
        if (error) *error = "This contact may already exist.";
        return false;
    }
    QSqlQuery q;
    q.prepare(R"(INSERT INTO CONTACT(UserID,FullName,PersonalPhone,WorkPhone,
        PersonalEmail,WorkEmail,Address,Gender,DateOfBirth,Company,Occupation,Notes)
        VALUES(?,?,?,?,?,?,?,?,?,?,?,?))");
    q.addBindValue(c.userId);
    bindContact(q, c);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    return true;
}

bool ContactService::update(int userId, Contact c, QString *error) {
    c.userId = userId;
    if (!validate(c, error) || !ownedBy(userId, c.id)) {
        if (error && error->isEmpty()) *error = "Khong tim thay lien he thuoc tai khoan nay.";
        return false;
    }
    QSqlQuery q;
    q.prepare(R"(UPDATE CONTACT SET FullName=?, PersonalPhone=?, WorkPhone=?,
        PersonalEmail=?, WorkEmail=?, Address=?, Gender=?, DateOfBirth=?, Company=?,
        Occupation=?, Notes=? WHERE ContactID=? AND UserID=?)");
    bindContact(q, c);
    q.addBindValue(c.id);
    q.addBindValue(userId);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool ContactService::remove(int userId, int contactId, QString *error) {
    QSqlQuery q;
    q.prepare("DELETE FROM CONTACT WHERE UserID=? AND ContactID=?");
    q.addBindValue(userId);
    q.addBindValue(contactId);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() < 1) {
        if (error) *error = "Khong tim thay lien he de xoa.";
        return false;
    }
    return true;
}
