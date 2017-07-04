#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QItemSelection>
#include <QStringListModel>
#include <QStandardItemModel>
#include "trackerlistmodel.h"
#include "addtrackerdialog.h"
#include "settingsdialog.h"
#include "tracker.h"
#include "settings.h"
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
    Ui::MainWindow *ui;
    AddTrackerDialog addTrackerDialog;
    SettingsDialog settingsDialog;
    TrackerListModel *model;
    QTimer satInfoTimer;
    QStandardItemModel* tableModel;
    Network network;
    Control control;

    void loadTrackersFromSettings();
    QString betterDate(DateTime datetime);

public slots:
   void rowChangedSlot(QItemSelection selected, QItemSelection);
   void addTrackerDialogSlot();
   void settingsDialogSlot(bool);
   void editSelectedTrackerSlot();
   void removeSelectedTrackerSlot();
   void acceptedTleSlot(int confirm);
   void acceptedSettingsSlot(int confirm);
   void satInfoUpdateSlot();
};

#endif // MAINWINDOW_H
