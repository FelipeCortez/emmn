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

class Control : public QObject
{
    Q_OBJECT

public:
    Control(const wchar_t *port, TrackerListModel* trackerListModel, QObject *parent = 0);
    ~Control();

    /*
     *   FUNÇÕES PARA COMUNICAÇÃO SERIAL ARDUINO - PC -----------------------------
     */

    /** \brief Gera uma rapa
    *
    *  Essa função recebe como parametros dois arrays onde serão armazenados os
    *  valores da rampa calculada (30 valores). Ela utiliza os valores atuais de
    *  refAZ, refELE (referências atuais) e AZ e ELE (posição atual da antena) para
    *  calcular as rampas.
    *
    *  @param _rampa_AZ azimute atual da antena
    *  @param _rampa_ELE elevação atual da antena
    */
    void gerar_rampa(float *_rampa_AZ, float *_rampa_ELE);

    /** \brief Send set
     *
     *  @param AZ
     *  @param ELE
     *  @param _cont_erro
     */
    void send_set(float az, float ele); //OK

    /** \brief Send state - Um nome mais apropriado talvez seria "get state". Envia um sinal state pro Arduino e lê a resposta
     *
     *  @todo verificar se o comando enviado foi entendido
     */
    AzEle send_state();

    /** \brief Send power???
    *
    *  @todo verificar se o comando enviado foi entendido
    */
    void send_power(void);

    /** \brief Verifica se o último comando enviado foi entendido pelo Arduino
    *
    *  @returns 0 se o comando foi reconhecido, 1 se não foi
    *  @todo trocar essa ordem...
    */
    int reconhecimento_arduino(unsigned char *_input_aknow);

    /** \brief Verifica se o checksum do string de STATE recebido está correto
    *
    *  @param _input_aknow
    *  @returns 0 se o comando foi reconhecido, 1 se não foi
    *  @todo trocar essa ordem...
    *  @todo trocar aknow por ack?
    */
    int verifica_checksum();

    /** \brief Envia para o arduino se o ultimo comando recebido pelo PC foi reconhecido
    *
    *  Envia ACK caso reconhecido e NACK caso contrário
    *
    *  @param _erro
    */
    void envia_reconhecimento(int _erro);

    void setDeltas(float deltaAz, float deltaEle);
    void setTarget(float az, float ele);
    void moveToTarget();
    void updateAntennaPosition();
private:
    TrackerListModel* trackerListModel;
    float az;
    float ele;
    float targetAz;
    float targetEle;
    QTimer antennaTimer;
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
    int erro_ack = 0; //Variavel auxiliar para detecção de erros

    //arrays que armazenarão rampas de posicionamento inicial e final
    float rampa_AZ[30] = { 0 };
    float rampa_ELE[30] = { 0 };

    //contador de erros de comunicação
    int cont_erro = 0;

    //Status do sistema
    int ka1 = 0, ka2 = 0, daz = 0, del = 0, m = 0, p = 0;

public slots:
    void updateSlot();
};

#endif // CONTROL_H
