#include "AdminService.h"
#include <QSqlError>
#include <QSqlQuery>

bool AdminService::isAdmin(int userId) {
    if (userId <= 0) return false;
    QSqlQuery q;
    q.prepare(R"(SELECT 1 FROM "USER" WHERE UserID=? AND Role='ADMIN' AND Status='ACTIVE')");
    q.addBindValue(userId);
    return q.exec() && q.next();
}

QVector<UserAccount> AdminService::accounts(int adminId, const QString &search) {
    QVector<UserAccount> out;
    if (!isAdmin(adminId)) return out;
    QSqlQuery q;
    q.prepare(R"(SELECT UserID,UserName,UserEmail,Role,Status FROM "USER"
                  WHERE UserName LIKE ? OR UserEmail LIKE ? ORDER BY UserID)");
    const QString pattern = "%" + search.trimmed() + "%";
    q.addBindValue(pattern);
    q.addBindValue(pattern);
    if (!q.exec()) return out;
    while (q.next()) {
        out.append({q.value(0).toInt(), q.value(1).toString(), q.value(2).toString(),
                    q.value(3).toString(), q.value(4).toString()});
    }
    return out;
}

bool AdminService::setAccountLocked(int adminId, int targetId, bool locked, QString *error) {
    if (!isAdmin(adminId) || targetId <= 0 || adminId == targetId) {
        if (error) *error = "Khong co quyen thay doi tai khoan nay.";
        return false;
    }
    QSqlQuery q;
    q.prepare(R"(UPDATE "USER" SET Status=? WHERE UserID=? AND Role='USER')");
    q.addBindValue(locked ? "LOCKED" : "ACTIVE");
    q.addBindValue(targetId);
    if (!q.exec() || q.numRowsAffected() <= 0) {
        if (error) *error = q.lastError().text().isEmpty()
                   ? "Chi duoc thay doi tai khoan USER." : q.lastError().text();
        return false;
    }
    return true;
}

bool AdminService::deleteAccount(int adminId, int targetId, QString *error) {
    if (!isAdmin(adminId) || targetId <= 0 || adminId == targetId) {
        if (error) *error = "Khong co quyen xoa tai khoan nay.";
        return false;
    }
    QSqlQuery q;
    q.prepare(R"(DELETE FROM "USER" WHERE UserID=? AND Role='USER')");
    q.addBindValue(targetId);
    if (!q.exec() || q.numRowsAffected() <= 0) {
        if (error) *error = q.lastError().text().isEmpty()
                   ? "Chi duoc xoa tai khoan USER." : q.lastError().text();
        return false;
    }
    return true;
}
