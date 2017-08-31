#ifndef MANUALCONTROLDIALOG_H
#define MANUALCONTROLDIALOG_H

#include <QDialog>
#include "control.h"
#include "helpers.h"

namespace Ui {
class ManualControlDialog;
}

class ManualControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualControlDialog(Control *control, QWidget *parent = 0);
    ~ManualControlDialog();

private:
    Ui::ManualControlDialog *ui;
    Control* control;
    QTimer antennaInfoUpdateTimer;
    QTimer joystickRefreshTimer;

public slots:
    void tabChanged(int index);
    void updateAntennaInfo();
    void sendJoystickDeltas();
    void sendAz();
    void sendEle();
    void sendBoth();
};

#endif // MANUALCONTROLDIALOG_H
