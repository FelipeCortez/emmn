#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QTimer>
#include "helpers.h"

//! Graus de liberdade do joystick
enum class ControlAxes {
    azimuthOnly, elevationOnly, free
};

//! Joystick virtual usado através da janela de controle manual
class JoystickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JoystickWidget(QWidget *parent = 0);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

    /*! \brief Desenha joystick
     */
    void paintEvent(QPaintEvent *);

    /*! \brief Calcula deltas de azimute e elevação baseando-se na posição do joystick
     *
     * @todo Parametrizar delta máximo
     */
    AzEle getDeltas();

private:
    const int widgetSize;             //!< Tamanho da widget do joystick
    const int margins;                //!< Tamanho das margens internas
    const double joyCircleProportion; //!< Razão de proporção entre tamanho do joystick e círculo externo
    QPointF joyCircleCenter;          //!< Armazena posição central do joystick
    QRectF limitCircle;               //!< Retângulo associado ao círculo do limite
    bool dragging;                    //!< Armazena se o joystick está sendo arrastado pelo mouse
    QPointF mouseOffset;              //!< Armazena distância do mouse para o centro do joystick no momento do clique
    ControlAxes controlAxes;          //!< Armazena graus de liberdade disponíveis para movimentação do joystick

    double getJoyCircleWidth();       //!< Calcula tamanho do círculo do joystick
    QPointF getJoyCircleRealCenter(); //!< Calcula centro real (não relativo) do círculo do joystick

signals:

public slots:
    /*! \brief Função chamada múltiplas vezes por segundo para redesenhar o widget e retornar à posição central ao soltar o botão do mouse
     */
    void refreshSlot();

    /*! \brief Determina se joystick está livre para mexer em qualquer direção
     *
     * @param toggle Se true, libera movimento em todas as direções
     */
    void setFreeSlot(bool toggled);

    /*! \brief Determina se joystick está livre para mexer apenas o azimute
     *
     * @param toggle Se true, libera movimento apenas azimutal
     */
    void setAzimuthSlot(bool toggled);

    /*! \brief Determina se joystick está livre para mexer apenas a elevação
     *
     * @param toggle Se true, libera movimento apenas de elevação
     */
    void setElevationSlot(bool toggled);
};

#endif // JOYSTICKWIDGET_H
