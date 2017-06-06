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

    addTrackerDialog.tleInput->setWhatsThis("Two-line element set de um satélite. É possível adquiri-lo através de um site como CelesTrak (https://www.celestrak.com/)");

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
    connect(ui->editTrackerButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(editSelectedTrackerSlot()));
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
        ui->satelliteGroupBox->setTitle(tracker.getTitle());

        QList<PassDetails> pd = selectedIndex.data(TrackerListModel::PassesRole).value<QList<PassDetails>>();
        // http://stackoverflow.com/a/11907059
        const int numRows = pd.size();
        const int numColumns = 1;

        if(tableModel) { delete tableModel; }
        tableModel = new QStandardItemModel(numRows, numColumns);
        tableModel->setHorizontalHeaderLabels(QStringList() << "Aquisição de sinal"
                                              << "Perda de sinal"
                                              << "Elevação máxima"
                                              << "Duração");

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

        ui->passesView->verticalHeader()->setDefaultSectionSize(ui->passesView->verticalHeader()->fontMetrics().height()+2);
        ui->passesView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui->passesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui->passesView->setModel(tableModel);
        ui->passesView->resizeColumnsToContents();
    } else {
        tableModel->clear();
        enableSatelliteButtons(false);
    }
}

void MainWindow::persistenceChangedSlot(const QString text) {
    settings.setPersistentString(text);
}

void MainWindow::addTrackerDialogSlot() {
    ui->satellitesView->selectionModel()->clear();
    addTrackerDialog.tleInput->clear();
    addTrackerDialog.exec();
}

void MainWindow::editSelectedTrackerSlot() {
    auto index = ui->satellitesView->selectionModel()->selectedIndexes().first().row();
    auto tracker = model->getTrackers()[index];
    //settings.saveTrackers(model->getTrackers());
    addTrackerDialog.tleInput->setPlainText(tracker.getFullTLE());
    addTrackerDialog.exec();
}

void MainWindow::removeSelectedTrackerSlot() {
    foreach(const QModelIndex &index, ui->satellitesView->selectionModel()->selectedIndexes()) {
        qDebug() << index.data(Qt::DisplayRole).toString();
        model->removeRow(index.row());
        settings.saveTrackers(model->getTrackers());
    }
}

void MainWindow::acceptedTleSlot(int confirm) {
    if(confirm) {
        //qDebug() << addTrackerDialog.tleInput->toPlainText();
        QStringList tle = addTrackerDialog.tleInput->toPlainText().split("\n");
        if(tle.size() == 3) { // Check if two line element set has three elements :)
            try {
                Tracker t(tle);
                auto selected = ui->satellitesView->selectionModel()->selection();
                if(selected.isEmpty()) {
                    auto index = model->addTracker(t);
                    ui->satellitesView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
                    settings.saveTrackers(model->getTrackers());
                } else {
                    auto selectedIndex = selected.indexes().first();
                    auto tracker = model->getTrackers()[selectedIndex.row()];
                    ui->satellitesView->selectionModel()->setCurrentIndex(selectedIndex, QItemSelectionModel::ClearAndSelect);
                    model->setTracker(selectedIndex.row(), t);
                    settings.saveTrackers(model->getTrackers());
                    rowChangedSlot(selected, QItemSelection());
                }
            } catch(TleException e) {
                qDebug() << "TLE inválida";
            }

            //model->setData();
        } else {
            qDebug() << "Wrong";
        }
    }
}

void MainWindow::satInfoUpdateSlot() {
    auto selected = ui->satellitesView->selectionModel()->selection();
    if(!selected.isEmpty()) {
        auto selectedIndex = selected.indexes().first();
        auto tracker = model->getTrackers()[selectedIndex.row()];

        ui->satElevation->setText(tracker.getSatInfo(Tracker::Elevation) + QString("°"));
        ui->satAzimuth->setText(tracker.getSatInfo(Tracker::Azimuth) + QString("°"));
        ui->satNextPass->setText(tracker.nextPass());
    } else {
        satInfoTimer.stop();
        ui->satelliteGroupBox->setTitle("Satélite");
        ui->satElevation->setText("");
        ui->satAzimuth->setText("");
        ui->satNextPass->setText("");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
