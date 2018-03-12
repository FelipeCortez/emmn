#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QLocale>
#include <string>
#include "helpers.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , satInfoTimer(this)
  , updateTLETimer(this)
  , tableModel(nullptr)
  , control(nullptr)
  , network(this)
{
    trackedSatellites = new TrackerListModel();
    satelliteCatalogue = Helpers::readTLEList();

    ui->setupUi(this);
    ui->satellitesView->setModel(trackedSatellites);
    ui->passesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->passesView->verticalHeader()->setDefaultSectionSize(ui->passesView->verticalHeader()->fontMetrics().height() + 6);
    ui->passesView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->passesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->satellitesView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->satellitesView->setDragEnabled(true);

    QDateTime now = QDateTime::currentDateTime();
    QDateTime lastUpdate = Settings::getLastUpdatedDate();
    prevTime = now;

    // updateTrackersListSlot demora um pouco / rodar paralelamente!
    if (lastUpdate < now.addDays(-1)) {
        ui->statusBar->showMessage("Atualizando");
        network.updateSatelliteCatalogue();
    } else {
        updateTrackersListSlot();
    }

    setPortFromSettings();

    satInfoTimer.start(100);
    updateTLETimer.start(60000);

    connect(ui->actionConfig,
            SIGNAL(triggered(bool)),
            this,
            SLOT(settingsDialogSlot(bool)));
    connect(ui->actionManualControl,
            SIGNAL(triggered(bool)),
            this,
            SLOT(manualControlDialogSlot(bool)));
    connect(ui->actionSendPower,
            SIGNAL(triggered(bool)),
            this,
            SLOT(sendPowerSlot(bool)));
    connect(ui->actionDebugar,
            SIGNAL(triggered(bool)),
            this,
            SLOT(debugSlot(bool)));
    connect(ui->actionUpdateTLEs,
            SIGNAL(triggered(bool)),
            this,
            SLOT(updateTLESlot(bool)));
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
    connect(&updateTLETimer,
            SIGNAL(timeout()),
            this,
            SLOT(updateTLECheckSlot()));
    connect(ui->trackSatellitesCheckbox,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(trackSatellitesCheckboxChanged(int)));
    connect(&network,
            SIGNAL(updateTrackersUI()),
            this,
            SLOT(updateTrackersListSlot()));
}

void MainWindow::setPortFromSettings() {
    if (!control) {
        control = new Control(Settings::getSerialPort(), trackedSatellites);
    } else {
        control->changePort(Settings::getSerialPort());
    }

    if (!control->isPortValid()) {
        ui->actionManualControl->setEnabled(false);
    } else {
        ui->actionManualControl->setEnabled(true);
    }
}

void MainWindow::loadTrackersFromSettings() {
    auto trackers = Settings::loadTrackers();

    trackedSatellites->getTrackersRef().clear();

    for (auto& tSaved : trackers) {
        for (auto& tCatalog : satelliteCatalogue->getTrackersRef()) {
            if (tCatalog.getSatCatNumber() == tSaved.getSatCatNumber()) {
                tSaved.setTle(tCatalog.getTle());
                trackedSatellites->addTracker(tSaved);
            }
        }
    }

    Settings::saveTrackers(trackedSatellites->getTrackersRef());
}

void MainWindow::enableSatelliteButtons(bool enable) {
    ui->removeTrackerButton->setEnabled(enable);
    ui->showAllPassesButton->setEnabled(enable);
    auto selected = ui->satellitesView->selectionModel()->selection();
    if (enable) {
        auto selectedIndex = selected.indexes().first();
        ui->moveTrackerUpButton->setEnabled(selectedIndex.row() > 0);
        ui->moveTrackerDownButton->setEnabled(selectedIndex.row() < trackedSatellites->rowCount() - 1);
    } else {
        ui->moveTrackerUpButton->setEnabled(false);
        ui->moveTrackerDownButton->setEnabled(false);
    }
}

void MainWindow::rowChangedSlot(QItemSelection selected, QItemSelection) {
    if (!selected.isEmpty()) {
        auto selectedIndex = selected.indexes().first();
        auto tracker = trackedSatellites->getTrackers()[selectedIndex.row()];
        enableSatelliteButtons();
        satInfoUpdateSlot();
        ui->passesViewLabel->setText("Passagens para " + tracker.getCommonName());
        ui->satelliteGroupBox->setTitle(tracker.getCommonName());
        ui->nextPassesView->repaint();

        QList<PassDetails> pd = tracker.GeneratePassList();
        // http://stackoverflow.com/a/11907059
        const int numRows = pd.size();
        const int numColumns = 1;

        if (tableModel) { delete tableModel; }
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

        QList<PassDetailsWithTracker> allPasses = trackedSatellites->getAllPasses();
        const int numRows = allPasses.size();
        const int numColumns = 1;

        QStringList headers;
        headers << "Satélite"
                << "Aquisição de sinal"
                << "Perda de sinal"
                << "El. máx."
                << "Duração"
                << "Tipo";

        if (tableModel) { delete tableModel; }

        if (trackedSatellites->rowCount() > 0) {
            tableModel = new QStandardItemModel(numRows, numColumns);
            tableModel->setHorizontalHeaderLabels(headers);

            if (allPasses.begin() == allPasses.end()) {
                qDebug() << "no passes found";
            } else {
                int row = 0;
                QList<PassDetailsWithTracker>::const_iterator itr = allPasses.begin();
                do {
                    QString text;
                    QStandardItem* item;

                    text = itr->tracker->getCommonName();
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
        } else {
            tableModel = new QStandardItemModel();
        }

        ui->passesView->setModel(tableModel);
        ui->passesView->resizeColumnsToContents();
    }
}

void MainWindow::addTrackerDialogSlot() {
    AddTrackerDialog dialog(satelliteCatalogue, trackedSatellites, this);
    if (dialog.exec()) {
        auto selectionModel = ui->satellitesView->selectionModel();
        auto idx = trackedSatellites->index(trackedSatellites->rowCount() - 1);
        selectionModel->clear();
        selectionModel->setCurrentIndex(idx,
                                        QItemSelectionModel::ClearAndSelect);
    }
}

void MainWindow::settingsDialogSlot(bool) {
    SettingsDialog dialog;
    if (dialog.exec()) {
        auto selected = ui->satellitesView->selectionModel()->selection();
        rowChangedSlot(selected, QItemSelection());
        setPortFromSettings();
    }
}

void MainWindow::manualControlDialogSlot(bool) {
    satInfoTimer.stop();
    ui->azLabel->setText("--");
    ui->eleLabel->setText("--");
    control->setControlMode(ControlMode::Manual);
    ManualControlDialog dialog(control, this);
    // exec bloqueia, então o timer só será iniciado ao fechar a caixa de diálogo
    dialog.exec();
    control->setControlMode(ControlMode::None);
    ui->trackSatellitesCheckbox->setChecked(false);
    satInfoTimer.start(100);
}

void MainWindow::removeSelectedTrackerSlot() {
    foreach(const QModelIndex &index, ui->satellitesView->selectionModel()->selectedIndexes()) {
        trackedSatellites->removeRow(index.row());
        ui->satellitesView->selectionModel()->clear();
        Settings::saveTrackers(trackedSatellites->getTrackers());
    }

    ui->nextPassesView->repaint();

    auto selected = ui->satellitesView->selectionModel()->selection();
    rowChangedSlot(selected, QItemSelection());
}

void MainWindow::satInfoUpdateSlot() {
    const auto selected = ui->satellitesView->selectionModel()->selection();

    if (!selected.isEmpty()) {
        auto selectedIndex = selected.indexes().first();
        auto& tracker = trackedSatellites->getTrackersRef()[selectedIndex.row()];

        ui->satEle->setText(tracker.getSatInfo(Tracker::Elevation) + QString("°"));
        ui->satAz->setText(tracker.getSatInfo(Tracker::Azimuth) + QString("°"));
        ui->satNextPass->setText(tracker.nextPass());
    } else {
        ui->satelliteGroupBox->setTitle("Satélite");
        ui->satEle->setText("");
        ui->satAz->setText("");
        ui->satNextPass->setText("");
    }

    if (trackedSatellites->rowCount() > 0) {
        const auto nextPass = trackedSatellites->getAllPasses().at(0);
        const auto remaining = nextPass.passDetails.aos - DateTime::Now();

        if (prevTime.time().minute() != QDateTime::currentDateTime().time().minute()) {
            ui->nextPassesView->repaint();
            prevTime = QDateTime::currentDateTime();
        }

        if (nextPass.tracker->getElevationForObserver() >= 0) {
            ui->nextPassCountdownLabel->setText("Passando");
        } else {
            ui->nextPassCountdownLabel->setText(QString::fromStdString(remaining.ToString()));
        }

        ui->nextPassSatLabel->setText(trackedSatellites->getAllPasses().at(0).tracker->getCommonName());
    } else {
        ui->nextPassCountdownLabel->setText("");
        ui->nextPassSatLabel->setText("");
    }

    if (control->isPortValid()) {
        AzEle antennaInfo = control->getState();
        ui->azLabel->setText(QString::number(antennaInfo.azimuth));
        ui->eleLabel->setText(QString::number(antennaInfo.elevation));
        ui->powerLabel->setText(control->getPowerStatus() ? "Ligado" : "Desligado");
    } else {
        //! \todo Estas duas variáveis estão espalhadas por vários lugares do código. Deixar num canto só
        ui->azLabel->setText("");
        ui->eleLabel->setText("");
        ui->powerLabel->setText("Porta inválida");
    }
}

void MainWindow::clearSelectedTrackerSlot() {
    ui->satellitesView->selectionModel()->clear();
}

void MainWindow::moveTrackerUpSlot() {
    auto index = ui->satellitesView->selectionModel()->selectedIndexes().first();
    auto indexRow = index.row();
    trackedSatellites->getTrackersPointer()->swap(indexRow, indexRow - 1);
    auto newIndex = trackedSatellites->index(indexRow - 1);
    ui->satellitesView->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
    ui->satellitesView->setModel(trackedSatellites);
    Settings::saveTrackers(trackedSatellites->getTrackers());
}

void MainWindow::moveTrackerDownSlot() {
    auto index = ui->satellitesView->selectionModel()->selectedIndexes().first();
    auto indexRow = index.row();
    trackedSatellites->getTrackersPointer()->swap(indexRow, indexRow + 1);
    auto newIndex = trackedSatellites->index(indexRow + 1);
    ui->satellitesView->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
    ui->satellitesView->setModel(trackedSatellites);
    Settings::saveTrackers(trackedSatellites->getTrackers());
}

void MainWindow::trackSatellitesCheckboxChanged(int state) {
    ControlMode mode = (state == 0) ? ControlMode::None : ControlMode::Schedule;
    control->setControlMode(mode);
}

void MainWindow::debugSlot(bool) {
    //
}

void MainWindow::sendPowerSlot(bool) {
    control->sendPower();
}

void MainWindow::updateTLESlot(bool) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmação",
                                  "As TLEs são atualizadas diariamente e esta ação só é recomendada em casos de emergência",
                                  QMessageBox::Yes | QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) {
        ui->statusBar->showMessage("Atualizando");
        network.updateSatelliteCatalogue();
    } else {
        qDebug() << "Yes was *not* clicked";
    }
}

void MainWindow::updateTrackersListSlot() {
    loadTrackersFromSettings();
    ui->nextPassesView->setTrackers(trackedSatellites->getTrackersPointer());

    // Força atualização da tabela de passagens (meio gambiarra... talvez mudar)
    auto selected = ui->satellitesView->selectionModel()->selection();
    rowChangedSlot(selected, QItemSelection());

    QDateTime lastUpdate = Settings::getLastUpdatedDate();
    //QLabel* lastUpdateLabel = new QLabel("Última atualização: " + lastUpdate.toString());
    //ui->statusBar->addWidget(lastUpdateLabel);
    ui->statusBar->showMessage("Última atualização: " + lastUpdate.toString());
}

void MainWindow::updateTLECheckSlot() {
    QDateTime now = QDateTime::currentDateTime();
    QDateTime lastUpdate = Settings::getLastUpdatedDate();
    prevTime = now;

    if (lastUpdate < now.addDays(-1)) {
        ui->statusBar->showMessage("Atualizando");
        network.updateSatelliteCatalogue();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete trackedSatellites;
    delete satelliteCatalogue;
    delete tableModel;
    delete control;
    /*
    delete logger;
    */
}
