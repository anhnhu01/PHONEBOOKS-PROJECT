#include "MainWindow.h"
#include "../modules/auth/AuthPage.h"
#include "../modules/auth/ProfileDialog.h"
#include "../modules/contacts/ContactsPage.h"
#include "../modules/tags/TagFavouritePage.h"
#include "../modules/importstats/ImportStatsPage.h"
#include "../modules/admin/AdminPage.h"
#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyle>
#include <QTime>
#include <QToolButton>
#include <QVBoxLayout>

// Stage 3 MOBILE PREVIEW: Qt Widgets desktop application in a portrait window.
// The services and SQLite schema remain unchanged; only the presentation shell changes.
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Phonebook · Mobile preview");
    resize(402, 900);
    setMinimumSize(380, 720);
    setMaximumWidth(460);

    root_ = new QStackedWidget(this);
    setCentralWidget(root_);
    auth_ = new AuthPage;
    root_->addWidget(auth_);

    auto *shell = new QWidget;
    auto *column = new QVBoxLayout(shell);
    column->setContentsMargins(0,0,0,0);
    column->setSpacing(0);

    // Figma-style phone status strip (demo decoration; this is not Android OS status).
    auto *status = new QWidget;
    status->setObjectName("statusStrip");
    status->setFixedHeight(36);
    auto *statusRow = new QHBoxLayout(status);
    statusRow->setContentsMargins(26,0,26,0);
    auto *clock = new QLabel(QTime::currentTime().toString("h:mm"));
    clock->setStyleSheet("font-weight:600; font-size:13px;");
    statusRow->addWidget(clock);
    statusRow->addStretch();
    statusRow->addWidget(new QLabel("●  ▰"));
    column->addWidget(status);

    auto *accountBar = new QWidget;
    accountBar->setObjectName("accountBar");
    accountBar->setFixedHeight(50);
    auto *accountRow = new QHBoxLayout(accountBar);
    accountRow->setContentsMargins(24, 2, 20, 2);
    userInfo_ = new QLabel;
    userInfo_->setObjectName("accountCaption");
    accountRow->addWidget(userInfo_,1);
    auto *menuButton = new QToolButton;
    menuButton->setObjectName("menuButton");
    menuButton->setText("⋮");
    menuButton->setToolTip("Account menu");
    menuButton->setFixedSize(38,38);
    auto *menu = new QMenu(menuButton);
    QAction *profileAction = menu->addAction("Profile");
    QAction *logoutAction = menu->addAction("Log out");
    menuButton->setMenu(menu);
    menuButton->setPopupMode(QToolButton::InstantPopup);
    accountRow->addWidget(menuButton);
    column->addWidget(accountBar);

    workspace_ = new QStackedWidget;
    contacts_ = new ContactsPage;
    tags_ = new TagFavouritePage;
    importStats_ = new ImportStatsPage;
    admin_ = new AdminPage;
    workspace_->addWidget(contacts_);
    workspace_->addWidget(tags_);
    workspace_->addWidget(importStats_);
    workspace_->addWidget(admin_);
    column->addWidget(workspace_, 1);

    auto *bottom = new QWidget;
    bottom->setObjectName("bottomNavigation");
    bottom->setFixedHeight(66);
    auto *bottomRow = new QHBoxLayout(bottom);
    bottomRow->setContentsMargins(10, 5, 10, 5);
    bottomRow->setSpacing(4);
    auto nav = [this, bottomRow](const QString &label, int index) {
        auto *button = new QPushButton(label);
        button->setObjectName("navButton");
        button->setCursor(Qt::PointingHandCursor);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        bottomRow->addWidget(button,1);
        navButtons_.append(button);
        connect(button, &QPushButton::clicked, this, [this,index]() { showPage(index); });
        return button;
    };
    nav("Contacts",0);
    nav("Tags",1);
    nav("Import",2);
    adminNav_ = nav("Admin",3);
    column->addWidget(bottom);

    auto *homeHandle = new QLabel("━━━━");
    homeHandle->setObjectName("homeHandle");
    homeHandle->setAlignment(Qt::AlignCenter);
    homeHandle->setFixedHeight(24);
    column->addWidget(homeHandle);
    root_->addWidget(shell);

    connect(auth_, &AuthPage::authenticated, this,
            [this](const Session &s) { loggedIn(s); });
    connect(profileAction, &QAction::triggered, this, [this]() {
        ProfileDialog dialog(session_, this);
        dialog.exec();
    });
    connect(logoutAction, &QAction::triggered, this, [this]() { logOut(); });
    root_->setCurrentIndex(0);
}

void MainWindow::loggedIn(const Session &session) {
    session_ = session;
    userInfo_->setText("Hello, " + session.username + (session.isAdmin() ? "  ·  Admin" : ""));
    adminNav_->setVisible(session.isAdmin());
    contacts_->setUserId(session.userId);
    tags_->setUserId(session.userId);
    importStats_->setSession(session);
    admin_->setAdminId(session.isAdmin() ? session.userId : 0);
    showPage(0);
    root_->setCurrentIndex(1);
}
void MainWindow::logOut() {
    session_ = Session{};
    contacts_->setUserId(0);
    tags_->setUserId(0);
    importStats_->setSession(Session{});
    admin_->setAdminId(0);
    auth_->resetForm();
    root_->setCurrentIndex(0);
}
void MainWindow::showPage(int index) {
    if (!session_.valid()) return;
    if (index == 3 && !session_.isAdmin()) return;
    if (index == 0) contacts_->refresh();
    if (index == 1) tags_->refresh();
    if (index == 2) importStats_->refreshStatistics();
    if (index == 3) admin_->refresh();
    workspace_->setCurrentIndex(index);
    for (int i = 0; i < navButtons_.size(); ++i) {
        navButtons_[i]->setProperty("active", i == index);
        navButtons_[i]->style()->unpolish(navButtons_[i]);
        navButtons_[i]->style()->polish(navButtons_[i]);
    }
}
