#include "settingsdialog.h"
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget * parent) :
    QDialog(parent)
{
    setupUi(this);
}

void SettingsDialog::updateWithSettings(Settings* settings) {
    useLocalTimeCheckbox->setChecked(settings->getUseLocalTime());
}
