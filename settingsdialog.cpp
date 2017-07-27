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
}

void SettingsDialog::updateWithSettings() {
    useLocalTimeCheckbox->setChecked(Settings::getUseLocalTime());
}
