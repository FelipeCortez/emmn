#include "manualcontroldialog.h"
#include "ui_manualcontrol_dialog.h"

ManualControlDialog::ManualControlDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManualControlDialog)
    , control(nullptr)
    , joystickRefreshTimer(this)
{
    ui->setupUi(this);
    ui->azimuthEdit->setFocus(Qt::OtherFocusReason);

    joystickRefreshTimer.start(1000.0f / 60);

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
    connect(&joystickRefreshTimer,
            SIGNAL(timeout()),
            ui->joystickWidget,
            SLOT(refreshSlot()));
    connect(&joystickRefreshTimer,
            SIGNAL(timeout()),
            this,
            SLOT(updateAntennaInfo()));
}

void ManualControlDialog::setControlRef(Control* control) {
    this->control = control;
}

void ManualControlDialog::updateAntennaInfo() {
    if(control != nullptr) {
        AzEle antennaInfo = control->send_state();
        ui->azimuthLabel->setText(QString::number(antennaInfo.azimuth));
        ui->elevationLabel->setText(QString::number(antennaInfo.elevation));
        AzEle antennaDeltas = ui->joystickWidget->getDeltas();
        control->setDeltas(antennaDeltas.azimuth, antennaDeltas.elevation);
    }
}

ManualControlDialog::~ManualControlDialog()
{
    delete ui;
}
