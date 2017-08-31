#ifndef ADDTRACKERDIALOG_H
#define ADDTRACKERDIALOG_H

#include <QDialog>
#include "ui_tracker_dialog.h"
#include "trackerlistmodel.h"
#include "tracker.h"

enum class TrackerDialogMode {
    New, Edit
};

class AddTrackerDialog : public QDialog, public Ui::AddTrackerDialog {
    Q_OBJECT

public:
    AddTrackerDialog(QWidget* parent = 0);
    AddTrackerDialog(TrackerListModel* model, QWidget* parent = 0);
    AddTrackerDialog(TrackerListModel* model, Tracker* tracker, QWidget* parent = 0);
private:
     TrackerDialogMode mode;
     Tracker* tracker;
     TrackerListModel* model;
public slots:
     void accept();

};

#endif // ADDTRACKERDIALOG_H
