#ifndef TRACKER_H
#define TRACKER_H

#include <Observer.h>
#include <SGP4.h>
#include <Util.h>
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include "DateTime.h"
#include <vector>
#include <list>
#include <QString>
#include <QList>
#include <QMetaType>

//! Azimute e elevação
struct AzEle {
    double azimuth;
    double elevation;
};

//! Detalhes de uma passagem
struct PassDetails
{
    DateTime aos; //!< Data/hora de aquisição do sinal
    DateTime los; //!< Data/hora de perda do sinal
    double max_elevation; //!< Elevação máxima durante passagem
    bool reverse; //!< Passagem faz antena passar pelo fim de curso ou não
};

// Necessário para tratar PassDetails como QVariant para salvar dados no sistema (serialização)
Q_DECLARE_METATYPE(PassDetails)
Q_DECLARE_METATYPE(QList<PassDetails>)

/*! \brief Armazena TLEs de um satélite
 *
 * @todo Não armazenar localização geográfica (userGeo) nesta classe
 */
class Tracker
{
public:
    enum Satellite {
        Azimuth,
        Elevation,
        Range,
        Latitude,
        Longitude,
        Altitude
    };

    // std::string utilizada pelo construtor para facilitar integração com biblioteca SGP4

    Tracker();
    Tracker(std::vector<std::string> tle_list);
    Tracker(QList<QString> tle_list);
    Tracker(std::string commonName, std::string tle1, std::string tle2);

    /*! \brief Transforma uma data da classe DateTime (SGP4)  em string
     *
     * Formatação YYYY/MM/DD HH:MM:SS UTC/GMT
     *
     * @param date Classe DateTime da biblioteca SGP4
     */
    static QString betterDate(DateTime date);

    /*! \brief Encontra elevação máxima para uma passagem
     *
     * @param aos Acquistion of Signal
     * @param los Loss of Signal
     */
    double FindMaxElevation(
        const DateTime& aos,
        const DateTime& los) const;

    /*! \brief Encontra horário em que passagem começa
     *
     * @param initial_time1 Horário inicial
     * @param initial_time2 Horário final
     */
    DateTime FindCrossingPoint(
        const DateTime& initial_time1,
        const DateTime& initial_time2,
        bool finding_aos) const;

    /*! \brief Gera lista de passagens entre dois timestamps
     *
     * @param start_time Horário inicial
     * @param end_time Horário final
     * @param time_step Valor do "passo" em segundos
     */
    QList<PassDetails> generatePassList(
        const DateTime& start_time = DateTime::Now(true),
        const DateTime& end_time = DateTime::Now(true).AddDays(7.0),
        const int time_step = 180);
    QList<PassDetails> getPassList() const;
    bool isPassReverse(PassDetails pd) const;
    QString nextPass();
    QString getCommonName() const;
    QString getFullTLE() const;
    QString getSatInfo(int info) const;
    QString getSatCatNumber() const;
    double getGeographicalAzimuth();
    double getMechanicalAzimuth();
    double getGeographicalElevation();
    AzEle getGeographicalAzEleAtTime(DateTime time) const;
    QList<QString> getTle();
    void setTle(QList<QString> tle);
    friend QDataStream &operator <<(QDataStream &stream, const Tracker &val);
    friend QDataStream &operator >>(QDataStream &stream, Tracker &val);
private:
    CoordGeodetic userGeo;
    QString commonName;
    QString tle1;
    QString tle2;
    QString satelliteCatalogNumber;
    QList<PassDetails> passList;
};

Q_DECLARE_METATYPE(Tracker)

#endif // TRACKER_H
