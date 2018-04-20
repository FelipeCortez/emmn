#include "manualcontroldialog.h"
#include "ui_manualcontroldialog.h"
#include <QTime>

ManualControlDialog::ManualControlDialog(Control* control, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManualControlDialog)
    , control(control)
    , antennaInfoUpdateTimer(this)
    , joystickRefreshTimer(this)
    , validAz(false)
    , validEle(false)
{
    ui->setupUi(this);

    ui->sendBothButton->setEnabled(false);

    antennaInfoUpdateTimer.start(100);
    joystickRefreshTimer.start(joystickRefreshRate);

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
    connect(ui->sendBothButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(sendBoth()));
    connect(ui->elevationLineEdit,
            SIGNAL(textChanged(QString)),
            this,
            SLOT(validateElevation(QString)));
    connect(ui->azimuthLineEdit,
            SIGNAL(textChanged(QString)),
            this,
            SLOT(validateAzimuth(QString)));

}

void ManualControlDialog::tabChanged(int index) {
    if (index == 0) {
        joystickRefreshTimer.start(joystickRefreshRate);
    } else if (index == 1) {
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

void ManualControlDialog::validateBoth() {
    ui->sendBothButton->setEnabled(validAz && validEle);
}

void ManualControlDialog::validateAzimuth(QString azimuth) {
    //! @todo Levar em conta diferença entre azimute mecânico e azimute geográfico

    bool ok;
    double az = azimuth.toDouble(&ok);
    if (ok && az >= 0 && az < 360) {
        validAz = true;
    } else {
        validAz = false;
    }

    validateBoth();
}

void ManualControlDialog::validateElevation(QString elevation) {
    bool ok;
    double ele = elevation.toDouble(&ok);
    if (ok && ele >= 0 && ele < 360) {
        validEle = true;
    } else {
        validEle = false;
    }

    validateBoth();
}

ManualControlDialog::~ManualControlDialog() {
    delete ui;
}
