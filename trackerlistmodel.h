#ifndef TRACKERLISTMODEL_H
#define TRACKERLISTMODEL_H

#include <QAbstractListModel>
#include "tracker.h"

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
private:
    QList<Tracker> trackers;
};

#endif // TRACKERLISTMODEL_H
