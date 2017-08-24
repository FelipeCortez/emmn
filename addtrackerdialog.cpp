#include "addtrackerdialog.h"
#include "helpers.h"
#include <QDebug>

AddTrackerDialog::AddTrackerDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    tleInput->setWhatsThis("Two-line element set de um satélite. É possível adquiri-lo através de um site como CelesTrak (https://www.celestrak.com/)");
    tleInput->setFont(fixedFont);
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
    mode = TrackerDialogMode::New;
}

AddTrackerDialog::AddTrackerDialog(TrackerListModel* model, Tracker* tracker, QWidget* parent)
    : AddTrackerDialog(parent)
{
    this->model = model;
    this->tracker = tracker;
    mode = TrackerDialogMode::Edit;
    tleInput->setPlainText(tracker->getFullTLE());
}

void AddTrackerDialog::accept() {
    qDebug() << tleInput->toPlainText();
    QStringList tle = tleInput->toPlainText().split("\n");
    if(tle.size() == 3) {
        try {
            // TODO: TLE não está sendo validada!
            Tracker* t = new Tracker(tle);
            if(mode == TrackerDialogMode::New) {
                qDebug() << "New";
                model->addTracker(*t);
                Settings::saveTrackers(model->getTrackers());
            } else if(mode == TrackerDialogMode::Edit) {
                qDebug() << "Edit";
                tracker->setTle(tle);
                Settings::saveTrackers(model->getTrackers());
            }
        } catch(TleException) {
            QDialog::reject();
            return;
        }
    } else {
        QDialog::reject();
        return;
    }

    QDialog::accept(); // TODO: rodar só se for sucesso!
}
