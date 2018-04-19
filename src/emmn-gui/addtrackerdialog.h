#ifndef ADDTRACKERDIALOG_H
#define ADDTRACKERDIALOG_H
#include <QDialog>
#include <QCompleter>
#include <QStringListModel>
#include "ui_trackerdialog.h"
#include "trackerlistmodel.h"
#include "tracker.h"

//!  Caixa de diálogo para inserção de um satélite
class AddTrackerDialog : public QDialog, public Ui::AddTrackerDialog {
    Q_OBJECT

public:
    AddTrackerDialog(QWidget* parent = 0);

     /*! \brief Construtor para acrescentar um novo satélite. Connecta signals e slots e configura apresentação
      *
      * @param model Ponteiro para lista que armazena satélites cadastrados
      */
    AddTrackerDialog(TrackerListModel* satelliteCatalogue,
                     TrackerListModel* trackedSatellites,
                     QWidget* parent = 0);
private:
    TrackerListModel* satelliteCatalogue; //!< A lista de satélites do sistema do programa principal
    TrackerListModel* trackedSatellites; //!< A lista de satélites do sistema do programa principal
    QCompleter* completer;
    QStringListModel* s;
public slots:
     //! Slot chamado ao clicar em "Ok"
     void accept();

};

#endif // ADDTRACKERDIALOG_H
