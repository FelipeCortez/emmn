#ifndef ADDTRACKERDIALOG_H
#define ADDTRACKERDIALOG_H

#include <QDialog>
#include "ui_tracker_dialog.h"

class AddTrackerDialog : public QDialog, public Ui::AddTrackerDialog {
    Q_OBJECT

public:
     AddTrackerDialog(QWidget * parent = 0);
};

#endif // ADDTRACKERDIALOG_H
