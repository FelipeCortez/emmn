#include "trackerlistmodel.h"
#include <QDebug>
#include <algorithm>

TrackerListModel::TrackerListModel() {}

QModelIndex TrackerListModel::addTracker(const Tracker &tracker, bool recalculatePassList) {
    int rowIndex = rowCount();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    trackers.push_back(tracker);
    endInsertRows();
    if (recalculatePassList) { generatePassListForAllTrackers(); }
    return index(rowIndex);
}

int TrackerListModel::rowCount(const QModelIndex &) const {
    return trackers.size();
}

QVariant TrackerListModel::data(const QModelIndex &index, int role) const {
    if (index.row() >= trackers.size()) {
       return QVariant();
    }

    switch(role) {
        case Roles::IdRole:
            return index.row();
        case Roles::NameRole:
        case Qt::DisplayRole:
            return trackers[index.row()].getCommonName();
        case Roles::PassesRole:
            return QVariant::fromValue(trackers[index.row()].getPassList());
        case Roles::SatCatRole:
            return QVariant::fromValue(trackers[index.row()].getSatCatNumber());
        default:
            return QVariant();
    }
}

bool TrackerListModel::setData(const QModelIndex &index, const QVariant &, int) {
    if (index.row() >= trackers.size()) {
       return false;
    }

    return false;
}

bool TrackerListModel::removeRows(int row, int count, const QModelIndex &parent) {
    if (   row < 0
       || row >= rowCount()
       || row < 0
       || (row + count) > rowCount())
    {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    int countLeft = count;
    while (countLeft--) {
        trackers.removeAt(row + countLeft);
    }

    endRemoveRows();
    generatePassListForAllTrackers();

    return true;
}

QList<Tracker> TrackerListModel::getTrackers() {
    return trackers;
}

QList<Tracker>* TrackerListModel::getTrackersPointer() {
    return &trackers;
}

QList<Tracker>& TrackerListModel::getTrackersRef() {
    return trackers;
}

Tracker* TrackerListModel::findTracker(QString satCatNumber) {
    for (auto &t : trackers) {
        if (t.getSatCatNumber() == satCatNumber) {
            return &t;
        }
    }

    return nullptr;
}

bool comparePassDetails(PassDetailsWithTracker pd1, PassDetailsWithTracker pd2) {
    return pd1.passDetails.aos < pd2.passDetails.aos;
}

void TrackerListModel::generatePassListForAllTrackers() {
    QList<PassDetailsWithTracker> allPassList;
    for (auto &t : trackers) {
        QList<PassDetails> pdList = t.generatePassList();
        for (auto pd : pdList) {
            PassDetailsWithTracker pdt;
            pdt.tracker = &t;
            pdt.passDetails = pd;
            allPassList.push_back(pdt);
        }
    }

    std::sort(allPassList.begin(), allPassList.end(), comparePassDetails);
    allPasses = allPassList;
}

QList<PassDetailsWithTracker> TrackerListModel::getAllPasses() {
    if (allPasses.empty()) {
        return QList<PassDetailsWithTracker>();
    }

    auto it = allPasses.begin();
    int pos = 0;

    while (it->passDetails.los < DateTime::Now()) {
        ++it;
        ++pos;
    }

    return allPasses.mid(pos);
}

void TrackerListModel::setTracker(int row, Tracker tracker) {
    trackers[row] = tracker;
    generatePassListForAllTrackers();
}

