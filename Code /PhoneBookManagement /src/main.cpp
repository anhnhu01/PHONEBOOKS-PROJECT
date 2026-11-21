#include <QApplication>
#include <QMessageBox>
#include "core/Database.h"
#include "modules/auth/AuthService.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("StudentProject");
    QCoreApplication::setApplicationName("PhoneBookManagement");
    app.setStyle("Fusion");
    app.setStyleSheet(R"(
        QWidget { font-family: 'Inter', 'Segoe UI', Arial; font-size: 13px; color: #202F29; background: #F8F7F2; }
        QLabel { background: transparent; }
        QMainWindow, QDialog { background: #F8F7F2; }
        QLabel#heading { font-size: 29px; font-weight: 600; color: #202F29; }
        QLabel#authHeading { font-size: 30px; font-weight: 500; color: #202F29; }
        QLabel#brandName { font-size: 26px; color: #21745B; }
        QLabel#brandIcon { background: #21745B; color: white; font-size: 20px; border-radius: 18px; font-weight: bold; }
        QLabel#subtleText, QLabel#accountCaption { color: #738078; font-size: 13px; }
        QLabel#sectionHeading { color: #738078; font-size: 12px; font-weight: 600; }
        QLabel#fieldLabel { color: #202F29; font-size: 13px; font-weight: 600; }
        QLabel#contactName { color: #202F29; font-size: 15px; font-weight: 600; }
        QLabel#contactAvatar { background: #DCE9DF; border-radius: 24px; font-size: 16px; font-weight: 600; }
        QLineEdit, QComboBox, QDateEdit, QTextEdit, QTableWidget, QListWidget {
            background: white; border: 1px solid #E2E5DC; border-radius: 12px; padding: 9px;
            color: #202F29; selection-background-color: #DCE9DF; selection-color: #202F29;
        }
        QComboBox { min-height: 32px; }
        QLineEdit:focus, QComboBox:focus { border: 1px solid #21745B; }
        QPushButton { background: #21745B; color: white; border: none; border-radius: 12px;
                      padding: 9px 10px; font-weight: 600; min-height: 24px; }
        QPushButton:hover { background: #185D49; }
        QPushButton:disabled { background: #A4B7AA; }
        QPushButton#textButton { color: #21745B; background: transparent; font-size: 13px; border: none; }
        QPushButton#secondaryButton { color: #21745B; background: #E5EFE7; font-size: 12px; }
        QPushButton#addContactButton { color: #21745B; background: #E5EFE7; font-size: 24px; padding: 0px; }
        QPushButton#navButton { color: #738078; background: transparent; border-radius: 8px;
                                font-size: 12px; padding: 8px 2px; font-weight: 500; }
        QPushButton#navButton[active="true"] { color: #21745B; background: #E5EFE7; font-weight: 700; }
        QWidget#bottomNavigation { background: #FFFFFF; border-top: 1px solid #E2E5DC; }
        QWidget#accountBar, QWidget#statusStrip { background: #F8F7F2; }
        QToolButton#menuButton { background: #E5EFE7; color: #21745B; border-radius: 18px; font-size: 20px; }
        QLabel#homeHandle { background: #F8F7F2; font-size: 14px; }
        QListWidget#contactsList { background: #F8F7F2; border: none; padding: 0; }
        QListWidget#contactsList::item { border-bottom: 1px solid #E2E5DC; padding: 0px; }
        QListWidget#contactsList::item:selected { background: #E5EFE7; }
        QTableWidget { gridline-color: #E2E5DC; }
        QHeaderView::section { background: #E5EFE7; color: #202F29; padding: 6px; border: none; font-weight: 600; }
        QTabWidget::pane { border: 1px solid #E2E5DC; border-radius: 8px; }
        QTabBar::tab { background: #E5EFE7; color: #738078; padding: 9px 12px; }
        QTabBar::tab:selected { background: white; color: #21745B; font-weight: 700; }
    )");
    QString error;
    if (!Database::initialize(&error)) {
        QMessageBox::critical(nullptr, "Database error", error);
        return 1;
    }
    if (!AuthService::bootstrapAdminFromEnvironment(&error)) {
        QMessageBox::warning(nullptr, "Admin setup", error);
    }
    MainWindow window;
    window.show();
    return app.exec();
}
