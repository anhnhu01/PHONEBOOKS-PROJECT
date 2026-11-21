#pragma once
#include <QString>

struct Session {
    int userId = 0;
    QString username;
    QString email;
    QString role;
    bool valid() const { return userId > 0; }
    bool isAdmin() const { return role == "ADMIN"; }
};

struct Contact {
    int id = 0;
    int userId = 0;
    QString fullName;
    QString personalPhone;
    QString workPhone;
    QString personalEmail;
    QString workEmail;
    QString address;
    QString gender;
    QString dateOfBirth;
    QString company;
    QString occupation;
    QString notes;
};

struct Tag {
    int id = 0;
    QString name;
};

struct UserAccount {
    int id = 0;
    QString username;
    QString email;
    QString role;
    QString status;
};

struct SystemStatistics {
    int users = 0;
    int contacts = 0;
    int tags = 0;
    int favourites = 0;
    int lockedUsers = 0;
};
