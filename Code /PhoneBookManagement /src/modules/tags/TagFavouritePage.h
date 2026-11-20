#pragma once
#include <QWidget>
class QComboBox;
class QLineEdit;
class QListWidget;
class QTableWidget;

class TagFavouritePage : public QWidget {
public:
    explicit TagFavouritePage(QWidget *parent = nullptr);
    void setUserId(int userId);
    void refresh();
private:
    int userId_ = 0;
    QListWidget *tags_;
    QLineEdit *tagName_;
    QComboBox *tagCombo_, *tagContact_, *favContact_;
    QTableWidget *assignments_, *favourites_;
    int selectedTagId() const;
    int selectedFavouriteId() const;
    void buildTagsTab(QWidget *tab);
    void buildFavouritesTab(QWidget *tab);
};
