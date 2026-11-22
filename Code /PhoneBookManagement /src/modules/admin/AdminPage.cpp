#include "AdminPage.h"
#include "AdminService.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QStringList>
#include <QVBoxLayout>

AdminPage::AdminPage(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel("Admin – User accounts");
    title->setObjectName("heading");
    layout->addWidget(title);
    auto *toolbar = new QVBoxLayout;
    search_ = new QLineEdit;
    search_->setPlaceholderText("Find a user...");
    auto *lock = new QPushButton("Lock");
    auto *unlock = new QPushButton("Unlock");
    auto *del = new QPushButton("Delete user");
    toolbar->addWidget(search_);
    auto *actions = new QHBoxLayout;
    actions->addWidget(lock);
    actions->addWidget(unlock);
    actions->addWidget(del);
    toolbar->addLayout(actions);
    layout->addLayout(toolbar);
    table_ = new QTableWidget(0,5);
    table_->setHorizontalHeaderLabels({"ID", "Username", "Email", "Role", "Status"});
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // Preserve all five data fields, but hide ID/email in the narrow preview.
    table_->setColumnHidden(0,true);
    table_->setColumnHidden(2,true);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table_->verticalHeader()->setVisible(false);
    layout->addWidget(table_);
    connect(search_, &QLineEdit::textChanged, this, [this]() { refresh(); });
    connect(lock, &QPushButton::clicked, this, [this]() { changeStatus(true); });
    connect(unlock, &QPushButton::clicked, this, [this]() { changeStatus(false); });
    connect(del, &QPushButton::clicked, this, [this]() { deleteAccount(); });
}
void AdminPage::setAdminId(int adminId) {
    adminId_ = adminId;
    search_->clear();
    refresh();
}
void AdminPage::refresh() {
    table_->setRowCount(0);
    for (const auto &u : AdminService::accounts(adminId_, search_->text())) {
        const int row = table_->rowCount();
        table_->insertRow(row);
        const QStringList values = {QString::number(u.id), u.username, u.email, u.role, u.status};
        for (int col=0; col<values.size(); ++col)
            table_->setItem(row,col,new QTableWidgetItem(values.at(col)));
    }
}
int AdminPage::selectedId() const {
    const int r = table_->currentRow();
    return r >= 0 && table_->item(r,0) ? table_->item(r,0)->text().toInt() : 0;
}
void AdminPage::changeStatus(bool locked) {
    const int id = selectedId();
    if (!id) return;
    QString error;
    if (!AdminService::setAccountLocked(adminId_, id, locked, &error)) {
        QMessageBox::warning(this, "Admin", error); return;
    }
    refresh();
}
void AdminPage::deleteAccount() {
    const int id = selectedId();
    if (!id) return;
    if (QMessageBox::question(this, "Delete user", "Delete their account and ALL of their contacts?") != QMessageBox::Yes)
        return;
    QString error;
    if (!AdminService::deleteAccount(adminId_, id, &error)) {
        QMessageBox::warning(this, "Admin", error); return;
    }
    refresh();
}
