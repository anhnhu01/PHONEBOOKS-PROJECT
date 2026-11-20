#include "TagFavouriteService.h"
#include "../contacts/ContactService.h"
#include <QSqlError>
#include <QSqlQuery>

namespace {
bool validTag(int userId, int tagId) {
    QSqlQuery q;
    q.prepare("SELECT 1 FROM TAG WHERE UserID=? AND TagID=?");
    q.addBindValue(userId);
    q.addBindValue(tagId);
    return q.exec() && q.next();
}
bool resultOf(QSqlQuery &q, QString *error) {
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() <= 0) {
        if (error) *error = "Khong thay doi du lieu (khong ton tai hoac da them truoc do).";
        return false;
    }
    return true;
}
}

QVector<Tag> TagFavouriteService::tags(int userId) {
    QVector<Tag> out;
    QSqlQuery q;
    q.prepare("SELECT TagID,TagName FROM TAG WHERE UserID=? ORDER BY TagName COLLATE NOCASE");
    q.addBindValue(userId);
    if (q.exec()) while (q.next()) out.append({q.value(0).toInt(), q.value(1).toString()});
    return out;
}

bool TagFavouriteService::createTag(int userId, const QString &name, QString *error) {
    const QString n = name.trimmed();
    if (userId <= 0 || n.isEmpty() || n.size() > 200) {
        if (error) *error = "Ten nhan phai co 1-200 ky tu.";
        return false;
    }
    QSqlQuery q;
    q.prepare("INSERT INTO TAG(UserID,TagName) VALUES(?,?)");
    q.addBindValue(userId);
    q.addBindValue(n);
    return resultOf(q, error);
}

bool TagFavouriteService::renameTag(int userId, int tagId, const QString &name, QString *error) {
    const QString n = name.trimmed();
    if (n.isEmpty() || n.size() > 200) {
        if (error) *error = "Ten nhan phai co 1-200 ky tu.";
        return false;
    }
    QSqlQuery q;
    q.prepare("UPDATE TAG SET TagName=? WHERE UserID=? AND TagID=?");
    q.addBindValue(n);
    q.addBindValue(userId);
    q.addBindValue(tagId);
    return resultOf(q, error);
}

bool TagFavouriteService::deleteTag(int userId, int tagId, QString *error) {
    QSqlQuery q;
    q.prepare("DELETE FROM TAG WHERE UserID=? AND TagID=?");
    q.addBindValue(userId);
    q.addBindValue(tagId);
    return resultOf(q, error);
}

bool TagFavouriteService::assignTag(int userId, int tagId, int contactId, QString *error) {
    if (!validTag(userId, tagId) || !ContactService::ownedBy(userId, contactId)) {
        if (error) *error = "Tag va Contact phai thuoc cung tai khoan.";
        return false;
    }
    QSqlQuery q;
    q.prepare("INSERT OR IGNORE INTO Contact_Tag(TagID,ContactID) VALUES(?,?)");
    q.addBindValue(tagId);
    q.addBindValue(contactId);
    return resultOf(q, error);
}

bool TagFavouriteService::unassignTag(int userId, int tagId, int contactId, QString *error) {
    if (!validTag(userId, tagId) || !ContactService::ownedBy(userId, contactId)) {
        if (error) *error = "Khong co quyen doi gan nhan.";
        return false;
    }
    QSqlQuery q;
    q.prepare("DELETE FROM Contact_Tag WHERE TagID=? AND ContactID=?");
    q.addBindValue(tagId);
    q.addBindValue(contactId);
    return resultOf(q, error);
}

QVector<TagFavouriteService::Assignment> TagFavouriteService::assignments(int userId) {
    QVector<Assignment> out;
    QSqlQuery q;
    q.prepare(R"(SELECT t.TagID,c.ContactID,t.TagName,c.FullName FROM Contact_Tag ct
                 JOIN TAG t ON ct.TagID=t.TagID
                 JOIN CONTACT c ON ct.ContactID=c.ContactID
                 WHERE t.UserID=? AND c.UserID=? ORDER BY t.TagName,c.FullName)");
    q.addBindValue(userId);
    q.addBindValue(userId);
    if (q.exec()) while (q.next())
        out.append({q.value(0).toInt(),q.value(1).toInt(),q.value(2).toString(),q.value(3).toString()});
    return out;
}

QVector<Contact> TagFavouriteService::favourites(int userId) {
    QVector<Contact> out;
    QSqlQuery q;
    q.prepare(R"(SELECT c.ContactID FROM FAVOURITE_LIST f JOIN CONTACT c ON c.ContactID=f.ContactID
                 WHERE c.UserID=? ORDER BY f.AddDate DESC)");
    q.addBindValue(userId);
    if (q.exec()) while (q.next()) {
        Contact c;
        if (ContactService::get(userId, q.value(0).toInt(), &c)) out.append(c);
    }
    return out;
}

bool TagFavouriteService::addFavourite(int userId, int contactId, QString *error) {
    if (!ContactService::ownedBy(userId, contactId)) {
        if (error) *error = "Contact khong thuoc tai khoan nay.";
        return false;
    }
    QSqlQuery q;
    q.prepare("INSERT OR IGNORE INTO FAVOURITE_LIST(ContactID) VALUES(?)");
    q.addBindValue(contactId);
    return resultOf(q, error);
}

bool TagFavouriteService::removeFavourite(int userId, int contactId, QString *error) {
    if (!ContactService::ownedBy(userId, contactId)) {
        if (error) *error = "Contact khong thuoc tai khoan nay.";
        return false;
    }
    QSqlQuery q;
    q.prepare("DELETE FROM FAVOURITE_LIST WHERE ContactID=?");
    q.addBindValue(contactId);
    return resultOf(q, error);
}
