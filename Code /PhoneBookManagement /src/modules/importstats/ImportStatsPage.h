#pragma once
#include "../../core/Models.h"
#include <QWidget>
class QLineEdit;
class QLabel;
class QTabWidget;

class ImportStatsPage : public QWidget {
public:
    explicit ImportStatsPage(QWidget *parent = nullptr);
    void setSession(const Session &session);
    void refreshStatistics();
private:
    Session session_;
    QLineEdit *path_;
    QLabel *status_, *statistics_;
    QTabWidget *tabs_;
    void importFile();
};
