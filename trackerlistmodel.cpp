#include "trackerlistmodel.h"
#include <QDebug>

TrackerListModel::TrackerListModel(QObject*)
{

}

QModelIndex TrackerListModel::addTracker(const Tracker &tracker) {
    int rowIndex = rowCount();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    trackers.push_back(tracker);
    endInsertRows();
    return index(rowIndex);
}

int TrackerListModel::rowCount(const QModelIndex &) const {
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

bool TrackerListModel::setData(const QModelIndex &index, const QVariant &, int) {
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
    if(   row < 0
       || row >= rowCount()
       || row < 0
       || (row + count) > rowCount()) {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    int countLeft = count;
    while(countLeft--) {
        //qDebug() << row + countLeft;
        //qDebug() << "Removing: " << trackers.at(row + countLeft).getTitle();
        trackers.removeAt(row + countLeft);
    }
    endRemoveRows();
    //qDebug() << trackers.length();
    return true;
}

QList<Tracker> TrackerListModel::getTrackers() {
    return trackers;
}

QList<Tracker>* TrackerListModel::getTrackersRef() {
    return &trackers;
}

void TrackerListModel::setTracker(int row, Tracker tracker) {
    trackers[row] = tracker;
}
