#include "AuthPage.h"
#include "AuthService.h"
#include <QAction>
#include <QColor>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSettings>
#include <QStackedWidget>
#include <QTime>
#include <QVBoxLayout>

namespace {
QLabel *label(const QString &text, const char *objectName = nullptr) {
    auto *l = new QLabel(text);
    l->setWordWrap(true);
    if (objectName) l->setObjectName(objectName);
    return l;
}
QLineEdit *field(const QString &hint, bool password = false) {
    auto *edit = new QLineEdit;
    edit->setPlaceholderText(hint);
    edit->setMinimumHeight(50);
    if (password) edit->setEchoMode(QLineEdit::Password);
    return edit;
}
void addField(QVBoxLayout *layout, const QString &name, QLineEdit *edit) {
    layout->addWidget(label(name, "fieldLabel"));
    layout->addWidget(edit);
}
QWidget *scrollPage(QWidget *content) {
    auto *wrapper = new QWidget;
    auto *layout = new QVBoxLayout(wrapper);
    layout->setContentsMargins(0,0,0,0);
    auto *scroll = new QScrollArea;
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidget(content);
    layout->addWidget(scroll);
    return wrapper;
}
// Draw the same eye/eye-off interaction without an external image file.
// `visible` refers to the password's CURRENT visibility.
QIcon passwordEyeIcon(bool visible) {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    const QPen line(QColor("#738078"), 1.8, Qt::SolidLine,
                    Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(line);

    QPainterPath eye;
    eye.moveTo(2, 12);
    eye.cubicTo(7, 5.5, 17, 5.5, 22, 12);
    eye.cubicTo(17, 18.5, 7, 18.5, 2, 12);
    painter.drawPath(eye);
    painter.drawEllipse(QPointF(12, 12), 3, 3);

    if (!visible) {
        // A gap beneath the slash keeps the closed-eye icon readable.
        painter.setPen(QPen(QColor("#FFFFFF"), 3.6, Qt::SolidLine,
                            Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(3.5, 20.5), QPointF(20.5, 3.5));
        painter.setPen(line);
        painter.drawLine(QPointF(3.5, 20.5), QPointF(20.5, 3.5));
    }
    return QIcon(pixmap);
}

void setPasswordVisibility(QLineEdit *edit, QAction *action, bool visible) {
    edit->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    action->setIcon(passwordEyeIcon(visible));
    const QString caption = visible ? QStringLiteral("Hide password")
                                    : QStringLiteral("Show password");
    action->setText(caption);
    action->setToolTip(caption);
}

} // namespace

AuthPage::AuthPage(QWidget *parent): QWidget(parent) {
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0,0,0,0);
    outer->setSpacing(0);

    auto *strip = new QWidget;
    strip->setFixedHeight(44);
    auto *stripRow = new QHBoxLayout(strip);
    stripRow->setContentsMargins(28,0,28,0);
    stripRow->addWidget(label(QTime::currentTime().toString("h:mm")));
    stripRow->addStretch();
    stripRow->addWidget(label("●  ▰"));
    outer->addWidget(strip);

    pages_ = new QStackedWidget;
    outer->addWidget(pages_,1);

    // Login — portrait composition based on Figma frame 2006:8413.
    auto *loginContent = new QWidget;
    auto *l = new QVBoxLayout(loginContent);
    l->setContentsMargins(24,20,24,24);
    l->setSpacing(0);
    l->addSpacing(20);
    auto *icon = label("PB", "brandIcon");
    icon->setAlignment(Qt::AlignCenter);
    icon->setFixedSize(62,62);
    l->addWidget(icon);
    l->addSpacing(16);
    l->addWidget(label("Phonebook", "brandName"));
    l->addSpacing(34);
    l->addWidget(label("Hello,\nwelcome back.", "authHeading"));
    l->addSpacing(6);
    l->addWidget(label("Log in to connect with the people who matter.", "subtleText"));
    l->addSpacing(28);
    loginName_ = field("Username or email");
    loginPassword_ = field("Password", true);
    addField(l, "Username", loginName_);
    l->addSpacing(14);
    addField(l, "Password", loginPassword_);
    // Add an eye icon INSIDE the Login password field, as in Figma.
    loginPasswordToggle_ = loginPassword_->addAction(
        passwordEyeIcon(false), QLineEdit::TrailingPosition);
    setPasswordVisibility(loginPassword_, loginPasswordToggle_, false);
    connect(loginPasswordToggle_, &QAction::triggered, this, [this]() {
        const bool show = (loginPassword_->echoMode() == QLineEdit::Password);
        setPasswordVisibility(loginPassword_, loginPasswordToggle_, show);
    });

    // Inline error notification, matching the two Figma login error states.
    // Hide the whole container when there is no error so it takes no space.
    loginErrorContainer_ = new QWidget;
    auto *errorContainerLayout = new QVBoxLayout(loginErrorContainer_);
    errorContainerLayout->setContentsMargins(0, 12, 0, 0);
    errorContainerLayout->setSpacing(0);
    auto *errorCard = new QFrame;
    errorCard->setObjectName("loginErrorCard");
    auto *errorRow = new QHBoxLayout(errorCard);
    errorRow->setContentsMargins(12, 11, 12, 11);
    errorRow->setSpacing(9);

    loginErrorIcon_ = label("ⓘ", "loginErrorIcon");
    loginErrorIcon_->setFixedWidth(18);
    loginErrorIcon_->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    errorRow->addWidget(loginErrorIcon_);

    auto *errorTexts = new QVBoxLayout;
    errorTexts->setContentsMargins(0, 0, 0, 0);
    errorTexts->setSpacing(3);
    loginErrorTitle_ = label("", "loginErrorTitle");
    loginErrorDetail_ = label("", "loginErrorDetail");
    errorTexts->addWidget(loginErrorTitle_);
    errorTexts->addWidget(loginErrorDetail_);
    errorRow->addLayout(errorTexts, 1);
    errorContainerLayout->addWidget(errorCard);

    // Scoped style: the warning box won't change other screens.
    errorCard->setStyleSheet(R"(
        QFrame#loginErrorCard {
            background-color: #F9EAE6;
            border: none;
            border-radius: 12px;
        }
        QLabel#loginErrorIcon, QLabel#loginErrorTitle, QLabel#loginErrorDetail {
            background: transparent;
            color: #BD4B42;
        }
        QLabel#loginErrorIcon { font-size: 16px; }
        QLabel#loginErrorTitle { font-size: 12px; font-weight: 600; }
        QLabel#loginErrorDetail { font-size: 11px; }
    )");
    l->addWidget(loginErrorContainer_);
    loginErrorContainer_->hide();
    l->addSpacing(15);
    auto *remember = new QCheckBox("Remember username on this computer");
    remember->setObjectName("rememberUser");
    QSettings prefs;
    remember->setChecked(prefs.value("login/remember", false).toBool());
    if (remember->isChecked()) loginName_->setText(prefs.value("login/username").toString());
    l->addWidget(remember);
    l->addSpacing(24);
    auto *loginButton = new QPushButton("Log in");
    loginButton->setMinimumHeight(50);
    l->addWidget(loginButton);
    l->addSpacing(12);
    switchAccountButton_ = new QPushButton("Log in with another account");
    switchAccountButton_->setObjectName("textButton");
    l->addWidget(switchAccountButton_);
    switchAccountButton_->hide();

    goRegisterButton_ = new QPushButton("No account yet? Sign up");
    goRegisterButton_->setObjectName("textButton");
    l->addWidget(goRegisterButton_);
    l->addSpacing(30);
    auto *privacy = label("Private contacts. Peace of mind.", "subtleText");
    privacy->setAlignment(Qt::AlignCenter);
    l->addWidget(privacy);
    l->addStretch();
    pages_->addWidget(scrollPage(loginContent));
    connect(loginButton, &QPushButton::clicked, this, [this,remember]() {
        QSettings settings;
        settings.setValue("login/remember", remember->isChecked());
        settings.setValue("login/username", remember->isChecked() ? loginName_->text().trimmed() : QString());
        doLogin();
    });
    connect(loginPassword_, &QLineEdit::returnPressed, loginButton, &QPushButton::click);
    connect(goRegisterButton_, &QPushButton::clicked, this, [this]() {
        setPasswordVisibility(loginPassword_, loginPasswordToggle_, false);
        pages_->setCurrentIndex(1);
    });
    connect(switchAccountButton_, &QPushButton::clicked, this, [this]() {
        loginName_->clear();
        loginPassword_->clear();
        setPasswordVisibility(loginPassword_, loginPasswordToggle_, false);
        loginErrorContainer_->hide();
        switchAccountButton_->hide();
        goRegisterButton_->show();
        loginName_->setFocus();
    });
    // Automatically remove the previous error as soon as the user edits a field.
    auto clearLoginAlert = [this]() {
        loginErrorContainer_->hide();
        switchAccountButton_->hide();
        goRegisterButton_->show();
    };
    connect(loginName_, &QLineEdit::textChanged, this, clearLoginAlert);
    connect(loginPassword_, &QLineEdit::textChanged, this, clearLoginAlert);

    // Sign-up: Figma default and validation states.
    auto *registerContent = new QWidget;
    auto *r = new QVBoxLayout(registerContent);
    r->setContentsMargins(24, 12, 24, 24);
    r->setSpacing(0);

    auto *back = new QPushButton(QStringLiteral("‹  Log in"));
    back->setObjectName("textButton");
    r->addWidget(back, 0, Qt::AlignLeft);
    r->addSpacing(24);
    r->addWidget(label("Create account", "authHeading"));
    r->addSpacing(6);
    r->addWidget(label("One place for all your connections.", "subtleText"));
    r->addSpacing(20);

    regName_ = field("Choose a username");
    regEmail_ = field("Email address");
    regPassword_ = field("At least 8 characters", true);
    regConfirm_ = field("Confirm password", true);

    // Each message is placed immediately below the corresponding input.
    auto addValidatedField = [r](const QString &caption, QLineEdit *edit,
                                 QLabel *&errorLabel) {
        addField(r, caption, edit);
        errorLabel = label("", "registerFieldError");
        errorLabel->setStyleSheet("color:#BD4B42; font-size:11px;");
        r->addWidget(errorLabel);
        errorLabel->hide();
        r->addSpacing(14);
    };
    addValidatedField("Username", regName_, regNameError_);
    addValidatedField("Email", regEmail_, regEmailError_);
    addValidatedField("Password", regPassword_, regPasswordError_);

    regPasswordHint_ = label("At least 8 characters", "passwordHint");
    regPasswordHint_->setStyleSheet("color:#738078; font-size:12px;");
    r->addWidget(regPasswordHint_);
    r->addSpacing(14);
    addValidatedField("Confirm password", regConfirm_, regConfirmError_);

    auto *showPasswords = new QCheckBox("Show passwords");
    showPasswords->setObjectName("rememberUser");
    r->addWidget(showPasswords);
    r->addSpacing(16);
    auto *termsRow = new QHBoxLayout;
    termsRow->setSpacing(9);
    termsCheck_ = new QCheckBox;
    termsCheck_->setObjectName("rememberUser");
    auto *termsText = label("I agree to Phonebook's Terms of Use and Privacy Policy.", "subtleText");
    termsRow->addWidget(termsCheck_, 0, Qt::AlignTop);
    termsRow->addWidget(termsText, 1);
    r->addLayout(termsRow);

    regServerError_ = label("", "registerServerError");
    regServerError_->setStyleSheet(
        "background:#F9EAE6; color:#BD4B42; padding:11px; border-radius:12px; font-size:12px;");
    r->addSpacing(12);
    r->addWidget(regServerError_);
    regServerError_->hide();

    r->addSpacing(20);
    auto *regButton = new QPushButton("Create account");
    regButton->setMinimumHeight(50);
    r->addWidget(regButton);
    r->addSpacing(12);
    auto *toLogin = new QPushButton("Already have an account? Log in");
    toLogin->setObjectName("textButton");
    r->addWidget(toLogin);
    r->addStretch();
    pages_->addWidget(scrollPage(registerContent)); // index 1

    // Successful signup uses its own Figma-like screen rather than a QMessageBox.
    auto *successContent = new QWidget;
    auto *success = new QVBoxLayout(successContent);
    success->setContentsMargins(24, 90, 24, 24);
    success->setSpacing(16);
    auto *successIcon = label(QStringLiteral("✓"), "brandIcon");
    successIcon->setAlignment(Qt::AlignCenter);
    successIcon->setFixedSize(62, 62);
    success->addWidget(successIcon, 0, Qt::AlignHCenter);
    auto *successHeading = label("Account created\nsuccessfully.", "authHeading");
    successHeading->setAlignment(Qt::AlignCenter);
    success->addWidget(successHeading);
    regSuccessDescription_ = label("", "subtleText");
    regSuccessDescription_->setAlignment(Qt::AlignCenter);
    success->addWidget(regSuccessDescription_);
    success->addSpacing(14);
    auto *successLogin = new QPushButton("Log in");
    successLogin->setMinimumHeight(50);
    success->addWidget(successLogin);
    success->addStretch();
    pages_->addWidget(scrollPage(successContent)); // index 2

    connect(back, &QPushButton::clicked, this,
            [this]() { pages_->setCurrentIndex(0); });
    connect(toLogin, &QPushButton::clicked, this,
            [this]() { pages_->setCurrentIndex(0); });
    connect(successLogin, &QPushButton::clicked, this,
            [this]() { pages_->setCurrentIndex(0); loginPassword_->setFocus(); });
    connect(regButton, &QPushButton::clicked, this, &AuthPage::doRegister);
    connect(showPasswords, &QCheckBox::toggled, this, [this](bool checked) {
        const auto mode = checked ? QLineEdit::Normal : QLineEdit::Password;
        regPassword_->setEchoMode(mode);
        regConfirm_->setEchoMode(mode);
    });

    // Hide outdated validation messages as soon as the user changes the input.
    auto clearError = [this](QLineEdit *edit, QLabel *message) {
        connect(edit, &QLineEdit::textChanged, this, [this, message]() {
            message->hide();
            regServerError_->hide();
        });
    };
    clearError(regName_, regNameError_);
    clearError(regEmail_, regEmailError_);
    clearError(regPassword_, regPasswordError_);
    clearError(regConfirm_, regConfirmError_);
    connect(regPassword_, &QLineEdit::textChanged, this, [this](const QString &value) {
        const bool strong = value.size() >= 8;
        regPasswordHint_->setStyleSheet(strong
            ? "color:#21745B; font-size:12px;"
            : "color:#738078; font-size:12px;");
    });

}

void AuthPage::resetForm() {
    loginPassword_->clear();
    setPasswordVisibility(loginPassword_, loginPasswordToggle_, false);
    regPassword_->clear();
    regConfirm_->clear();
    loginErrorContainer_->hide();
    switchAccountButton_->hide();
    goRegisterButton_->show();
    pages_->setCurrentIndex(0);
}
void AuthPage::doLogin() {
    QString error;
    const Session session = AuthService::login(loginName_->text(), loginPassword_->text(), &error);
    if (!session.valid()) {
        // AuthService currently returns this exact error when a valid-password
        // login is attempted for an account with Status != ACTIVE.
        const bool locked = (error == QStringLiteral("Tai khoan da bi khoa. Lien he quan tri vien."));
        loginErrorIcon_->setText(locked ? QStringLiteral("🔒") : QStringLiteral("ⓘ"));
        loginErrorTitle_->setText(locked ? QStringLiteral("Account locked") : QString());
        loginErrorTitle_->setVisible(locked);
        loginErrorDetail_->setText(locked
            ? QStringLiteral("You cannot access this account right now. Please contact an administrator for help.")
            : QStringLiteral("The username or password is incorrect. Please check and try again."));
        loginErrorContainer_->show();
        switchAccountButton_->setVisible(locked);
        goRegisterButton_->setVisible(!locked);
        return;
    }
    loginErrorContainer_->hide();
    switchAccountButton_->hide();
    goRegisterButton_->show();
    loginPassword_->clear();
    setPasswordVisibility(loginPassword_, loginPasswordToggle_, false);
    emit authenticated(session);
}
void AuthPage::doRegister() {
    // Front-end checks match the Figma Sign up validation screen.
    for (QLabel *message : {regNameError_, regEmailError_, regPasswordError_,
                            regConfirmError_, regServerError_}) {
        message->hide();
    }
    const auto showError = [](QLabel *target, const QString &message) {
        target->setText(message);
        target->show();
    };

    const QString name = regName_->text().trimmed();
    const QString email = regEmail_->text().trimmed();
    const QString password = regPassword_->text();
    static const QRegularExpression emailPattern(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");
    bool valid = true;
    if (name.size() < 3 || name.size() > 50) {
        showError(regNameError_, "Username must contain 3–50 characters.");
        valid = false;
    }
    if (!emailPattern.match(email).hasMatch() || email.size() > 100) {
        showError(regEmailError_, "Enter a valid email address, e.g. an.nguyen@gmail.com.");
        valid = false;
    }
    if (password.size() < 8) {
        showError(regPasswordError_, "Password must be at least 8 characters.");
        valid = false;
    }
    
    if (password != regConfirm_->text()) {
        showError(regConfirmError_, "Passwords do not match. Please try again.");
        valid = false;
    }
    if (!termsCheck_->isChecked()) {
        showError(regServerError_, "Please accept the Terms of Use and Privacy Policy to continue.");
        valid = false;
    }
    if (!valid) return;

    QString error;
    if (!AuthService::registerUser(name, email, password, &error)) {
        // Keep internal database errors out of the UI.
        showError(regServerError_,
            "Unable to create an account. The username or email may already be in use. "
            "Please check your details and try again.");
        return;
    }

    loginName_->setText(name);
    loginPassword_->clear();
    regSuccessDescription_->setText(
        QString("Welcome to Phonebook, %1. Your account is ready. "
                "Log in to start connecting.").arg(name));
    regPassword_->clear();
    regConfirm_->clear();
    pages_->setCurrentIndex(2);
}
