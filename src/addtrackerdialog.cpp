#include "addtrackerdialog.h"
#include "helpers.h"
#include <QDebug>

AddTrackerDialog::AddTrackerDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    completer = new QCompleter(this);
    s = Helpers::readTLEList();
    completer->setModel(s);
    titleInput->setCompleter(completer);

    connect(addTLEButtonBox,
            SIGNAL(accepted()),
            this,
            SLOT(accept()));
    connect(addTLEButtonBox,
            SIGNAL(rejected()),
            this,
            SLOT(reject()));
}

// Delegating constructors - https://stackoverflow.com/a/308318
AddTrackerDialog::AddTrackerDialog(TrackerListModel* model, QWidget* parent)
    : AddTrackerDialog(parent)
{
    this->model = model;
}

void AddTrackerDialog::accept() {
    QString tleTitle = titleInput->text();
    qDebug() << tleTitle;
    tleTitle.remove(QRegExp("[AEIOU]."));
    qDebug() << tleTitle;
    //Helpers::findInTLEList();
    //Tracker* t = new Tracker(tle);
    //Settings::saveTrackers(model->getTrackers());

    QDialog::accept();
}
