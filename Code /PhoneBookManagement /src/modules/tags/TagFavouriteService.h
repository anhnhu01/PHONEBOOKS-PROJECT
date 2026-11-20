#pragma once
#include "../../core/Models.h"
#include <QPair>
#include <QVector>

// THANH VIEN 3: phan loai TAG, Contact_Tag va FAVOURITE_LIST.
class TagFavouriteService {
public:
    static QVector<Tag> tags(int userId);
    static bool createTag(int userId, const QString &name, QString *error = nullptr);
    static bool renameTag(int userId, int tagId, const QString &name, QString *error = nullptr);
    static bool deleteTag(int userId, int tagId, QString *error = nullptr);
    static bool assignTag(int userId, int tagId, int contactId, QString *error = nullptr);
    static bool unassignTag(int userId, int tagId, int contactId, QString *error = nullptr);
    // Item(tagId, contactId, tagName, contactName)
    struct Assignment { int tagId; int contactId; QString tagName; QString contactName; };
    static QVector<Assignment> assignments(int userId);
    static QVector<Contact> favourites(int userId);
    static bool addFavourite(int userId, int contactId, QString *error = nullptr);
    static bool removeFavourite(int userId, int contactId, QString *error = nullptr);
};
