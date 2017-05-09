#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringListModel>
#include <QStandardItemModel>
#include <QDebug>
#include <QNetworkAccessManager>
#include <vector>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    std::vector<std::string> tle;
    tle.push_back("SCD 1                   ");
    tle.push_back("1 22490U 93009B   17123.70008125  .00000249  00000-0  18216-4 0  9992");
    tle.push_back("2 22490  24.9694 217.7757 0042800 225.1509 269.3688 14.44484423278880");
    Tracker tracker(tle);

    /*
    std::list<struct PassDetails> pd = tracker.GeneratePassList();

    if (pd.begin() == pd.end()) {
        std::cout << "No passes found" << std::endl;
    } else {
        std::stringstream ss;

        ss << std::right << std::setprecision(1) << std::fixed;

        std::list<struct PassDetails>::const_iterator itr = pd.begin();
        do {
            ss  << "AOS: " << itr->aos
                << ", LOS: " << itr->los
                << ", Max El: " << std::setw(4) << Util::RadiansToDegrees(itr->max_elevation)
                << ", Duration: " << (itr->los - itr->aos)
                << std::endl;
        } while (++itr != pd.end());

        std::cout << ss.str();
    }
    */

    QStringListModel *model = new QStringListModel();
    QStringList list;
    list << "SCD 1" << "SCD 2" << "CBERS-4";
    model->setStringList(list);
    ui->satellitesView->setModel(model);
    ui->satellitesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QItemSelectionModel *selectionModel = ui->satellitesView->selectionModel();
    connect(selectionModel,
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this,
            SLOT(rowChangedSlot(QItemSelection, QItemSelection)));
}

void MainWindow::rowChangedSlot(QItemSelection selected, QItemSelection) {
    qDebug() << selected.indexes()[0].data(0);
    std::list<struct PassDetails> pd = tracker.GeneratePassList();

    // http://stackoverflow.com/a/11907059
    const int numRows = pd.size();
    const int numColumns = 1;

    QStandardItemModel* model = new QStandardItemModel(numRows, numColumns);
    model->setHorizontalHeaderLabels(QStringList() << "AOS"
                                                   << "LOS"
                                                   << "Max elevation"
                                                   << "Duration");


    if (pd.begin() == pd.end()) {
        qDebug() << "no passes found";
    } else {
        int row = 0;
        std::list<struct PassDetails>::const_iterator itr = pd.begin();
        do {
            QString text;
            QStandardItem* item;

            text = QString::fromStdString(itr->aos.ToString());
            item = new QStandardItem(text);
            model->setItem(row, 0, item);

            text = QString::fromStdString(itr->los.ToString());
            item = new QStandardItem(text);
            model->setItem(row, 1, item);

            text = QString::number(Util::RadiansToDegrees(itr->max_elevation));
            item = new QStandardItem(text);
            model->setItem(row, 2, item);

            text = QString::fromStdString((itr->los - itr->aos).ToString());
            item = new QStandardItem(text);
            model->setItem(row, 3, item);

            ++row;
        } while (++itr != pd.end());
    }

    ui->passesView->setModel(model);
    ui->passesView->resizeColumnsToContents();
}

MainWindow::~MainWindow()
{
    delete ui;
}
