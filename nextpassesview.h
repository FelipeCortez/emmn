#ifndef NEXTPASSESVIEW_H
#define NEXTPASSESVIEW_H

#include <QWidget>

class NextPassesView : public QWidget
{
    Q_OBJECT
public:
    explicit NextPassesView(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);

signals:

public slots:
};

#endif // NEXTPASSESVIEW_H
