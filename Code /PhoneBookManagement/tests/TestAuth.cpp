#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "../src/modules/auth/AuthService.h"

class TestAuth : public QObject {
    Q_OBJECT

private slots:
    // Chạy trước mỗi test case để tạo DB sạch trong RAM
    void init() {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        QSqlQuery q;
        QVERIFY(q.exec(R"(
            CREATE TABLE "USER" (
                UserID INTEGER PRIMARY KEY AUTOINCREMENT,
                UserName TEXT UNIQUE NOT NULL,
                UserEmail TEXT UNIQUE NOT NULL,
                PasswordHash TEXT NOT NULL,
                PasswordSalt TEXT NOT NULL,
                Role TEXT DEFAULT 'USER',
                Status TEXT DEFAULT 'ACTIVE'
            )
        )"));
    }

    // Chạy sau mỗi test case để dọn DB
    void cleanup() {
        db.close();
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }

    // TC01_auth: Đăng ký thành công với dữ liệu hợp lệ
    void testTC01_RegisterSuccess() {
        QString error;
        bool result = AuthService::registerUser("chisbaor", "bao@gmail.com", "12345678", &error);
        QVERIFY2(result, qPrintable(error));
    }

    // TC02_auth: Đăng ký thất bại do mật khẩu dưới 8 ký tự
    void testTC02_RegisterFailShortPassword() {
        QString error;
        bool result = AuthService::registerUser("chisbaor", "bao@gmail.com", "1234567", &error);
        QVERIFY(!result);
    }

    // TC03_auth: Đăng ký thất bại do trùng Username
    void testTC03_RegisterFailDuplicateUser() {
        QString error;
        QVERIFY(AuthService::registerUser("chisbaor", "bao1@gmail.com", "12345678", &error));
        
        bool duplicateResult = AuthService::registerUser("chisbaor", "bao2@gmail.com", "12345678", &error);
        QVERIFY(!duplicateResult);
    }

    // TC04_auth: Đăng nhập thành công bằng Username và Email
    void testTC04_LoginSuccess() {
        QString error;
        QVERIFY(AuthService::registerUser("chisbaor", "bao@gmail.com", "matkhau123", &error));

        Session s1 = AuthService::login("chisbaor", "matkhau123", &error);
        QVERIFY(s1.valid());
        QCOMPARE(s1.username, QString("chisbaor"));
        QCOMPARE(s1.role, QString("USER"));

        Session s2 = AuthService::login("bao@gmail.com", "matkhau123", &error);
        QVERIFY(s2.valid());
    }

    // TC05_auth: Đăng nhập thất bại do sai mật khẩu
    void testTC05_LoginFailWrongPassword() {
        QString error;
        QVERIFY(AuthService::registerUser("chisbaor", "bao@gmail.com", "matkhau123", &error));

        Session s = AuthService::login("chisbaor", "saipass123", &error);
        QVERIFY(!s.valid());
        QCOMPARE(error, QString("Sai ten dang nhap hoac mat khau."));
    }

    // TC06_auth: Đăng nhập thất bại khi tài khoản bị khóa
    void testTC06_LoginFailLockedAccount() {
        QString error;
        QVERIFY(AuthService::registerUser("lockeduser", "lock@gmail.com", "matkhau123", &error));

        QSqlQuery q;
        QVERIFY(q.exec(R"(UPDATE "USER" SET Status='LOCKED' WHERE UserName='lockeduser')"));

        Session s = AuthService::login("lockeduser", "matkhau123", &error);
        QVERIFY(!s.valid());
        QCOMPARE(error, QString("Tai khoan da bi khoa. Lien he quan tri vien."));
    }

private:
    QSqlDatabase db;
};

QTEST_MAIN(TestAuth)
#include "TestAuth.moc"