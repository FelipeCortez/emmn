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
    explicit ManualControlDialog(QWidget *parent = 0);
    void setControlRef(Control* control);
    ~ManualControlDialog();

private:
    Ui::ManualControlDialog *ui;
    Control* control;
    QTimer joystickRefreshTimer;

public slots:
    void updateAntennaInfo();
};

#endif // MANUALCONTROLDIALOG_H
