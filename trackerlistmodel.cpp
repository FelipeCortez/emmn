#include "trackerlistmodel.h"
#include <QDebug>

TrackerListModel::TrackerListModel(QObject* parent)
{
    /*
    trackers.append(Tracker("SCD 1                   ",
                     "1 22490U 93009B   17123.70008125  .00000249  00000-0  18216-4 0  9992",
                     "2 22490  24.9694 217.7757 0042800 225.1509 269.3688 14.44484423278880"));
    trackers.append(Tracker("SCD 2                   ",
                            "1 25504U 98060A   17135.94005337  .00000226  00000-0  12993-4 0  9995",
                            "2 25504  24.9970 345.3437 0017313  29.8315 104.9927 14.44006564980273"));
                            */
}

QModelIndex TrackerListModel::addTracker(const Tracker &tracker) {
    int rowIndex = rowCount();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    trackers.push_back(tracker);
    endInsertRows();
    return index(rowIndex);
}

int TrackerListModel::rowCount(const QModelIndex &parent) const {
    return trackers.size();
}

QVariant TrackerListModel::data(const QModelIndex &index, int role) const {
    if(index.row() >= trackers.size()) {
       return QVariant();
    }

    switch(role) {
        case Roles::IdRole:
            return index.row();
        case Roles::NameRole:
        case Qt::DisplayRole:
            return trackers[index.row()].getTitle();
        case Roles::PassesRole:
            return QVariant::fromValue(trackers[index.row()].GeneratePassListQt());
        default:
            return QVariant();
    }
}

bool TrackerListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(index.row() >= trackers.size()) {
       return false;
    }

    /*
    switch(role) {
        case Roles::NameRole:
        case Qt::DisplayRole:
            trackers[index.row()].setTitle(value.toString());
            return true;
        case
    }
    */

    return false;
}

bool TrackerListModel::removeRows(int row, int count, const QModelIndex &parent) {

}

QList<Tracker> TrackerListModel::getTrackers() {
    return trackers;
}
