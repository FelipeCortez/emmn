#include "manualcontroldialog.h"
#include "ui_manualcontrol_dialog.h"
#include <QTime>

ManualControlDialog::ManualControlDialog(Control* control, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManualControlDialog)
    , control(control)
    , antennaInfoUpdateTimer(this)
    , joystickRefreshTimer(this)
{
    ui->setupUi(this);

    antennaInfoUpdateTimer.start(1000.0f / 30);
    joystickRefreshTimer.start(1000.0f / 30);

    // Tab --------
    connect(ui->controlSourceTab,
            SIGNAL(currentChanged(int)),
            this,
            SLOT(tabChanged(int)));
    // Joystick ---
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
            SLOT(sendJoystickDeltas()));
    connect(&antennaInfoUpdateTimer,
            SIGNAL(timeout()),
            this,
            SLOT(updateAntennaInfo()));
    // Form ------
    connect(ui->sendAzButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(sendAz()));
    connect(ui->sendEleButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(sendEle()));
    connect(ui->sendBothButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(sendBoth()));
}

void ManualControlDialog::tabChanged(int index) {
    if(index == 0) {
        joystickRefreshTimer.start(1000.0f / 30);
    } else if(index == 1) {
        joystickRefreshTimer.stop();
        ui->azimuthLineEdit->setFocus(Qt::OtherFocusReason);
    }
}

void ManualControlDialog::updateAntennaInfo() {
    AzEle antennaInfo = control->getState();
    ui->azimuthLabel->setText(QString::number(antennaInfo.azimuth));
    ui->elevationLabel->setText(QString::number(antennaInfo.elevation));
}

void ManualControlDialog::sendJoystickDeltas() {
    AzEle antennaDeltas = ui->joystickWidget->getDeltas();
    control->setDeltas(antennaDeltas.azimuth, antennaDeltas.elevation);
}

void ManualControlDialog::sendBoth() {
    control->setTarget(ui->azimuthLineEdit->text().toDouble(),
                       ui->elevationLineEdit->text().toDouble());
}

void ManualControlDialog::sendAz() {
    control->setTarget(ui->azimuthLineEdit->text().toDouble(),
                       control->getState().elevation);
}

void ManualControlDialog::sendEle() {
    control->setTarget(control->getState().azimuth,
                       ui->elevationLineEdit->text().toDouble());
}

ManualControlDialog::~ManualControlDialog() {
    delete ui;
}
