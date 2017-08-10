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

void ManualControlDialog::setControlRef(Control* control) {
    this->control = control;
    QMap<QString, float> answerMap = control->send_state();
    ui->azimuthLabel->setText(QString::number(answerMap.value("az")));
}

ManualControlDialog::~ManualControlDialog()
{
    delete ui;
}
