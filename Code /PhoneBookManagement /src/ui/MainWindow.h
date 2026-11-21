#pragma once
#include "../core/Models.h"
#include <QMainWindow>
#include <QVector>
class AuthPage;
class ContactsPage;
class TagFavouritePage;
class ImportStatsPage;
class AdminPage;
class QStackedWidget;
class QPushButton;
class QLabel;

// THUOC THANH VIEN 5. Cac thanh vien khac KHONG SUA main.cpp/MainWindow/CMakeLists.
class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget *parent = nullptr);
private:
    Session session_;
    QStackedWidget *root_;
    QStackedWidget *workspace_;
    AuthPage *auth_;
    ContactsPage *contacts_;
    TagFavouritePage *tags_;
    ImportStatsPage *importStats_;
    AdminPage *admin_;
    QPushButton *adminNav_;
    QLabel *userInfo_;
    QVector<QPushButton*> navButtons_;
    void loggedIn(const Session &session);
    void logOut();
    void showPage(int index);
};
