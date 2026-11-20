#pragma once
#include <QWidget>
class QLineEdit;
class QListWidget;
class QLabel;
class ContactsPage : public QWidget {
public:
    explicit ContactsPage(QWidget *parent = nullptr);
    void setUserId(int userId);
    void refresh();
private:
    int userId_ = 0;
    QLineEdit *search_;
    QListWidget *list_;
    QLabel *count_;
    int selectedId() const;
    void addContact();
    void editContact();
    void deleteContact();
};
