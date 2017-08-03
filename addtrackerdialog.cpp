#include "addtrackerdialog.h"

// http://stackoverflow.com/questions/12470806/qdialog-exec-and-getting-result-value
AddTrackerDialog::AddTrackerDialog(QWidget * parent)
    : QDialog(parent)
{
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
