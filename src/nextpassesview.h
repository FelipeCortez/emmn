#ifndef NEXTPASSESVIEW_H
#define NEXTPASSESVIEW_H

#include <QWidget>
#include "tracker.h"

class NextPassesView : public QWidget
{
    Q_OBJECT
public:
    explicit NextPassesView(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);
    void setTrackers(QList<Tracker>* trackers);
private:
    QList<Tracker>* trackers;

signals:

public slots:
};

#endif // NEXTPASSESVIEW_H
