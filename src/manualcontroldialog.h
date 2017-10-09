#ifndef MANUALCONTROLDIALOG_H
#define MANUALCONTROLDIALOG_H

#include <QDialog>
#include "control.h"
#include "helpers.h"

const int joystickRefreshRate = 1000.0f / 60; //!< Período utilizado por timer

namespace Ui {
class ManualControlDialog;
}

/*! Caixa de diálogo de controle manual (abas com joystick e formulário de posição) */
class ManualControlDialog : public QDialog
{
    Q_OBJECT

public:
    /*! \brief Construtor da caixa de diálogo
     *
     * Inicializa timer do joystick e timer do estado da antena e conecta signals e slots de interação
     */
    explicit ManualControlDialog(Control *control, QWidget *parent = 0);
    ~ManualControlDialog();

private:
    Ui::ManualControlDialog *ui; //!< Ponteiro que dá acesso aos elementos da interface gráfica em manualcontrol_dialog.ui
    Control* control; //!< Ponteiro para enviar mensagens de posicionamento para classe de controle
    QTimer antennaInfoUpdateTimer; //!< Timer para atualizar posição atual da antena
    QTimer joystickRefreshTimer; //!< Timer para redesenhar o joystick
    bool validAz;
    bool validEle;

public slots:
    /*! \brief Slot para mudança de aba escolhida
     *
     * @todo Mandar modificação de método de controle
     */
    void tabChanged(int index);

    /*! \brief Atualiza labels de posição e azimute da antena periodicamente
     */
    void updateAntennaInfo();

    /*! \brief Envia estado do joystick para classe de controle a fim de movimentar a antena
     */
    void sendJoystickDeltas();

    /*! \brief Envia apenas azimute do formulário de posicionamento
     */
    void sendAz();

    /*! \brief Envia apenas elevação do formulário de posicionamento
     */
    void sendEle();

    /*! \brief Envia azimute e elevação do formulário de posicionamento
     */
    void sendBoth();

    /*! \brief Detecta se tanto o azimute quanto a elevação informada são válidos e ativa ou desativa botão de enviar ambos
     */
    void validateBoth();

    /*! \brief Detecta se elevação informada é válida e ativa ou desativa botões
     */
    void validateElevation(QString elevation);

    /*! \brief Detecta se azimute informado é válido e ativa ou desativa botões
     */
    void validateAzimuth(QString azimuth);
};

#endif // MANUALCONTROLDIALOG_H
