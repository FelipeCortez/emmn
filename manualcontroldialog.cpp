#include "manualcontroldialog.h"
#include "ui_manualcontrol_dialog.h"

ManualControlDialog::ManualControlDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManualControlDialog)
{
    ui->setupUi(this);

    ui->azimuthEdit->setFocus(Qt::OtherFocusReason);

    connect(ui->freeRadio,
            SIGNAL(toggled(bool)),
            ui->joystickWidget,
            SLOT(setFreeSlot(bool)));
    connect(ui->azimuthRadio,
            SIGNAL(toggled(bool)),
            ui->joystickWidget,
            SLOT(setAzimuthSlot(bool)));
    connect(ui->elevationRadio,
            SIGNAL(toggled(bool)),
            ui->joystickWidget,
            SLOT(setElevationSlot(bool)));
}

ManualControlDialog::~ManualControlDialog()
{
    delete ui;
}
