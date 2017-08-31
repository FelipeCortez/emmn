#include "settingsdialog.h"
#include "helpers.h"
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);

    QList<QSerialPortInfo> serialList = Helpers::getSerialPortsAvailable();
    for(auto serialPort : serialList) {
        serialPortsCombo->addItem(serialPort.portName() + " (" + serialPort.description() + ")",
                                  serialPort.portName());

        if(Settings::getSerialPort() == serialPort.portName()) {
            serialPortsCombo->setCurrentIndex(serialPortsCombo->count() - 1);
        }
    }

    updateWithSettings();

    connect(settingsDialogButtonBox,
            SIGNAL(accepted()),
            this,
            SLOT(accept()));
    connect(settingsDialogButtonBox,
            SIGNAL(rejected()),
            this,
            SLOT(reject()));
}

void SettingsDialog::updateWithSettings() {
    useLocalTimeCheckbox->setChecked(Settings::getUseLocalTime());
}

void SettingsDialog::accept() {
    Settings::setUseLocalTime(useLocalTimeCheckbox->isChecked());

    Settings::setSerialPort(serialPortsCombo->currentData().toString());
    //setPortFromSettings();

    QDialog::accept();
}
