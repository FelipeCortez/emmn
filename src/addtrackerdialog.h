#ifndef ADDTRACKERDIALOG_H
#define ADDTRACKERDIALOG_H
#include <QDialog>
#include "ui_tracker_dialog.h"
#include "trackerlistmodel.h"
#include "tracker.h"

//! Modo de uso da caixa de diálogo: inserção ou edição
enum class TrackerDialogMode {
    New, Edit
};

//!  Caixa de diálogo para inserção ou edição de um satélite/TLE
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

     /*! \brief Construtor para editar um satélite já cadastrado
      *
      * @param model Ponteiro para lista que armazena satélites cadastrados
      * @param tracker Ponteiro para satélite a ser editado
      */
    AddTrackerDialog(TrackerListModel* model, Tracker* tracker, QWidget* parent = 0);
private:
     TrackerDialogMode mode; //!< A caixa de diálogo pode ser usada tanto para adicionar um satélite à lista quando editar um existente
     Tracker* tracker; //!< Caso a caixa de diálogo esteja editando um satélite cadastrado, essa é a referência
     TrackerListModel* model; //!< A lista de satélites do sistema do programa principal
public slots:
     //! Slot chamado ao clicar em "Ok"
     void accept();

};

#endif // ADDTRACKERDIALOG_H
