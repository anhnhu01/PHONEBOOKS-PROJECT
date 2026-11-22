#include "ContactsPage.h"
#include "ContactService.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace {
bool editContactDialog(QWidget *parent, Contact *c) {
    QDialog dialog(parent);
    dialog.setWindowTitle(c->id ? "Edit contact" : "Add contact");
    dialog.resize(380, 680);
    dialog.setMinimumWidth(340);
    auto *outer = new QVBoxLayout(&dialog);
    outer->setContentsMargins(14,14,14,14);
    auto *scroll = new QScrollArea;
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    auto *contents = new QWidget;
    auto *form = new QVBoxLayout(contents);
    form->setSpacing(5);
    auto *hint = new QLabel("Full name is required. Enter at least one phone number or email.");
    hint->setObjectName("subtleText");
    hint->setWordWrap(true);
    form->addWidget(hint);
    auto make = [&](const QString &title, const QString &value) {
        auto *label = new QLabel(title);
        label->setObjectName("fieldLabel");
        form->addWidget(label);
        auto *input = new QLineEdit(value);
        input->setMinimumHeight(44);
        form->addWidget(input);
        form->addSpacing(8);
        return input;
    };
    auto *name = make("Full name *", c->fullName);
    auto *phone = make("Personal phone", c->personalPhone);
    auto *workPhone = make("Work phone", c->workPhone);
    auto *email = make("Personal email", c->personalEmail);
    auto *workEmail = make("Work email", c->workEmail);
    auto *address = make("Address", c->address);
    auto *gender = make("Gender", c->gender);
    auto *dob = make("Date of birth (YYYY-MM-DD)", c->dateOfBirth);
    auto *company = make("Company", c->company);
    auto *occupation = make("Occupation", c->occupation);
    auto *notes = make("Notes", c->notes);
    form->addStretch();
    scroll->setWidget(contents);
    outer->addWidget(scroll);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    outer->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return false;
    c->fullName = name->text();
    c->personalPhone = phone->text();
    c->workPhone = workPhone->text();
    c->personalEmail = email->text();
    c->workEmail = workEmail->text();
    c->address = address->text();
    c->gender = gender->text();
    c->dateOfBirth = dob->text();
    c->company = company->text();
    c->occupation = occupation->text();
    c->notes = notes->text();
    return true;
}
}

ContactsPage::ContactsPage(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24,8,24,8);
    layout->setSpacing(12);
    auto *headingRow = new QHBoxLayout;
    auto *title = new QLabel("Contacts");
    title->setObjectName("heading");
    headingRow->addWidget(title,1);
    auto *add = new QPushButton("+");
    add->setObjectName("addContactButton");
    add->setFixedSize(42,42);
    headingRow->addWidget(add);
    layout->addLayout(headingRow);
    count_ = new QLabel("0 contacts · Stay connected");
    count_->setObjectName("subtleText");
    layout->addWidget(count_);
    search_ = new QLineEdit;
    search_->setPlaceholderText("Search name, phone or email...");
    search_->setMinimumHeight(48);
    layout->addWidget(search_);
    auto *section = new QLabel("ALL CONTACTS");
    section->setObjectName("sectionHeading");
    layout->addWidget(section);
    list_ = new QListWidget;
    list_->setObjectName("contactsList");
    list_->setFrameShape(QFrame::NoFrame);
    list_->setSelectionMode(QAbstractItemView::SingleSelection);
    list_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    list_->setSpacing(0);
    layout->addWidget(list_,1);

    // Edit and delete buttons.
    auto *tools = new QHBoxLayout;
    auto *edit = new QPushButton("Edit selected");
    auto *del = new QPushButton("Delete selected");
    edit->setObjectName("secondaryButton");
    del->setObjectName("secondaryButton");
    tools->addWidget(edit,1);
    tools->addWidget(del,1);
    layout->addLayout(tools);
    connect(search_, &QLineEdit::textChanged, this, [this]() { refresh(); });
    connect(add, &QPushButton::clicked, this, [this]() { addContact(); });
    connect(edit, &QPushButton::clicked, this, [this]() { editContact(); });
    connect(del, &QPushButton::clicked, this, [this]() { deleteContact(); });
    connect(list_, &QListWidget::itemDoubleClicked, this, [this]() { editContact(); });
}

void ContactsPage::setUserId(int userId) {
    userId_ = userId;
    search_->clear();
    refresh();
}
void ContactsPage::refresh() {
    list_->clear();
    if (!userId_) { count_->setText("0 contacts"); return; }
    const auto contacts = ContactService::list(userId_, search_->text());
    count_->setText(QString("%1 contacts · Stay connected").arg(contacts.size()));
    for (const auto &c : contacts) {
        auto *item = new QListWidgetItem(list_);
        item->setData(Qt::UserRole, c.id);
        item->setSizeHint(QSize(290,76));
        auto *row = new QWidget;
        row->setObjectName("contactRow");
        auto *rl = new QHBoxLayout(row);
        rl->setContentsMargins(0,5,0,5);
        rl->setSpacing(12);
        auto *avatar = new QLabel(c.fullName.left(1).toUpper());
        avatar->setObjectName("contactAvatar");
        avatar->setAlignment(Qt::AlignCenter);
        avatar->setFixedSize(48,48);
        rl->addWidget(avatar);
        auto *labels = new QVBoxLayout;
        labels->setSpacing(2);
        auto *name = new QLabel(c.fullName);
        name->setObjectName("contactName");
        auto *details = new QLabel((c.company.isEmpty() ? QString() : c.company + " · ") + c.personalPhone);
        details->setObjectName("subtleText");
        details->setWordWrap(true);
        labels->addWidget(name);
        labels->addWidget(details);
        rl->addLayout(labels,1);
        auto *chevron = new QLabel("›");
        chevron->setObjectName("subtleText");
        rl->addWidget(chevron);
        row->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        list_->setItemWidget(item, row);
    }
}
int ContactsPage::selectedId() const {
    return list_->currentItem() ? list_->currentItem()->data(Qt::UserRole).toInt() : 0;
}
void ContactsPage::addContact() {
    if (!userId_) return;
    Contact c; c.userId = userId_;
    if (!editContactDialog(this, &c)) return;
    QString error;
    if (!ContactService::validate(c, &error)) { QMessageBox::warning(this,"Add failed",error); return; }
    switch (ContactService::checkDuplicate(c)) {
    case ContactService::Duplicate::SameContact:
        QMessageBox::warning(this,"Duplicate contact","This contact may already exist, so it was not added.");
        return;
    case ContactService::Duplicate::SharedInfo:
        if (QMessageBox::question(this,"Duplicate info",
                "This phone number or email already belongs to another contact with a different name.\nAdd anyway?")
                != QMessageBox::Yes) return;
        break;
    case ContactService::Duplicate::None:
        break;
    }
    if (!ContactService::add(c, &error)) { QMessageBox::warning(this,"Add failed",error); return; }
    refresh();
    QMessageBox::information(this,"Success","Contact added successfully.");
}
void ContactsPage::editContact() {
    const int id=selectedId();
    if (!id) { QMessageBox::information(this,"Edit","Choose a contact first."); return; }
    Contact c;
    if (!ContactService::get(userId_,id,&c)) return;
    if (!editContactDialog(this,&c)) return;
    QString error;
    if (!ContactService::update(userId_,c,&error)) { QMessageBox::warning(this,"Edit failed",error); return; }
    refresh();
}
void ContactsPage::deleteContact() {
    const int id=selectedId();
    if (!id) { QMessageBox::information(this,"Delete","Choose a contact first."); return; }
    if (QMessageBox::question(this,"Confirm delete","Delete selected contact?")!=QMessageBox::Yes) return;
    QString error;
    if (!ContactService::remove(userId_,id,&error)) { QMessageBox::warning(this,"Delete failed",error); return; }
    refresh();
}
