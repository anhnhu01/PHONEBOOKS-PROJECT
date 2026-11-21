#pragma once
#include <QString>

class Database {
public:
    static bool initialize(QString *error = nullptr);
    static QString filePath();
};
