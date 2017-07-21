#include "manualcontroldialog.h"
#include "ui_manualcontrol_dialog.h"

ManualControlDialog::ManualControlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualControlDialog)
{
    ui->setupUi(this);
}

ManualControlDialog::~ManualControlDialog()
{
    delete ui;
}
