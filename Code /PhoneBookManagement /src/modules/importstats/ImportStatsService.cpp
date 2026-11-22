#include "ImportStatsService.h"
#include "../contacts/ContactService.h"
#include "../admin/AdminService.h"
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVector>

namespace {

bool parseCsv(const QString &text, QVector<QStringList> *rows, QString *error) {
    QString current;
    QStringList row;
    bool quoted = false;
    for (qsizetype i=0; i<text.size(); ++i) {
        const QChar ch = text.at(i);
        if (ch == '"') {
            if (quoted && i + 1 < text.size() && text.at(i+1) == '"') {
                current += '"'; ++i;
            } else {
                quoted = !quoted;
            }
        } else if (ch == ',' && !quoted) {
            row << current;
            current.clear();
        } else if ((ch == '\n' || ch == '\r') && !quoted) {
            if (ch == '\r' && i + 1 < text.size() && text.at(i+1) == '\n') ++i;
            row << current;
            current.clear();
            if (!(row.size() == 1 && row.first().trimmed().isEmpty())) rows->append(row);
            row.clear();
        } else {
            current += ch;
        }
    }
    if (quoted) {
        if (error) *error = "Tep CSV co dau nhay kep chua dong.";
        return false;
    }
    row << current;
    if (!(row.size() == 1 && row.first().trimmed().isEmpty())) rows->append(row);
    return true;
}
int headerIndex(const QStringList &fields, const QStringList &alternatives) {
    for (int i=0; i<fields.size(); ++i) {
        if (alternatives.contains(fields.at(i).trimmed().toLower())) return i;
    }
    return -1;
}
QString column(const QStringList &row, int idx) {
    return idx >= 0 && idx < row.size() ? row.at(idx).trimmed() : QString{};
}
bool countSql(const QString &sql, int *out, QString *error) {
    QSqlQuery q;
    if (!q.exec(sql) || !q.next()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    *out = q.value(0).toInt();
    return true;
}
}

bool ImportStatsService::importCsv(int userId, const QString &path,
                                  ImportSummary *summary, QString *error) {
    if (summary) *summary = {};
    if (userId <= 0) { if (error) *error = "Chua dang nhap."; return false; }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = "Khong mo duoc tep CSV.";
        return false;
    }
    if (file.size() > 5 * 1024 * 1024) {
        if (error) *error = "Tep qua lon (gioi han 5MB).";
        return false;
    }
    QString csv = QString::fromUtf8(file.readAll());
    if (csv.startsWith(QChar(0xFEFF))) csv.remove(0,1);
    QVector<QStringList> rows;
    if (!parseCsv(csv, &rows, error)) return false;
    if (rows.isEmpty()) {
        if (error) *error = "Tep CSV rong.";
        return false;
    }
    if (rows.size() > 10001) {
        if (error) *error = "Toi da 10000 lien he trong 1 lan import.";
        return false;
    }
    const int nameIndex = headerIndex(rows.first(), {"fullname", "name"});
    const int phoneIndex = headerIndex(rows.first(), {"personalphone", "phone"});
    const int mailIndex = headerIndex(rows.first(), {"personalemail", "email"});
    if (nameIndex < 0) {
        if (error) *error = "Thieu cot FullName (hoac Name) trong hang tieu de CSV.";
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        if (error) *error = db.lastError().text();
        return false;
    }
    ImportSummary result;
    for (int i=1; i<rows.size(); ++i) {
        Contact c;
        c.userId = userId;
        c.fullName = column(rows.at(i), nameIndex);
        c.personalPhone = column(rows.at(i), phoneIndex);
        c.personalEmail = column(rows.at(i), mailIndex);
        QString rowError;
        if (c.fullName.isEmpty() || !ContactService::add(c, &rowError)) {
            ++result.skipped;
        } else {
            ++result.imported;
        }
    }
    if (!db.commit()) {
        db.rollback();
        if (error) *error = "Khong luu duoc giao dich: " + db.lastError().text();
        return false;
    }
    if (summary) *summary = result;
    return true;
}

bool ImportStatsService::statistics(int adminId, SystemStatistics *stats, QString *error) {
    if (!stats) return false;
    if (!AdminService::isAdmin(adminId)) {
        if (error) *error = "Chuc nang thong ke chi danh cho Admin.";
        return false;
    }
    SystemStatistics s;
    if (!countSql("SELECT COUNT(*) FROM \"USER\"", &s.users, error) ||
        !countSql("SELECT COUNT(*) FROM CONTACT", &s.contacts, error) ||
        !countSql("SELECT COUNT(*) FROM TAG", &s.tags, error) ||
        !countSql("SELECT COUNT(*) FROM FAVOURITE_LIST", &s.favourites, error) ||
        !countSql("SELECT COUNT(*) FROM \"USER\" WHERE Status='LOCKED'", &s.lockedUsers, error))
        return false;
    *stats = s;
    return true;
}
