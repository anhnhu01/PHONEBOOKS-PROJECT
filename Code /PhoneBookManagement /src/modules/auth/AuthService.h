#pragma once
#include "../../core/Models.h"
#include <QString>

// THANH VIEN 1: giao dien va logic xac thuc; contract chung, khong doi tuy tien.
class AuthService {
public:
    static bool registerUser(const QString &username, const QString &email,
                             const QString &password, QString *error = nullptr);
    static Session login(const QString &username, const QString &password,
                         QString *error = nullptr);
    // Tao admin demo 1 lan tu bien moi truong. Khong dat mat khau admin trong code.
    static bool bootstrapAdminFromEnvironment(QString *error = nullptr);
};
