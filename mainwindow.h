#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QItemSelection>
#include <QStringListModel>
#include <QStandardItemModel>
#include "trackerlistmodel.h"
#include "addtrackerdialog.h"
#include "tracker.h"
#include "settings.h"
#include "network.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void enableSatelliteButtons(bool enable = true);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    AddTrackerDialog addTrackerDialog;
    Settings settings;
    Network network;
    TrackerListModel *model;
    void loadTrackersFromSettings();
    QTimer satInfoTimer;
    QStandardItemModel* tableModel;

public slots:
   void rowChangedSlot(QItemSelection selected, QItemSelection);
   void persistenceChangedSlot(const QString text);
   void addTrackerDialogSlot();
   void removeSelectedTrackerSlot();
   void acceptedTleSlot(int);
   void satInfoUpdateSlot();
};

QString betterDate(DateTime datetime);

#endif // MAINWINDOW_H
