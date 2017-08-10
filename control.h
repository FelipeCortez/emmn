#ifndef CONTROL_H
#define CONTROL_H

#include <time.h>
#include <stdio.h>
#include <Windows.h>
#include <QDebug>
#include "efem.h"
#include "serial.h"
#include "trackerlistmodel.h"

class Control
{
public:
    Control(const wchar_t *port, TrackerListModel* trackerListModel);
    ~Control();

    /*
    *   FUNÇÕES PARA CARREGAMENTO E TRATAMENTO DAS EFEMÉRIDES ---------------------
    */

    /** \brief Conta passagens para um dado arquivo
    *
    *  Abre o arquivo arq_name, armazena o nome em sat_name e retorna o número de
    *  passagens identificado ou -1 se houve erro na leitura do arquivo
    *
    *  @param arq_name o nome do arquivo que contém a tabela
    *  @param _sat_name receberá o nome do satélite lido no arquivo
    *  @return o número de passagens lido no arquivo
    *  @todo Acrescentar contagem do numero de efemérides para futura utilização
    */

    int cont_passagem(char *arq_name, char *_sat_name);

    /** \brief Atualiza array global passagem com dados do arquivo
    *
    *  Abre um arquivo de efemérides, e passa os dados de uma passagem
    *  identificada no arquivo para um objeto Efem (hora de inicio e fim, duração,
    *  máxima elevação, azimute, elevação e hora das efemerides) apontado pela
    *  variável passagem. Os parâmetros _n_pass0 e _n_pass1 serão utilizados como
    *  limites do intervalo das passagens que serão carregadas. Ex: se um segundo
    *  arquivo carregado tem 10 passagens e o primeiro tem 5, então, para o
    *  segundo arquivo, _n_pass0 = 5 e _n_pass1 = 15.  caso a função não consiga
    *  carregar o arquivo, ela retornará -1. Caso contrário, retornará 0.
    *
    *  @param arq_name o nome do arquivo que contém a tabela
    *  @param _n_pass0 limite inferior do intervalo de passagens
    *  @param _n_pass1 limite superior do intervalo de passagens
    *  @param _sat_name receberá o nome do satélite lido no arquivo
    *  @return o número de passagens lido no arquivo
    */

    int load_efem(char *_arq_name, int _n_pass0, int _n_pass1, char *_sat_name);

    /** \brief Realiza ajustes necessários nas passagens
    *
    *  Esta função realiza os ajustes necessários nas passagens: offset magnetico
    *  e ajuste em caso de passagem pelo final de curso.
    *
    *  @param _n_pass número total de passagens
    */

    void tratar_efem(int _n_pass);

    /** \brief Ordena passagens em ordem cronológica crescente
    *
    *  @param _n_pass número total de passagens
    */

    void ordenar_passagens(int _n_pass);

    /* FALTA FAZER:
    -Alocação dinâmica de memoria para as efemérides(AZ, ELE, RANGE, ETC): Utilizar função cont_pass
    para realizar contagem de cada efeméride;
    -Ajustar o programa para realizar rastreios de passagens que já estão ocorrendo
    */


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
    QMap<QString, float> send_state();

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

    void setTarget(float az, float ele);
    void moveToTarget();
    void updateAntennaPosition();
private:
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
    int erro_ack = 0; //Variavel auxiliar para detecção de erros

    //arrays que armazenarão rampas de posicionamento inicial e final
    float rampa_AZ[30] = { 0 };
    float rampa_ELE[30] = { 0 };

    //contador de erros de comunicação
    int cont_erro = 0;

    //Status do sistema
    int ka1 = 0, ka2 = 0, daz = 0, del = 0, m = 0, p = 0;


};

#endif // CONTROL_H
