#ifndef CONTROL_H
#define CONTROL_H

#include <time.h>
#include <stdio.h>
#include <Windows.h>
#include <QDebug>
#include "efem.h"
#include "helpers.h"
#include "serial.h"
#include "trackerlistmodel.h"
#include "logger.h"

//!  Modo de controle da antena.
enum class ControlMode {
    Schedule, Manual, None
};

//!  Classe responsável pela manipulação da antena
class Control : public QObject
{
    Q_OBJECT

public:
    Control(QString, TrackerListModel* trackerListModel, QObject *parent = 0);
    ~Control();

    /*! \brief Envia para o Arduino posições de azimute e elevação desejadas
     *
     *
     *  @param az Azimute desejado
     *  @param ele Elevação desejada
     */
    void sendPosition(float az, float ele);

    /*! \brief Envia um sinal state pro Arduino e lê a resposta
     *
     *  @todo Verificar se o comando enviado foi entendido
     */
    AzEle getState();

    /*! \brief Envia um sinal de power para o Arduino
     *
     * Utilizado para verificar se porta serial é válida
     *
     */
    bool sendPower();

    /*! \brief Verifica se o último comando enviado foi entendido pelo Arduino
     *
     *  @returns 0 se o comando foi reconhecido, 1 se não foi
     *  @todo Trocar retorno por valor booleano
     */
    int acknowledge(unsigned char *_input_aknow);

    /*! \brief Verifica se o checksum do string de STATE recebido está correto
     *
     *  @param _input_aknow
     *  @returns 0 se o comando foi reconhecido, 1 se não foi
     *  @todo Trocar retorno por valor booleano
     *  @todo Refatorar aknow por ack
     */
    int verifyChecksum();

    /*! \brief Verifica se o último comando recebido pelo PC foi reconhecido
     *
     *  Envia ACK caso reconhecido e NACK caso contrário
     */
    void sendAck(int _erro);

    /*! \brief Altera porta da comunicação com Arduino
     *
     *  @param port Nome da porta (e.g. "COM3")
     */
    void changePort(QString port);

    /*! \brief Atualiza valores alvo para a antena baseados em deltas em vez de posições absolutas
     *
     * @param deltaAz
     * @param deltaEle
     *
     * @todo Mudar para struct AzEle
     */
    void setDeltas(float deltaAz, float deltaEle);

    /*! \brief Atualiza valores alvo para a antena
     *
     * @param az
     * @param ele
     */
    void setTarget(float az, float ele);

    /*! \brief Determina como a posição da antena será atualizada
     *
     * @param controller Modo de controle (manual ou automático)
     *
     * @todo Refatorar para setControlMode
     */
    void setControlMode(ControlMode controlMode);

    /*! \brief Atualiza posição da antena baseado nos valores alvo registrados
     */
    void moveToTarget();

    /*! \brief Verifica se porta escolhida é válida
      */
    bool isPortValid();
private:
    bool validPort; //!< true se porta verdadeira é um dispotivo Arduino válido
    ControlMode controlMode; //!< Modo de controle da antena
    QTimer antennaTimer; //!< Timer responsável por mandar informações para a antena repetidamente
    double speedAz;
    double speedEle;
    double desiredSpeedAz;
    double desiredSpeedEle;
    double accelerationAz;
    double accelerationEle;
    double maxSpeed;
    double maxAcceleration;
    CSerial serial; //!< Classe de comunicação serial
    TrackerListModel* trackerListModel; //!< Ponteiro para lista que armazena todos os satélites cadastrados
    AzEle lastAzEle; //!< Última posição lida
    Logger* logger;
    float targetAz; //!< Azimute alvo para a antena
    float targetEle; //!< Elevação alvo para a antena

    unsigned char a1 = 0, a0 = 0, e1 = 0, e0 = 0; //bytes a serem enviados (refAZ e refELE)
    int cont_aux = 0;

    // Por que não usar 'S', 'P', 'A', 'N', 'X' direto???
    unsigned char setString[6] = {87,48,48,48,48,48}; //Array onde será formado o comando SET a ser enviado para o arduino
    unsigned char input_state[7] = {87,48,48,48,48,48,0}; //Array de comando STATE a ser recebido do arduino
    unsigned char state[1] = { 83 };    // 'S'
    unsigned char power[1] = { 80 };    //'P'
    unsigned char ack[1] = { 65 };      // 'A'
    unsigned char nack[1] = { 78 };     // 'N'
    unsigned char input_ack[1] = { 88 };// 'X' //será utilizada para verificar reconhecimento de mensagens enviadas e recebidas

    //Status do sistema
    int ka1 = 0, ka2 = 0, daz = 0, del = 0, m = 0, p = 0;

public slots:
    //! Lógica de atualização da posição da antena
    void updateSlot();
};

#endif // CONTROL_H
