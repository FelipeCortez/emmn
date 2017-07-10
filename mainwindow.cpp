#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <string>
#include "helpers.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , addTrackerDialog(this)
  , settingsDialog(this)
  , satInfoTimer(this)
  , antennaTimer(this)
  , tableModel(nullptr)
  , control(L"COM3")
{
    model = new TrackerListModel();

    counter = 0;
    ui->setupUi(this);
    ui->satellitesView->setModel(model);
    setWindowState(Qt::WindowMaximized);
    //ui->satellitesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->passesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->passesView->verticalHeader()->setDefaultSectionSize(ui->passesView->verticalHeader()->fontMetrics().height()+6);
    ui->passesView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->passesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->satellitesView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->satellitesView->setDragEnabled(true);

    satInfoTimer.start(1000);
    antennaTimer.start(100);

    addTrackerDialog.tleInput->setWhatsThis("Two-line element set de um satélite. É possível adquiri-lo através de um site como CelesTrak (https://www.celestrak.com/)");

    loadTrackersFromSettings();
    ui->nextPassesView->setTrackers(model->getTrackersRef());

    // Força atualização da tabela (meio gambiarra... talvez mudar)
    auto selected = ui->satellitesView->selectionModel()->selection();
    rowChangedSlot(selected, QItemSelection());

    connect(ui->actionConfig,
            SIGNAL(triggered(bool)),
            this,
            SLOT(settingsDialogSlot(bool)));
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
    connect(&addTrackerDialog,
            SIGNAL(finished(int)),
            this,
            SLOT(acceptedTleSlot(int)));
    connect(&settingsDialog,
            SIGNAL(finished(int)),
            this,
            SLOT(acceptedSettingsSlot(int)));
    connect(&satInfoTimer,
            SIGNAL(timeout()),
            this,
            SLOT(satInfoUpdateSlot()));
    connect(&antennaTimer,
            SIGNAL(timeout()),
            this,
            SLOT(antennaUpdateSlot()));

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

        //network.getTLE(tracker.getTitle());
        //tracker.UpdateTLE();
        QList<PassDetails> pd = tracker.GeneratePassListQt();
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
                                                            << "Duração");

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

                ++row;
            } while (++itr != allPasses.end());
        }

        ui->passesView->setModel(tableModel);
        ui->passesView->resizeColumnsToContents();
    }
}

void MainWindow::addTrackerDialogSlot() {
    ui->satellitesView->selectionModel()->clear();
    addTrackerDialog.tleInput->clear();
    addTrackerDialog.exec();
}

void MainWindow::settingsDialogSlot(bool) {
    settingsDialog.updateWithSettings();
    settingsDialog.exec();
}

void MainWindow::editSelectedTrackerSlot() {
    auto index = ui->satellitesView->selectionModel()->selectedIndexes().first().row();
    auto tracker = model->getTrackers()[index];
    addTrackerDialog.tleInput->setPlainText(tracker.getFullTLE());
    addTrackerDialog.exec();
}

void MainWindow::removeSelectedTrackerSlot() {
    foreach(const QModelIndex &index, ui->satellitesView->selectionModel()->selectedIndexes()) {
        qDebug() << index.data(Qt::DisplayRole).toString();
        model->removeRow(index.row());
        Settings::saveTrackers(model->getTrackers());
    }

    ui->nextPassesView->repaint();
}

void MainWindow::acceptedTleSlot(int confirm) {
    if(confirm) {
        //qDebug() << addTrackerDialog.tleInput->toPlainText();
        QStringList tle = addTrackerDialog.tleInput->toPlainText().split("\n");
        if(tle.size() == 3) { // Check if two line element set has three lines :)
            try {
                Tracker t(tle);
                auto selected = ui->satellitesView->selectionModel()->selection();
                if(selected.isEmpty()) {
                    auto index = model->addTracker(t);
                    ui->satellitesView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
                    Settings::saveTrackers(model->getTrackers());
                } else {
                    auto selectedIndex = selected.indexes().first();
                    auto tracker = model->getTrackers()[selectedIndex.row()];
                    ui->satellitesView->selectionModel()->setCurrentIndex(selectedIndex, QItemSelectionModel::ClearAndSelect);
                    model->setTracker(selectedIndex.row(), t);
                    Settings::saveTrackers(model->getTrackers());
                    rowChangedSlot(selected, QItemSelection());
                }
            } catch(TleException e) {
                qDebug() << "TLE inválida";
            }
        } else {
            qDebug() << "Wrong";
        }
    }
}

void MainWindow::acceptedSettingsSlot(int confirm) {
    if(confirm) {
        Settings::setUseLocalTime(settingsDialog.useLocalTimeCheckbox->isChecked());
        auto selected = ui->satellitesView->selectionModel()->selection();
        rowChangedSlot(selected, QItemSelection());
    }
}

void MainWindow::satInfoUpdateSlot() {
    auto selected = ui->satellitesView->selectionModel()->selection();
    ui->nextPassesView->repaint();

    auto remaining = model->allPasses.at(0).passDetails.aos - DateTime::Now();
    if(remaining.Ticks() <= 0) {
        ui->nextPassCountdownLabel->setText("Passando");
    } else {
        ui->nextPassCountdownLabel->setText(QString::fromStdString(remaining.ToString()));
    }
    ui->nextPassSatLabel->setText(model->allPasses.at(0).tracker->getTitle());
    //QMap<QString, float> answerMap = control.send_state();
    //ui->azLabel->setText(QString::number(answerMap.value("az")));
    //ui->eleLabel->setText(QString::number(answerMap.value("ele")));

    if(!selected.isEmpty()) {
        auto selectedIndex = selected.indexes().first();
        auto tracker = model->getTrackers()[selectedIndex.row()];

        ui->satElevation->setText(tracker.getSatInfo(Tracker::Elevation) + QString("°"));
        ui->satAzimuth->setText(tracker.getSatInfo(Tracker::Azimuth) + QString("°"));
        ui->satNextPass->setText(tracker.nextPass());
    } else {
        //satInfoTimer.stop();
        ui->satelliteGroupBox->setTitle("Satélite");
        ui->satElevation->setText("");
        ui->satAzimuth->setText("");
        ui->satNextPass->setText("");
    }
}

double lerp(float t, float a, float b){
      return ((1-t) * a) + (t * b);
  }

void MainWindow::antennaUpdateSlot() {
    TimeSpan remaining = model->allPasses.at(0).passDetails.aos - DateTime::Now(true);
    auto nextPass = model->allPasses.at(0);
    float secondsRemaining = remaining.Ticks() / (1000000.f);
    //qDebug() << remaining.TotalSeconds();

    float startLerping = 60;

    if(nextPass.tracker->getObserverElevation() >= 0) {
        qDebug() << "Passando";
        qDebug() << nextPass.tracker->getObserverElevation();
    } else if(secondsRemaining <= startLerping &&
              secondsRemaining > 0) {
        qDebug() << "Interpolando";
        // Interpolar entre elevação atual e 0
        double t = (startLerping - secondsRemaining) / startLerping;
        qDebug() << "Antena:";
        qDebug() << lerp(t, 180, 0);
    } else {
        qDebug() << "Contagem regressiva";
        qDebug() << secondsRemaining - startLerping;
    }
    qDebug() << "--";
}

void MainWindow::clearSelectedTrackerSlot() {
    ui->satellitesView->selectionModel()->clear();
}

void MainWindow::moveTrackerUpSlot() {
    auto index = ui->satellitesView->selectionModel()->selectedIndexes().first();
    auto indexRow = index.row();
    model->getTrackersRef()->swap(indexRow, indexRow - 1);
    auto newIndex = model->index(indexRow - 1);
    ui->satellitesView->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
    ui->satellitesView->setModel(model);
    Settings::saveTrackers(model->getTrackers());
}

void MainWindow::moveTrackerDownSlot() {
    auto index = ui->satellitesView->selectionModel()->selectedIndexes().first();
    auto indexRow = index.row();
    model->getTrackersRef()->swap(indexRow, indexRow + 1);
    auto newIndex = model->index(indexRow + 1);
    ui->satellitesView->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
    ui->satellitesView->setModel(model);
    Settings::saveTrackers(model->getTrackers());
}

MainWindow::~MainWindow()
{
    delete ui;
}
