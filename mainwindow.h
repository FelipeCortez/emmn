#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QItemSelection>
#include <QStringListModel>
#include <QStandardItemModel>
#include "trackerlistmodel.h"
#include "addtrackerdialog.h"
#include "settingsdialog.h"
#include "manualcontroldialog.h"
#include "tracker.h"
#include "helpers.h"
#include "network.h"
#include "control.h"

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
    Ui::MainWindow* ui;
    AddTrackerDialog addTrackerDialog;
    SettingsDialog settingsDialog;
    ManualControlDialog manualControlDialog;
    TrackerListModel* model;
    QTimer satInfoTimer;
    QStandardItemModel* tableModel;
    Network network;
    Control* control;
    int counter;

    void setPortFromSettings();
    void loadTrackersFromSettings();
    QString betterDate(DateTime datetime);

public slots:
   void rowChangedSlot(QItemSelection selected, QItemSelection);
   void addTrackerDialogSlot();
   void settingsDialogSlot(bool);
   void manualControlDialogSlot(bool);
   void debugarSlot(bool);
   void editSelectedTrackerSlot();
   void removeSelectedTrackerSlot();
   void acceptedTleSlot(int confirm);
   void acceptedSettingsSlot(int confirm);
   void satInfoUpdateSlot();
   void clearSelectedTrackerSlot();
   void moveTrackerUpSlot();
   void moveTrackerDownSlot();
};

#endif // MAINWINDOW_H
