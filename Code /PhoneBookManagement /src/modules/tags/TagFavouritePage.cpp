#include "TagFavouritePage.h"
#include "TagFavouriteService.h"
#include "../contacts/ContactService.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QTableWidget>
#include <QTabWidget>
#include <QStringList>
#include <QVBoxLayout>

TagFavouritePage::TagFavouritePage(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel("Tags & Favourites");
    title->setObjectName("heading");
    layout->addWidget(title);
    auto *tabs = new QTabWidget;
    auto *tagsTab = new QWidget;
    auto *favTab = new QWidget;
    buildTagsTab(tagsTab);
    buildFavouritesTab(favTab);
    tabs->addTab(tagsTab, "Manage tags");
    tabs->addTab(favTab, "Favourites");
    layout->addWidget(tabs);
}

void TagFavouritePage::buildTagsTab(QWidget *tab) {
    auto *outer = new QVBoxLayout(tab);
    outer->setContentsMargins(4,4,4,4);
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *inner = new QWidget;
    auto *layout = new QVBoxLayout(inner);
    scroll->setWidget(inner);
    outer->addWidget(scroll);
    tags_ = new QListWidget;
    tags_->setMinimumHeight(155);
    tags_->setMaximumHeight(190);
    layout->addWidget(tags_);
    auto *controls = new QVBoxLayout;
    tagName_ = new QLineEdit;
    tagName_->setPlaceholderText("Tag name...");
    auto *create = new QPushButton("Create");
    auto *rename = new QPushButton("Rename selected");
    auto *remove = new QPushButton("Delete selected");
    controls->addWidget(tagName_);
    auto *buttons = new QHBoxLayout;
    create->setText("Create");
    rename->setText("Rename");
    remove->setText("Delete");
    buttons->addWidget(create);
    buttons->addWidget(rename);
    buttons->addWidget(remove);
    controls->addLayout(buttons);
    layout->addLayout(controls);
    layout->addWidget(new QLabel("Assign tag to contact:"));
    auto *assignControls = new QVBoxLayout;
    tagCombo_ = new QComboBox;
    tagContact_ = new QComboBox;
    auto *assign = new QPushButton("Assign");
    assignControls->addWidget(tagCombo_);
    assignControls->addWidget(tagContact_);
    assignControls->addWidget(assign);
    layout->addLayout(assignControls);
    assignments_ = new QTableWidget(0, 4);
    assignments_->setHorizontalHeaderLabels({"TagID", "ContactID", "Tag", "Contact"});
    assignments_->setColumnHidden(0, true);
    assignments_->setColumnHidden(1, true);
    assignments_->setSelectionBehavior(QAbstractItemView::SelectRows);
    assignments_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    assignments_->horizontalHeader()->setStretchLastSection(true);
    assignments_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    assignments_->setMinimumHeight(170);
    assignments_->setMaximumHeight(210);
    layout->addWidget(assignments_);
    auto *unassign = new QPushButton("Unassign selected tag");
    layout->addWidget(unassign);
    connect(tags_, &QListWidget::itemSelectionChanged, this, [this]() {
        if (tags_->currentItem()) tagName_->setText(tags_->currentItem()->text());
    });
    connect(create, &QPushButton::clicked, this, [this]() {
        QString error;
        if (!TagFavouriteService::createTag(userId_, tagName_->text(), &error))
            QMessageBox::warning(this, "Tag", error);
        else { tagName_->clear(); refresh(); }
    });
    connect(rename, &QPushButton::clicked, this, [this]() {
        QString error;
        if (!TagFavouriteService::renameTag(userId_, selectedTagId(), tagName_->text(), &error))
            QMessageBox::warning(this, "Tag", error);
        else refresh();
    });
    connect(remove, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, "Delete tag", "Delete selected tag?") != QMessageBox::Yes) return;
        QString error;
        if (!TagFavouriteService::deleteTag(userId_, selectedTagId(), &error))
            QMessageBox::warning(this, "Tag", error);
        else refresh();
    });
    connect(assign, &QPushButton::clicked, this, [this]() {
        QString error;
        if (!TagFavouriteService::assignTag(userId_, tagCombo_->currentData().toInt(),
                                             tagContact_->currentData().toInt(), &error))
            QMessageBox::warning(this, "Assign tag", error);
        else refresh();
    });
    connect(unassign, &QPushButton::clicked, this, [this]() {
        const int r = assignments_->currentRow();
        if (r < 0) return;
        QString error;
        if (!TagFavouriteService::unassignTag(userId_,
            assignments_->item(r,0)->text().toInt(), assignments_->item(r,1)->text().toInt(), &error))
            QMessageBox::warning(this, "Unassign tag", error);
        else refresh();
    });
}

void TagFavouritePage::buildFavouritesTab(QWidget *tab) {
    auto *layout = new QVBoxLayout(tab);
    auto *controls = new QVBoxLayout;
    favContact_ = new QComboBox;
    auto *add = new QPushButton("Add to favourites");
    controls->addWidget(favContact_);
    controls->addWidget(add);
    layout->addLayout(controls);
    favourites_ = new QTableWidget(0, 3);
    favourites_->setHorizontalHeaderLabels({"ContactID", "Full name", "Phone"});
    favourites_->setColumnHidden(0, true);
    favourites_->setSelectionBehavior(QAbstractItemView::SelectRows);
    favourites_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    favourites_->horizontalHeader()->setStretchLastSection(true);
    favourites_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    layout->addWidget(favourites_);
    auto *remove = new QPushButton("Remove selected favourite");
    layout->addWidget(remove);
    connect(add, &QPushButton::clicked, this, [this]() {
        QString error;
        if (!TagFavouriteService::addFavourite(userId_, favContact_->currentData().toInt(), &error))
            QMessageBox::warning(this, "Favourite", error);
        else refresh();
    });
    connect(remove, &QPushButton::clicked, this, [this]() {
        QString error;
        if (!TagFavouriteService::removeFavourite(userId_, selectedFavouriteId(), &error))
            QMessageBox::warning(this, "Favourite", error);
        else refresh();
    });
}

int TagFavouritePage::selectedTagId() const {
    return tags_->currentItem() ? tags_->currentItem()->data(Qt::UserRole).toInt() : 0;
}
int TagFavouritePage::selectedFavouriteId() const {
    int r = favourites_->currentRow();
    return r >= 0 && favourites_->item(r,0) ? favourites_->item(r,0)->text().toInt() : 0;
}
void TagFavouritePage::setUserId(int userId) { userId_ = userId; refresh(); }
void TagFavouritePage::refresh() {
    tags_->clear(); tagCombo_->clear(); tagContact_->clear(); favContact_->clear();
    assignments_->setRowCount(0); favourites_->setRowCount(0);
    if (userId_ <= 0) return;
    for (const auto &tag : TagFavouriteService::tags(userId_)) {
        auto *item = new QListWidgetItem(tag.name, tags_);
        item->setData(Qt::UserRole, tag.id);
        tagCombo_->addItem(tag.name, tag.id);
    }
    for (const auto &c : ContactService::list(userId_)) {
        const QString display = c.fullName + " (" + c.personalPhone + ")";
        tagContact_->addItem(display, c.id);
        favContact_->addItem(display, c.id);
    }
    for (const auto &a : TagFavouriteService::assignments(userId_)) {
        const int r = assignments_->rowCount();
        assignments_->insertRow(r);
        QStringList values{QString::number(a.tagId), QString::number(a.contactId), a.tagName, a.contactName};
        for (int i=0; i<values.size(); ++i)
            assignments_->setItem(r,i,new QTableWidgetItem(values.at(i)));
    }
    for (const auto &c : TagFavouriteService::favourites(userId_)) {
        const int r = favourites_->rowCount();
        favourites_->insertRow(r);
        favourites_->setItem(r,0,new QTableWidgetItem(QString::number(c.id)));
        favourites_->setItem(r,1,new QTableWidgetItem(c.fullName));
        favourites_->setItem(r,2,new QTableWidgetItem(c.personalPhone));
    }
}
