#pragma once
#include <QWidget>
class QLineEdit;
class QTableWidget;

class AdminPage : public QWidget {
public:
    explicit AdminPage(QWidget *parent = nullptr);
    void setAdminId(int adminId);
    void refresh();
private:
    int adminId_ = 0;
    QLineEdit *search_;
    QTableWidget *table_;
    int selectedId() const;
    void changeStatus(bool locked);
    void deleteAccount();
};
