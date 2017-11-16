#ifndef ADDTRACKERDIALOG_H
#define ADDTRACKERDIALOG_H
#include <QDialog>
#include <QCompleter>
#include "ui_tracker_dialog.h"
#include "trackerlistmodel.h"
#include "tracker.h"

//!  Caixa de diálogo para inserção de um satélite
class AddTrackerDialog : public QDialog, public Ui::AddTrackerDialog {
    Q_OBJECT

public:
     //! \brief Construtor base. Connecta signals e slots e configura apresentação
    AddTrackerDialog(QWidget* parent = 0);

     /*! \brief Construtor para acrescentar um novo satélite
      *
      * @param model Ponteiro para lista que armazena satélites cadastrados
      */
    AddTrackerDialog(TrackerListModel* model, QWidget* parent = 0);
private:
    TrackerListModel* model; //!< A lista de satélites do sistema do programa principal
    QCompleter* completer;
    QStringListModel* s;
public slots:
     //! Slot chamado ao clicar em "Ok"
     void accept();

};

#endif // ADDTRACKERDIALOG_H
