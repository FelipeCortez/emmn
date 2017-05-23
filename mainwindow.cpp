#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardItemModel>
#include <QDebug>
#include <QNetworkAccessManager>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , addTrackerDialog(this)
  , settings()
{
    model = new TrackerListModel();

    ui->setupUi(this);
    ui->persistenceInput->setText(settings.getPersistentString());
    ui->satellitesView->setModel(model);
    ui->satellitesView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->satellitesView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this,
            SLOT(rowChangedSlot(QItemSelection, QItemSelection)));
    connect(ui->persistenceInput,
            SIGNAL(textChanged(const QString)),
            this,
            SLOT(persistenceChangedSlot(const QString)));
    connect(ui->addTrackerButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(addTrackerDialogSlot()));
    connect(&addTrackerDialog,
            SIGNAL(finished(int)),
            this,
            SLOT(acceptedTleSlot(int)));
}

void MainWindow::rowChangedSlot(QItemSelection selected, QItemSelection) {
    qDebug() << selected.indexes().first().data(0).toString();
    qDebug() << selected.indexes().first().data(TrackerListModel::IdRole).toString();
    QList<PassDetails> pd = selected.indexes().first().data(TrackerListModel::PassesRole).value<QList<PassDetails>>();
    //std::list<PassDetails> pd = trackers[0].GeneratePassList();

    // http://stackoverflow.com/a/11907059
    const int numRows = pd.size();
    const int numColumns = 1;

    QStandardItemModel* tableModel = new QStandardItemModel(numRows, numColumns);
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

            text = QString::fromStdString(itr->aos.ToString());
            item = new QStandardItem(text);
            tableModel->setItem(row, 0, item);

            text = QString::fromStdString(itr->los.ToString());
            item = new QStandardItem(text);
            tableModel->setItem(row, 1, item);

            text = QString::number(Util::RadiansToDegrees(itr->max_elevation));
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
}

void MainWindow::persistenceChangedSlot(const QString text) {
    settings.setPersistentString(text);
}

void MainWindow::addTrackerDialogSlot() {
    addTrackerDialog.exec();
}

void MainWindow::acceptedTleSlot(int) {
    qDebug() << addTrackerDialog.tleInput->toPlainText();
    QStringList tle = addTrackerDialog.tleInput->toPlainText().split("\n");
    if(tle.size() == 3) { // Check if two line element set has three elements :)
        try {
            Tracker t(tle);
            model->addTracker(t);
        } catch(TleException e) {
            qDebug() << "TLE inválida";
        }

        //model->setData();
    } else {
        qDebug() << "Wrong";
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
