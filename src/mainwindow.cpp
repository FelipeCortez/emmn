#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <string>
#include "helpers.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , satInfoTimer(this)
  , tableModel(nullptr)
  , control(nullptr)
{
    model = new TrackerListModel();

    ui->setupUi(this);
    ui->satellitesView->setModel(model);
    //setWindowState(Qt::WindowMaximized);
    //ui->satellitesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->passesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->passesView->verticalHeader()->setDefaultSectionSize(ui->passesView->verticalHeader()->fontMetrics().height()+6);
    ui->passesView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->passesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->satellitesView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->satellitesView->setDragEnabled(true);

    setPortFromSettings();

    satInfoTimer.start(1000);

    loadTrackersFromSettings();
    ui->nextPassesView->setTrackers(model->getTrackersPointer());

    // Força atualização da tabela (meio gambiarra... talvez mudar)
    auto selected = ui->satellitesView->selectionModel()->selection();
    rowChangedSlot(selected, QItemSelection());

    connect(ui->actionConfig,
            SIGNAL(triggered(bool)),
            this,
            SLOT(settingsDialogSlot(bool)));
    connect(ui->actionManualControl,
            SIGNAL(triggered(bool)),
            this,
            SLOT(manualControlDialogSlot(bool)));
    connect(ui->actionDebugar,
            SIGNAL(triggered(bool)),
            this,
            SLOT(debugarSlot(bool)));
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
    connect(ui->showAllPassesButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(clearSelectedTrackerSlot()));
    connect(ui->moveTrackerUpButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(moveTrackerUpSlot()));
    connect(ui->moveTrackerDownButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(moveTrackerDownSlot()));
    connect(&satInfoTimer,
            SIGNAL(timeout()),
            this,
            SLOT(satInfoUpdateSlot()));
}

void MainWindow::setPortFromSettings() {
    if(!control) {
        control = new Control(Settings::getSerialPort(), model);
    } else {
        control->changePort(Settings::getSerialPort());
    }

    if(!control->isPortValid()) {
        ui->actionManualControl->setEnabled(false);
        ui->azLabel->setText("---");
        ui->eleLabel->setText("---");
        ui->powerLabel->setText("Porta inválida");
    } else {
        ui->actionManualControl->setEnabled(true);
        ui->powerLabel->setText("Ligado");
    }
}

void MainWindow::loadTrackersFromSettings() {
    auto trackers = Settings::loadTrackers();
    for(auto t : trackers) {
        model->addTracker(t);
    }
}

void MainWindow::enableSatelliteButtons(bool enable) {
    ui->removeTrackerButton->setEnabled(enable);
    ui->editTrackerButton->setEnabled(enable);
    ui->showAllPassesButton->setEnabled(enable);
    auto selected = ui->satellitesView->selectionModel()->selection();
    if(enable) {
        auto selectedIndex = selected.indexes().first();
        ui->moveTrackerUpButton->setEnabled(selectedIndex.row() > 0);
        ui->moveTrackerDownButton->setEnabled(selectedIndex.row() < model->rowCount() - 1);
    } else {
        ui->moveTrackerUpButton->setEnabled(false);
        ui->moveTrackerDownButton->setEnabled(false);
    }
}

void MainWindow::rowChangedSlot(QItemSelection selected, QItemSelection) {
    if(!selected.isEmpty()) {
        auto selectedIndex = selected.indexes().first();
        auto tracker = model->getTrackers()[selectedIndex.row()];
        enableSatelliteButtons();
        satInfoUpdateSlot();
        ui->passesViewLabel->setText("Passagens para " + tracker.getTitle());
        ui->satelliteGroupBox->setTitle(tracker.getTitle());
        ui->nextPassesView->repaint();

        QList<PassDetails> pd = tracker.GeneratePassList();
        // http://stackoverflow.com/a/11907059
        const int numRows = pd.size();
        const int numColumns = 1;

        if(tableModel) { delete tableModel; }
        tableModel = new QStandardItemModel(numRows, numColumns);
        tableModel->setHorizontalHeaderLabels(QStringList() << "Aquisição de sinal"
                                                            << "Perda de sinal"
                                                            << "El. máx."
                                                            << "Duração");

        if (pd.begin() == pd.end()) {
            qDebug() << "no passes found";
        } else {
            int row = 0;
            QList<PassDetails>::const_iterator itr = pd.begin();
            do {
                //QCoreApplication::processEvents();
                //QEventLoop::processEvents();
                QString text;
                QStandardItem* item;

                text = Helpers::betterDate(itr->aos);
                item = new QStandardItem(text);
                tableModel->setItem(row, 0, item);

                text = Helpers::betterDate(itr->los);
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
        // Nenhum satélite selecionado!
        ui->passesViewLabel->setText("Todas as passagens");
        enableSatelliteButtons(false);

        QList<PassDetailsWithTracker> allPasses = model->getAllPasses();
        const int numRows = allPasses.size();
        const int numColumns = 1;
        if(tableModel) { delete tableModel; }
        tableModel = new QStandardItemModel(numRows, numColumns);
        tableModel->setHorizontalHeaderLabels(QStringList() << "Satélite"
                                                            << "Aquisição de sinal"
                                                            << "Perda de sinal"
                                                            << "El. máx."
                                                            << "Duração"
                                                            << "Inversa");

        if (allPasses.begin() == allPasses.end()) {
            qDebug() << "no passes found";
        } else {
            int row = 0;
            QList<PassDetailsWithTracker>::const_iterator itr = allPasses.begin();
            do {
                QString text;
                QStandardItem* item;

                text = itr->tracker->getTitle();
                item = new QStandardItem(text);
                tableModel->setItem(row, 0, item);

                text = Helpers::betterDate(itr->passDetails.aos);
                item = new QStandardItem(text);
                tableModel->setItem(row, 1, item);

                text = Helpers::betterDate(itr->passDetails.los);
                item = new QStandardItem(text);
                tableModel->setItem(row, 2, item);

                text = QString::number(Util::RadiansToDegrees(itr->passDetails.max_elevation)) + QString("°");
                item = new QStandardItem(text);
                tableModel->setItem(row, 3, item);

                text = QString::fromStdString((itr->passDetails.los - itr->passDetails.aos).ToString());
                item = new QStandardItem(text);
                tableModel->setItem(row, 4, item);

                text = itr->passDetails.reverse ? QString("Inversa") : QString("Normal");
                item = new QStandardItem(text);
                tableModel->setItem(row, 5, item);

                ++row;
            } while (++itr != allPasses.end());
        }

        ui->passesView->setModel(tableModel);
        ui->passesView->resizeColumnsToContents();
    }
}

void MainWindow::addTrackerDialogSlot() {
    AddTrackerDialog dialog(model, this);
    if(dialog.exec()) {
        ui->satellitesView->selectionModel()->clear();
        ui->satellitesView->selectionModel()->setCurrentIndex(model->index(model->rowCount() - 1),
                                                              QItemSelectionModel::ClearAndSelect);
    }
}

void MainWindow::editSelectedTrackerSlot() {
    auto index = ui->satellitesView->selectionModel()->selectedIndexes().first().row();
    Tracker* tracker = &model->getTrackersRef()[index];
    AddTrackerDialog dialog(model, tracker, this);
    if(dialog.exec()) {
    }
}

void MainWindow::settingsDialogSlot(bool) {
    SettingsDialog dialog;
    if(dialog.exec()) {
        auto selected = ui->satellitesView->selectionModel()->selection();
        rowChangedSlot(selected, QItemSelection());
        setPortFromSettings();
    }
}

void MainWindow::manualControlDialogSlot(bool) {
    satInfoTimer.stop();
    control->setController(Controller::Manual);
    ManualControlDialog dialog(control, this);
    // exec bloqueia, de forma que o start do timer só rodará quando o dialog for fechado
    dialog.exec();
    control->setController(Controller::Schedule);
    satInfoTimer.start();
}

void MainWindow::removeSelectedTrackerSlot() {
    foreach(const QModelIndex &index, ui->satellitesView->selectionModel()->selectedIndexes()) {
        qDebug() << index.data(Qt::DisplayRole).toString();
        model->removeRow(index.row());
        ui->satellitesView->selectionModel()->clear();
        Settings::saveTrackers(model->getTrackers());
    }

    ui->nextPassesView->repaint();
}

void MainWindow::satInfoUpdateSlot() {
    auto selected = ui->satellitesView->selectionModel()->selection();
    auto nextPass = model->getAllPasses().at(0);
    auto remaining = nextPass.passDetails.aos - DateTime::Now();
    ui->nextPassesView->repaint();

    if(nextPass.tracker->getElevationForObserver() >= 0) {
        ui->nextPassCountdownLabel->setText("Passando");
    } else {
        ui->nextPassCountdownLabel->setText(QString::fromStdString(remaining.ToString()));
    }

    ui->nextPassSatLabel->setText(model->getAllPasses().at(0).tracker->getTitle());

    if(control->isPortValid()) {
        AzEle antennaInfo = control->send_state();
        ui->azLabel->setText(QString::number(antennaInfo.azimuth));
        ui->eleLabel->setText(QString::number(antennaInfo.elevation));
    }

    if(!selected.isEmpty()) {
        auto selectedIndex = selected.indexes().first();
        auto tracker = model->getTrackers()[selectedIndex.row()];

        ui->satEle->setText(tracker.getSatInfo(Tracker::Elevation) + QString("°"));
        ui->satAz->setText(tracker.getSatInfo(Tracker::Azimuth) + QString("°"));
        ui->satNextPass->setText(tracker.nextPass());
    } else {
        //satInfoTimer.stop();
        ui->satelliteGroupBox->setTitle("Satélite");
        ui->satEle->setText("");
        ui->satAz->setText("");
        ui->satNextPass->setText("");
    }

    model->getAllPasses();
}

void MainWindow::clearSelectedTrackerSlot() {
    ui->satellitesView->selectionModel()->clear();
}

void MainWindow::moveTrackerUpSlot() {
    auto index = ui->satellitesView->selectionModel()->selectedIndexes().first();
    auto indexRow = index.row();
    model->getTrackersPointer()->swap(indexRow, indexRow - 1);
    auto newIndex = model->index(indexRow - 1);
    ui->satellitesView->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
    ui->satellitesView->setModel(model);
    Settings::saveTrackers(model->getTrackers());
}

void MainWindow::moveTrackerDownSlot() {
    auto index = ui->satellitesView->selectionModel()->selectedIndexes().first();
    auto indexRow = index.row();
    model->getTrackersPointer()->swap(indexRow, indexRow + 1);
    auto newIndex = model->index(indexRow + 1);
    ui->satellitesView->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
    ui->satellitesView->setModel(model);
    Settings::saveTrackers(model->getTrackers());
}

void MainWindow::debugarSlot(bool) {
    Helpers::getSerialPortsAvailable();
}

MainWindow::~MainWindow()
{
    delete ui;
}
