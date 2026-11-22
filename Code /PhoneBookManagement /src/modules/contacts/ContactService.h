#pragma once
#include "../../core/Models.h"
#include <QVector>

// Contact CRUD and search.
class ContactService {
public:
    // SharedInfo: same phone/email, different name. SameContact: same name too.
    enum class Duplicate { None, SharedInfo, SameContact };

    static QVector<Contact> list(int userId, const QString &search = {});
    static bool get(int userId, int contactId, Contact *out);
    static bool validate(const Contact &contact, QString *error = nullptr);
    static Duplicate checkDuplicate(const Contact &contact);
    static bool add(Contact contact, QString *error = nullptr);
    static bool update(int userId, Contact contact, QString *error = nullptr);
    static bool remove(int userId, int contactId, QString *error = nullptr);
    static bool ownedBy(int userId, int contactId);
};
