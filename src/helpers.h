#ifndef HELPERS_H
#define HELPERS_H

#include <QSettings>
#include <QList>
#include <QtSerialPort/QSerialPortInfo>
#include "tracker.h"
#include "trackerlistmodel.h"
#include "DateTime.h"

typedef struct AzEle {
    double azimuth;
    double elevation;
} AzEle;

//! Funções de configuração
namespace Settings {
    /*! \brief Retorna uma nova instância de settings
     *
     * Função criada para não precisar lembrar dos argumentos de inicialização
     *
     *  @todo Inicializar só no começo do programa e deletar no final
     */
    QSettings* getSettings();

    /*! \brief Salva lista de satélites persistentemente
     */
    void saveTrackers(QList<Tracker> trackers);

    /*! \brief Carrega lista de satélites salvos
     */
    QList<Tracker> loadTrackers();

    /*! \brief Carrega opção das configurações de usar tempo UTC ou GMT-3
     */
    bool getUseLocalTime();

    /*! \brief Salva opção das configurações de usar tempo UTC ou GMT-3
     */
    void setUseLocalTime(bool useLocalTime);

    /*! \brief Carrega nome da porta usada na última sessão do programa
     */
    QString getSerialPort();

    /*! \brief Salva nome da porta usada
     */
    void setSerialPort(QString serialPort);

    /*! \brief Carrega última data de atualização de catálogo das TLEs
     */
    QDateTime getLastUpdatedDate();

    /*! \brief Salva última data de atualização de catálogo das TLEs
     */
    void setLastUpdatedDate(QDateTime date);

}

//! Funções úteis
namespace Helpers {
    const float latitude = -5.7793f;
    const float longitude = -35.201f;
    const float altitude = 0.014f;

    /*! \brief Transforma uma data da classe DateTime (SGP4)  em string
     *
     * Formatação YYYY/MM/DD HH:MM:SS UTC/GMT
     *
     * @param date Classe DateTime da biblioteca SGP4
     */
    QString betterDate(DateTime date);

    /*! \brief Transforma um horário em string
     *
     * Formatação HH:MM
     *
     * @param time Classe DateTime da biblioteca SGP4
     */
    QString betterTime(DateTime time);

    /*! \brief Transforma radianos em graus
     */
    double radToDeg(double rad);

    /*! \brief Limita o módulo de val por um valor max
     *
     * Para -10, 5, saída é -5 <br>
     * Para  10, 5, saída é 5 <br>
     * Para  3, 4, saída é 3 <br>
     * Para  -3, 4, saída é -3 <br>
     *
     * @param val Valor a ser limitado
     * @param max Módulo máximo (limite)
     */
    double clip(double val, double max);

    /*! \brief Retorna uma lista de portas seriais disponíveis
     */
    QList<QSerialPortInfo> getSerialPortsAvailable();

    /*! \brief Salva a mais recentemente obtida lista de TLEs recebida pelo SpaceTrack
     */

    bool saveTLEList(QStringList tleList);

    TrackerListModel* readTLEList();

    QStringListModel *QStringListModelFromSatelliteCatalog(TrackerListModel *trackerList);

    QStringList getSpaceTrackCredentials();

    QStringList findInTLEList(QString catalogNumber);
}

#endif // HELPERS_H
