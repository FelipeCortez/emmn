#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QItemSelection>
#include <QStringListModel>
#include "trackerlistmodel.h"
#include "addtrackerdialog.h"
#include "tracker.h"
#include "settings.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    AddTrackerDialog addTrackerDialog;
    Settings settings;
    TrackerListModel *model;
    QList<Tracker> trackers;

public slots:
   void rowChangedSlot(QItemSelection selected, QItemSelection);
   void persistenceChangedSlot(const QString text);
   void addTrackerDialogSlot();
   void acceptedTleSlot(int);
};

#endif // MAINWINDOW_H
