#include "ProfileDialog.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>

ProfileDialog::ProfileDialog(const Session &session, QWidget *parent) : QDialog(parent) {
    setWindowTitle("Profile");
    setMinimumWidth(360);
    auto *form = new QFormLayout(this);
    form->addRow("UserID", new QLabel(QString::number(session.userId)));
    form->addRow("UserName", new QLabel(session.username));
    form->addRow("Email", new QLabel(session.email));
    form->addRow("Role", new QLabel(session.role));
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
