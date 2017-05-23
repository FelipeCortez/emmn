#include "addtrackerdialog.h"

AddTrackerDialog::AddTrackerDialog(QWidget * parent) : QDialog(parent) {
    // http://stackoverflow.com/questions/12470806/qdialog-exec-and-getting-result-value
    setupUi(this);
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
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
