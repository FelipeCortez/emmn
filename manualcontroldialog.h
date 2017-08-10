#ifndef MANUALCONTROLDIALOG_H
#define MANUALCONTROLDIALOG_H

#include <QDialog>
#include "control.h"

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
};

#endif // MANUALCONTROLDIALOG_H
