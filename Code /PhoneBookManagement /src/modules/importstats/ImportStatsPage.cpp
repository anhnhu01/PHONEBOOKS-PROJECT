#include "ImportStatsPage.h"
#include "ImportStatsService.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

ImportStatsPage::ImportStatsPage(QWidget *parent): QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel("Import & Statistics");
    title->setObjectName("heading");
    layout->addWidget(title);
    tabs_ = new QTabWidget;
    auto *import = new QWidget;
    auto *il = new QVBoxLayout(import);
    auto *hint = new QLabel("CSV columns: FullName, PersonalPhone, PersonalEmail");
    hint->setWordWrap(true);
    il->addWidget(hint);
    auto *row = new QVBoxLayout;
    path_ = new QLineEdit;
    path_->setPlaceholderText("Choose a CSV file...");
    auto *browse = new QPushButton("Browse");
    auto *run = new QPushButton("Import CSV");
    row->addWidget(path_); row->addWidget(browse); row->addWidget(run);
    il->addLayout(row);
    status_ = new QLabel("Ready to import. Invalid rows will be skipped.");
    status_->setWordWrap(true);
    il->addWidget(status_);
    il->addStretch();
    tabs_->addTab(import, "Import CSV");
    auto *stats = new QWidget;
    auto *sl = new QVBoxLayout(stats);
    statistics_ = new QLabel("Statistics are available to Admin only.");
    statistics_->setObjectName("sectionHeading");
    statistics_->setWordWrap(true);
    sl->addWidget(statistics_);
    auto *refresh = new QPushButton("Refresh statistics");
    sl->addWidget(refresh);
    sl->addStretch();
    tabs_->addTab(stats, "Statistics");
    layout->addWidget(tabs_);
    connect(browse, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(this, "Choose CSV", {}, "CSV (*.csv)");
        if (!path.isEmpty()) path_->setText(path);
    });
    connect(run, &QPushButton::clicked, this, [this]() { importFile(); });
    connect(refresh, &QPushButton::clicked, this, [this]() { refreshStatistics(); });
}

void ImportStatsPage::setSession(const Session &session) {
    session_ = session;
    tabs_->setTabVisible(1, session_.isAdmin());
    tabs_->setCurrentIndex(0);
    status_->setText("Ready to import. Invalid rows will be skipped.");
    if (session_.isAdmin()) refreshStatistics();
}
void ImportStatsPage::importFile() {
    ImportSummary summary;
    QString error;
    if (!ImportStatsService::importCsv(session_.userId, path_->text(), &summary, &error)) {
        QMessageBox::warning(this, "Import failed", error);
        return;
    }
    const QString report = QString("Imported: %1 | Skipped invalid: %2")
            .arg(summary.imported).arg(summary.skipped);
    status_->setText(report);
    QMessageBox::information(this, "Import complete", report);
    if (session_.isAdmin()) refreshStatistics();
}
void ImportStatsPage::refreshStatistics() {
    if (!session_.isAdmin()) return;
    SystemStatistics s;
    QString error;
    if (!ImportStatsService::statistics(session_.userId, &s, &error)) {
        statistics_->setText(error); return;
    }
    statistics_->setText(QString(
        "Accounts: %1\nContacts: %2\nTags: %3\nFavourites: %4\nLocked accounts: %5")
        .arg(s.users).arg(s.contacts).arg(s.tags).arg(s.favourites).arg(s.lockedUsers));
}
