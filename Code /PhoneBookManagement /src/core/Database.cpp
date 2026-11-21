#include "Database.h"
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringList>

QString Database::filePath() {
    const QString folder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return folder + "/phonebook.db";
}

bool Database::initialize(QString *error) {
    const QString folder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir().mkpath(folder)) {
        if (error) *error = "Khong the tao thu muc du lieu: " + folder;
        return false;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filePath());
    if (!db.open()) {
        if (error) *error = "Khong the mo SQLite: " + db.lastError().text();
        return false;
    }
    QSqlQuery q(db);
    if (!q.exec("PRAGMA foreign_keys = ON")) {
        if (error) *error = q.lastError().text();
        return false;
    }
    const QStringList statements = {
        R"(CREATE TABLE IF NOT EXISTS "USER" (
            UserID INTEGER PRIMARY KEY AUTOINCREMENT,
            UserName TEXT NOT NULL UNIQUE COLLATE NOCASE,
            UserEmail TEXT NOT NULL UNIQUE COLLATE NOCASE,
            PasswordHash TEXT NOT NULL,
            PasswordSalt TEXT NOT NULL,
            Role TEXT NOT NULL DEFAULT 'USER' CHECK(Role IN ('USER','ADMIN')),
            Status TEXT NOT NULL DEFAULT 'ACTIVE' CHECK(Status IN ('ACTIVE','LOCKED'))))",
        R"(CREATE TABLE IF NOT EXISTS CONTACT (
            ContactID INTEGER PRIMARY KEY AUTOINCREMENT,
            UserID INTEGER NOT NULL,
            FullName TEXT NOT NULL,
            PersonalPhone TEXT, WorkPhone TEXT,
            PersonalEmail TEXT, WorkEmail TEXT,
            Address TEXT, Gender TEXT, DateOfBirth TEXT,
            Company TEXT, Occupation TEXT, Notes TEXT,
            FOREIGN KEY(UserID) REFERENCES "USER"(UserID) ON DELETE CASCADE))",
        R"(CREATE TABLE IF NOT EXISTS TAG (
            TagID INTEGER PRIMARY KEY AUTOINCREMENT,
            UserID INTEGER NOT NULL,
            TagName TEXT NOT NULL,
            UNIQUE(UserID, TagName),
            FOREIGN KEY(UserID) REFERENCES "USER"(UserID) ON DELETE CASCADE))",
        R"(CREATE TABLE IF NOT EXISTS Contact_Tag (
            TagID INTEGER NOT NULL,
            ContactID INTEGER NOT NULL,
            PRIMARY KEY(TagID, ContactID),
            FOREIGN KEY(TagID) REFERENCES TAG(TagID) ON DELETE CASCADE,
            FOREIGN KEY(ContactID) REFERENCES CONTACT(ContactID) ON DELETE CASCADE))",
        R"(CREATE TABLE IF NOT EXISTS FAVOURITE_LIST (
            ContactID INTEGER PRIMARY KEY,
            AddDate TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(ContactID) REFERENCES CONTACT(ContactID) ON DELETE CASCADE))",
        "CREATE INDEX IF NOT EXISTS ix_contacts_owner ON CONTACT(UserID)",
        "CREATE INDEX IF NOT EXISTS ix_tags_owner ON TAG(UserID)"
    };
    for (const auto &sql : statements) {
        if (!q.exec(sql)) {
            if (error) *error = "Loi tao bang: " + q.lastError().text();
            return false;
        }
    }
    return true;
}
