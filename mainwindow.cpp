#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , addTrackerDialog(this),
    satInfoTimer(this),
    tableModel(nullptr)
{
    model = new TrackerListModel();

    ui->setupUi(this);
    ui->satellitesView->setModel(model);
    ui->satellitesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->passesView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    network.getTLE("TERRA");

    loadTrackersFromSettings();

    connect(ui->satellitesView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this,
            SLOT(rowChangedSlot(QItemSelection, QItemSelection)));
    connect(ui->addTrackerButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(addTrackerDialogSlot()));
    connect(ui->removeTrackerButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(removeSelectedTrackerSlot()));
    connect(&addTrackerDialog,
            SIGNAL(finished(int)),
            this,
            SLOT(acceptedTleSlot(int)));
    connect(&satInfoTimer,
            SIGNAL(timeout()),
            this,
            SLOT(satInfoUpdateSlot()));
}

void MainWindow::loadTrackersFromSettings() {
    auto trackers = settings.loadTrackers();
    for(auto t : trackers) {
        model->addTracker(t);
    }
}

void MainWindow::enableSatelliteButtons(bool enable) {
    ui->removeTrackerButton->setEnabled(enable);
    ui->editTrackerButton->setEnabled(enable);
}

QString betterDate(DateTime date) {
    return QString("%1/%2/%3 %4:%5:%6 UTC").arg(date.Year())
                                           .arg(date.Month(), 2, 10, QChar('0'))
                                           .arg(date.Day(), 2, 10, QChar('0'))
                                           .arg(date.Hour(), 2, 10, QChar('0'))
                                           .arg(date.Minute(), 2, 10, QChar('0'))
                                           .arg(date.Second(), 2, 10, QChar('0'));
}

void MainWindow::rowChangedSlot(QItemSelection selected, QItemSelection) {
    if(!selected.isEmpty()) {
        auto selectedIndex = selected.indexes().first();
        auto tracker = model->getTrackers()[selectedIndex.row()];
        enableSatelliteButtons();
        satInfoTimer.start(1000);
        satInfoUpdateSlot();

        QList<PassDetails> pd = selectedIndex.data(TrackerListModel::PassesRole).value<QList<PassDetails>>();
        // http://stackoverflow.com/a/11907059
        const int numRows = pd.size();
        const int numColumns = 1;

        if(tableModel) { delete tableModel; }
        tableModel = new QStandardItemModel(numRows, numColumns);
        tableModel->setHorizontalHeaderLabels(QStringList() << "AOS"
                                              << "LOS"
                                              << "Max elevation"
                                              << "Duration");

        if (pd.begin() == pd.end()) {
            qDebug() << "no passes found";
        } else {
            int row = 0;
            QList<PassDetails>::const_iterator itr = pd.begin();
            do {
                QString text;
                QStandardItem* item;

                text = betterDate(itr->aos);
                item = new QStandardItem(text);
                tableModel->setItem(row, 0, item);

                text = betterDate(itr->los);
                item = new QStandardItem(text);
                tableModel->setItem(row, 1, item);

                text = QString::number(Util::RadiansToDegrees(itr->max_elevation)) + QString("°");
                item = new QStandardItem(text);
                tableModel->setItem(row, 2, item);

                text = QString::fromStdString((itr->los - itr->aos).ToString());
                item = new QStandardItem(text);
                tableModel->setItem(row, 3, item);

                ++row;
            } while (++itr != pd.end());
        }

        ui->passesView->setModel(tableModel);
        ui->passesView->resizeColumnsToContents();
    } else {
        enableSatelliteButtons(false);
    }
}

void MainWindow::persistenceChangedSlot(const QString text) {
    settings.setPersistentString(text);
}

void MainWindow::addTrackerDialogSlot() {
    addTrackerDialog.tleInput->setWhatsThis("Two-line element set de um satélite. É possível adquiri-lo através de um site como CelesTrak (https://www.celestrak.com/)");
    addTrackerDialog.exec();
}

void MainWindow::removeSelectedTrackerSlot() {
    foreach(const QModelIndex &index, ui->satellitesView->selectionModel()->selectedIndexes()) {
        qDebug() << index.data(Qt::DisplayRole).toString();
        model->removeRow(index.row());
        settings.saveTrackers(model->getTrackers());
    }
}

void MainWindow::acceptedTleSlot(int) {
    //qDebug() << addTrackerDialog.tleInput->toPlainText();
    QStringList tle = addTrackerDialog.tleInput->toPlainText().split("\n");
    if(tle.size() == 3) { // Check if two line element set has three elements :)
        try {
            Tracker t(tle);
            model->addTracker(t);
            settings.saveTrackers(model->getTrackers());
        } catch(TleException e) {
            qDebug() << "TLE inválida";
        }

        //model->setData();
    } else {
        qDebug() << "Wrong";
    }
}

void MainWindow::satInfoUpdateSlot() {
    auto selected = ui->satellitesView->selectionModel()->selection();
    if(!selected.isEmpty()) {
        auto selectedIndex = selected.indexes().first();
        auto tracker = model->getTrackers()[selectedIndex.row()];

        ui->satElevation->setText(tracker.getSatInfo(Tracker::Elevation));
        ui->satAzimuth->setText(tracker.getSatInfo(Tracker::Azimuth));
        ui->satNextPass->setText(tracker.nextPass());
    } else {
        satInfoTimer.stop();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
