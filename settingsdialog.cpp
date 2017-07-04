#include "settingsdialog.h"
#include "helpers.h"
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget * parent) :
    QDialog(parent)
{
    setupUi(this);
}

void SettingsDialog::updateWithSettings() {
    useLocalTimeCheckbox->setChecked(Settings::getUseLocalTime());
}
