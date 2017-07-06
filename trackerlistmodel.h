#ifndef TRACKERLISTMODEL_H
#define TRACKERLISTMODEL_H

#include <QAbstractListModel>
#include "tracker.h"

struct PassDetailsWithTracker {
    Tracker* tracker;
    PassDetails passDetails;
};

class TrackerListModel : public QAbstractListModel
{
public:
    enum Roles {
            IdRole = Qt::UserRole + 1,
            NameRole,
            PassesRole,
    };

    TrackerListModel(QObject* = 0);


    QModelIndex addTracker(const Tracker& tracker);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &, int) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    QList<Tracker> getTrackers();
    QList<Tracker>* getTrackersRef();
    QList<PassDetailsWithTracker> getAllPasses(const DateTime& start_time = DateTime::Now(true),
                                    const DateTime& end_time = DateTime::Now(true).AddHours(24));
    void setTracker(int row, Tracker tracker);
    QList<PassDetailsWithTracker> allPasses;
private:
    QList<Tracker> trackers;
};

#endif // TRACKERLISTMODEL_H
