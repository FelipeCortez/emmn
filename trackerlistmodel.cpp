#include "trackerlistmodel.h"
#include <QDebug>
#include <algorithm>

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

bool comparePassDetails(PassDetailsWithTracker pd1, PassDetailsWithTracker pd2) {
    return pd1.passDetails.aos < pd2.passDetails.aos;
}

QList<PassDetailsWithTracker> TrackerListModel::getAllPasses(const DateTime& start_time, const DateTime& end_time) {
    QList<PassDetailsWithTracker> allPassList;
    qDebug() << &trackers.at(0);
    // https://stackoverflow.com/questions/15176104/c11-range-based-loop-get-item-by-value-or-reference-to-const
    for(auto &t : trackers) { // '&' avoids copying, making loop variable a reference
        QList<PassDetails> pdList = t.GeneratePassListQt(start_time, end_time);
        //qDebug() << t.getTitle();
        for(auto pd : pdList) {
            //qDebug() << QString::fromStdString(pd.aos.ToString());
            //qDebug() << QString::fromStdString(pd.los.ToString());
            //qDebug() << "/";
            PassDetailsWithTracker pdt;
            pdt.tracker = &t;
            pdt.passDetails = pd;
            allPassList.push_back(pdt);
        }
        //qDebug() << "---";
    }

    qDebug() << "all passes";
    std::sort(allPassList.begin(), allPassList.end(), comparePassDetails);

    for(auto pdt : allPassList) {
        //qDebug() << pdt.tracker->getTitle();
        //qDebug() << QString::fromStdString(pdt.passDetails.aos.ToString());
        //qDebug() << QString::fromStdString(pdt.passDetails.los.ToString());
        //qDebug() << "/";
    }

    return allPassList;
}
