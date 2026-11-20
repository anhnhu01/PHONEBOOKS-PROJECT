#pragma once
#include "../../core/Models.h"
#include <QString>

// THANH VIEN 4: Import CSV su dung ContactService::add() cua thanh vien 2.
struct ImportSummary {
    int imported = 0;
    int skipped = 0;
};

class ImportStatsService {
public:
    static bool importCsv(int userId, const QString &path, ImportSummary *summary,
                          QString *error = nullptr);
    static bool statistics(int adminId, SystemStatistics *stats,
                           QString *error = nullptr);
};
