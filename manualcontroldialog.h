#ifndef MANUALCONTROLDIALOG_H
#define MANUALCONTROLDIALOG_H

#include <QDialog>

namespace Ui {
class ManualControlDialog;
}

class ManualControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualControlDialog(QWidget *parent = 0);
    ~ManualControlDialog();

private:
    Ui::ManualControlDialog *ui;
};

#endif // MANUALCONTROLDIALOG_H
