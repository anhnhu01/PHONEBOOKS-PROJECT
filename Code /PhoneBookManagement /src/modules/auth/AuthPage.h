#pragma once
#include "../../core/Models.h"
#include <QWidget>
class QLineEdit;
class QStackedWidget;
class QLabel;
class QPushButton;
class QCheckBox;
class QAction;

class AuthPage : public QWidget {
    Q_OBJECT
public:
    explicit AuthPage(QWidget *parent = nullptr);
    void resetForm();
signals:
    void authenticated(const Session &session);
private:
    QStackedWidget *pages_;
    QLineEdit *loginName_, *loginPassword_;
    QAction *loginPasswordToggle_ = nullptr;
    QLineEdit *regName_, *regEmail_, *regPassword_, *regConfirm_;
    // Cảnh báo kiểm tra hợp lệ khi đăng nhập hiển thị ngay trên form 
    QWidget *loginErrorContainer_ = nullptr;
    QLabel *loginErrorIcon_ = nullptr;
    QLabel *loginErrorTitle_ = nullptr;
    QLabel *loginErrorDetail_ = nullptr;
    QPushButton *switchAccountButton_ = nullptr;
    QPushButton *goRegisterButton_ = nullptr;
   // Phản hồi trực tiếp trên form và màn hình đăng ký thành công.
    QLabel *regNameError_ = nullptr;
    QLabel *regEmailError_ = nullptr;
    QLabel *regPasswordError_ = nullptr;
    QLabel *regConfirmError_ = nullptr;
    QLabel *regServerError_ = nullptr;
    QLabel *regPasswordHint_ = nullptr;
    QLabel *regSuccessDescription_ = nullptr;
    QCheckBox *termsCheck_ = nullptr;
    void doLogin();
    void doRegister();
};
