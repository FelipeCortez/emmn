#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QItemSelection>
#include <QStringListModel>
#include <QStandardItemModel>
#include "trackerlistmodel.h"
#include "addtrackerdialog.h"
#include "settingsdialog.h"
#include "manualcontroldialog.h"
#include "tracker.h"
#include "helpers.h"
#include "network.h"
#include "control.h"

namespace Ui {
class MainWindow;
}

//! Janela principal
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    /*! \brief Habilita botões de manipulação de satélite (prioridade, edição, remoção)
     */
    void enableSatelliteButtons(bool enable = true);

    ~MainWindow();

private:
    Ui::MainWindow* ui;                   //! Ponteiro para elementos de interface gráfica do arquivo .ui
    TrackerListModel* trackedSatellites;  //! Lista de todos os satélites rastreados
    TrackerListModel* satelliteCatalogue; //! Lista de todos os satélites catalogados pelo SpaceTrack
    QTimer satInfoTimer;                  //! Timer para recarregar informações do satélite todo segundo
    QTimer updateTLETimer;                //! Timer para verificar atualização de TLEs
    QStandardItemModel* tableModel;       //! Elementos de tabela de passagens
    Control* control;                     //! Controle da antena
    Logger* logger;                       //! Logger para posição da antena
    Network network;                      //! Conexão com a Internet para atualizar TLEs
    QDateTime prevTime;                   //! Hora lida (para atualizar próximas passagens)

    /*! \brief Modifica porta serial utilizada pelo programa basead
     */
    void setPortFromSettings();

    /*! \brief Carrega satélites salvos persistentemente e insere-os na classe do ListModel
     */
    void loadTrackersFromSettings();

public slots:
    /*! \brief Slot chamado automaticamente ao selecionar-se um satélite
     *
     * Atualiza tabela baseado no satélite selecionado na lista de satélites
     */
   void rowChangedSlot(QItemSelection selected, QItemSelection);

    /*! \brief Slot chamado ao clicar no botão de adicionar satélite
     */
   void addTrackerDialogSlot();

    /*! \brief Slot chamado ao clicar em Arquivo > Configurações
     */
   void settingsDialogSlot(bool);

    /*! \brief Slot chamado ao clicar em Arquivo > Controle manual
     */
   void manualControlDialogSlot(bool);

    /*! \brief Abre pasta contendo logs
     */
    void openLogDirectorySlot(bool);

    /*! \brief Slot chamado ao clicar em Arquivo > Debug
     *
     * Para testes em geral.
     */
   void debugSlot(bool);

    /*! \brief Slot chamado ao clicar em Arquivo > Atualizar TLEs
     */
   void updateTLESlot(bool);

    /*! \brief Envia sinal de power pra antena
     */
    void sendPowerSlot(bool);

    /*! \brief Slot chamado ao selecionar um satélite e clicar em Remover
     */
   void removeSelectedTrackerSlot();

    /*! \brief Slot chamado periodicamente para atualizar informações do satélite selecionado
     */
   void satInfoUpdateSlot();

    /*! \brief Slot chamado ao clicar em Mostrar todas as passagens
     */
   void clearSelectedTrackerSlot();

    /*! \brief Slot chamado ao clicar no botão de aumentar prioridade
     */
   void moveTrackerUpSlot();

    /*! \brief Slot chamado ao clicar no botão de baixar prioridade
     */
   void moveTrackerDownSlot();

    /*! \brief Slot chamado ao clicar em um dos radio buttons
     */
   void modeRadioButtonsChanged(bool);

    /*! \brief Atualiza interface com informação das TLEs mais atualizada
     */
    void updateTrackersListSlot();

    /*! \brief Chamada periodicamente para verificar se TLEs precisam de atualização
     */
    void updateTLECheckSlot();

    /*! \brief Offset do azimute para posicionamento da antena
     */
    void updateAzOffsetSlot();
};

#endif // MAINWINDOW_H
