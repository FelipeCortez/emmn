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

enum class Controller {
    Schedule, Manual
};

class Control : public QObject
{
    Q_OBJECT

public:
    Control(QString, TrackerListModel* trackerListModel, QObject *parent = 0);
    ~Control();

    /** \brief Envia para o Arduino posições de azimute e elevação desejadas
     *
     *
     *  @param az Azimute desejado
     *  @param ele Elevação desejada
     */
    void send_set(float az, float ele);

    /** \brief Envia um sinal state pro Arduino e lê a resposta
     *
     *  Um nome mais apropriado talvez seria "get state".
     *
     *  @todo Verificar se o comando enviado foi entendido
     */
    AzEle send_state();

    /** \brief Envia um sinal de power para o Arduino
     *
     * Utilizado para verificar se porta serial é válida
     *
     */
    bool send_power();

    /** \brief Verifica se o último comando enviado foi entendido pelo Arduino
    *
    *  @returns 0 se o comando foi reconhecido, 1 se não foi
    *  @todo Trocar retorno por valor booleano
    */
    int reconhecimento_arduino(unsigned char *_input_aknow);

    /** \brief Verifica se o checksum do string de STATE recebido está correto
    *
    *  @param _input_aknow
    *  @returns 0 se o comando foi reconhecido, 1 se não foi
    *  @todo Trocar retorno por valor booleano
    *  @todo Refatorar aknow por ack
    */
    int verifica_checksum();

    /** \brief Verifica se o último comando recebido pelo PC foi reconhecido
      *
      *  Envia ACK caso reconhecido e NACK caso contrário
      *
      *  @param _erro
      */
    void envia_reconhecimento(int _erro);

    void changePort(QString port);
    void setDeltas(float deltaAz, float deltaEle);
    void setTarget(float az, float ele);
    void setController(Controller controller);
    void moveToTarget();
    void updateAntennaPosition();
    bool isPortValid();
private:
    CSerial serial;
    bool validPort;
    Controller controller;
    QTimer antennaTimer;
    TrackerListModel* trackerListModel;
    float az;
    float ele;
    float targetAz;
    float targetEle;
    time_t ef_time; // Horário da proxima efeméride em segundos desde 1 de janeiro de 1970 (Unix time)

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
    void updateSlot();
};

#endif // CONTROL_H
