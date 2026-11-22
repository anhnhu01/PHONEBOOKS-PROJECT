#pragma once
#include "../../core/Models.h"
#include <QString>

class AuthService {
public:
    static bool registerUser(const QString &username, const QString &email,
                             const QString &password, QString *error = nullptr);
    static Session login(const QString &username, const QString &password,
                         QString *error = nullptr);
    static bool bootstrapAdminFromEnvironment(QString *error = nullptr);
};
