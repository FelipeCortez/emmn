#include "addtrackerdialog.h"
#include "helpers.h"
#include <QDebug>

AddTrackerDialog::AddTrackerDialog(TrackerListModel* satelliteCatalogue,
                                   TrackerListModel* trackedSatellites,
                                   QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    completer = new QCompleter(this);
    completer->setModel(Helpers::QStringListModelFromSatelliteCatalog(satelliteCatalogue));
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    titleInput->setCompleter(completer);

    connect(addTLEButtonBox,
            SIGNAL(accepted()),
            this,
            SLOT(accept()));
    connect(addTLEButtonBox,
            SIGNAL(rejected()),
            this,
            SLOT(reject()));

    this->satelliteCatalogue = satelliteCatalogue;
    this->trackedSatellites = trackedSatellites;
}

void AddTrackerDialog::accept() {
    QString tleTitle = titleInput->text();
    QRegularExpression re2("\\[(\\w+)\\]");
    QString satCatNumber = re2.match(tleTitle).captured(1);
    qDebug() << satCatNumber;
    qDebug() << trackedSatellites->rowCount();
    qDebug() << satelliteCatalogue->rowCount();
    //auto satellite = satelliteCatalogue->findTracker(satCatNumber);
    /*
    if(satellite) {
        Settings::saveTrackers(model->getTrackers());
    }
    */

    QDialog::accept();
}
