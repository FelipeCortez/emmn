#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_settings_dialog.h"
#include "settings.h"

class SettingsDialog : public QDialog, public Ui::SettingsDialog {
    Q_OBJECT

public:
     SettingsDialog(QWidget * parent = 0);
     void updateWithSettings(Settings *settings);
};

#endif // SETTINGSDIALOG_H
