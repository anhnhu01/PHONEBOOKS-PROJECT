#pragma once
#include "../../core/Models.h"
#include <QVector>

class AdminService {
public:
    static bool isAdmin(int userId);
    static QVector<UserAccount> accounts(int adminId, const QString &search = {});
    static bool setAccountLocked(int adminId, int targetId, bool locked,
                                 QString *error = nullptr);
    static bool deleteAccount(int adminId, int targetId, QString *error = nullptr);
};
