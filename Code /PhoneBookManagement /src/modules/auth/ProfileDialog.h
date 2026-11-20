#pragma once
#include "../../core/Models.h"
#include <QDialog>

class ProfileDialog : public QDialog {
public:
    explicit ProfileDialog(const Session &session, QWidget *parent = nullptr);
};
