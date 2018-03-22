#ifndef TRACKERLISTMODEL_H
#define TRACKERLISTMODEL_H

#include <QAbstractListModel>
#include "tracker.h"

/*! Detalhes de uma passagem e o satélite referente à passagem
 *
 * @todo Usar apenas PassDetailsWithTracker, deletar PassDetails
*/
struct PassDetailsWithTracker {
    Tracker* tracker;
    PassDetails passDetails;
};

//! Armazena satélites rastreados em um formato que o Qt pode usar com um widget de lista
class TrackerListModel : public QAbstractListModel
{
public:
    enum Roles {
            IdRole = Qt::UserRole + 1,
            NameRole,
            PassesRole,
            SatCatRole,
    };

    TrackerListModel();

    /*! \brief Adicionar satélite à lista
     *
     * Adicionar um novo satélite gera a lista de passagens novamente
     *
     * @param tracker Referência à classe do satélite
     * @todo Gerar apenas novas passagens e adicioná-las à lista
     */
    QModelIndex addTracker(const Tracker& tracker, bool recalculatePassList = true);

    /*! \brief Retorna o número de satélites armazenados
     *
     * Implementação obrigatória para QAbstractListModel
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /*! \brief Acessa algum parâmetro de algum item da lista
     *
     * Implementação obrigatória para QAbstractListModel
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /*! \brief Muda algum parâmetro de algum item da lista
     *
     * Não implementado corretamente; não utilizado
     *
     * Implementação obrigatória para QAbstractListModel
     */
    bool setData(const QModelIndex &index, const QVariant &, int) override;

    /*! \brief Remove um ou mais satélites da lista
     *
     * Implementação obrigatória para QAbstractListModel
     */
    bool removeRows(int row, int count, const QModelIndex &parent) override;

    /*! \brief Retorna lista de satélites rastreados */
    QList<Tracker> getTrackers();

    /*! \brief Retorna ponteiro para lista de satélites rastreados */
    QList<Tracker>* getTrackersPointer();

    /*! \brief Retorna referência para lista de satélites rastreados */
    QList<Tracker>& getTrackersRef();

    /*! \brief Retorna ponteiro para satélite com satCatNumber informado */
    Tracker* findTracker(QString satCatNumber);

    /*! \brief Gera lista com todas as passagens */
    void generatePassList(const DateTime& start_time = DateTime::Now(true),
                          const DateTime& end_time = DateTime::Now(true).AddHours(24));

    /*! \brief Acessa lista com todas as passagens anteriormente gerada */
    QList<PassDetailsWithTracker> getAllPasses();

    /*! \brief Muda um satélite da lista pelo satélite passado e gera nova lista de passagens */
    void setTracker(int row, Tracker tracker);
private:
    QList<Tracker> trackers;
    QList<PassDetailsWithTracker> allPasses;
};

#endif // TRACKERLISTMODEL_H
