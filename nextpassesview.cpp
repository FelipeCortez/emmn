#include "nextpassesview.h"
#include <QPainter>
#include <QDebug>

NextPassesView::NextPassesView(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(1);
}

// https://stackoverflow.com/questions/24831484/how-to-align-qpainter-drawtext-around-a-point-not-a-rectangle
void drawText(QPainter & painter, qreal x, qreal y, Qt::Alignment flags,
              const QString & text, QRectF * boundingRect = 0)
{
   const qreal size = 32767.0;
   QPointF corner(x, y - size);
   if (flags & Qt::AlignHCenter) corner.rx() -= size/2.0;
   else if (flags & Qt::AlignRight) corner.rx() -= size;
   if (flags & Qt::AlignVCenter) corner.ry() += size/2.0;
   else if (flags & Qt::AlignTop) corner.ry() += size;
   else flags |= Qt::AlignBottom;
   QRectF rect{corner.x(), corner.y(), size, size};
   painter.drawText(rect, flags, text, boundingRect);
}

void drawText(QPainter & painter, const QPointF & point, Qt::Alignment flags,
              const QString & text, QRectF * boundingRect = {})
{
   drawText(painter, point.x(), point.y(), flags, text, boundingRect);
}

void NextPassesView::paintEvent(QPaintEvent *) {
    QPainter painter;
    QStringList trackerStrings;
    trackerStrings << "SCD1" << "SCD2" << "CBERS-4" << "SMOS" << "BIROS";
    int totalHeight = 0;
    const int margins = 10;
    const int trackerHeight = 18;

    totalHeight += margins;
    painter.begin(this);
    //painter.setRenderHint(QPainter::Antialiasing);
    int textMaxWidth = 0;
    for(QStringList::iterator it = trackerStrings.begin(); it != trackerStrings.end(); ++it) {
        if(textMaxWidth < painter.fontMetrics().width(*it)) {
            textMaxWidth = painter.fontMetrics().width(*it);
        }
    }

    painter.setPen(Qt::gray);

    QRectF xAxisRect(QPointF(margins + textMaxWidth + margins, totalHeight),
                     QPointF(width() - 1 - margins, totalHeight + trackerHeight * trackerStrings.length()));
    int i;
    for(i = 0; i < 10; ++i) {
        float lineX = xAxisRect.left() + (xAxisRect.width() * i / 9);
        painter.drawLine(QPointF(lineX, xAxisRect.top()),
                         QPointF(lineX, xAxisRect.bottom()));

        drawText(painter,
                 QPointF(lineX, xAxisRect.bottom() + margins),
                 Qt::AlignVCenter | Qt::AlignHCenter,
                 "13:30");
    }

    for(QStringList::iterator it = trackerStrings.begin(); it != trackerStrings.end(); ++it) {
        QString satName = *it;
        QRectF itemRect(xAxisRect.left(), totalHeight, xAxisRect.width() / 4, trackerHeight);
        QRectF barRect(itemRect);
        barRect.setTop(itemRect.top() + 3);
        barRect.setBottom(itemRect.bottom() - 3);
        painter.setPen(Qt::black);
        drawText(painter,
                 QPointF(margins, barRect.center().y()),
                 Qt::AlignVCenter,
                 satName);
        painter.setPen(Qt::black);
        painter.drawRect(barRect);
        painter.setPen(Qt::red);
        //painter.drawRect(itemRect);
        totalHeight += trackerHeight;
    }

    totalHeight += margins * 3;
    QRectF fullRect(QPointF(1, 1),
                    QPointF(width() - 1, totalHeight - 1));
    painter.setPen(Qt::red);
    painter.drawRect(fullRect);
    painter.end();

    setMinimumHeight(totalHeight);
}

/*
void NextPassesView::paintEvent(QPaintEvent *) {
    const int rectHeight = 10;
    const int padding = 5;
    const int trackerMargin = 15;
    const int xAxisStart = 50;
    QPainter painter;
    QStringList trackerStrings;
    trackerStrings << "SCD1" << "SCD2" << "CBERS-4" << "FELSAT";
    painter.begin(this);
    //painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, width() - 1, height() - 1);
    painter.setPen(Qt::red);
    painter.drawRect(padding, padding, width() - padding * 2, height() - padding * 2);
    painter.setPen(Qt::black);

    int currentMargin = 10;
    for(QStringList::iterator it = trackerStrings.begin(); it != trackerStrings.end(); ++it) {
        QString current = *it;
        painter.drawText(padding, padding + currentMargin, current);
        painter.drawRect(padding + xAxisStart, padding + currentMargin - 8, 300, rectHeight);
        currentMargin += trackerMargin;
    }

    const int axisSpacing = (width() - (padding + xAxisStart)) / 9;
    int i;
    for(i = 0; i < 10; ++i) {
        QPoint tickPoint(padding + xAxisStart + axisSpacing * i,
                         currentMargin + 10);
        painter.setPen(Qt::gray);
        painter.drawLine(tickPoint + QPoint(0, -currentMargin),
                         tickPoint + QPoint(0, -5));
        painter.setPen(Qt::black);
        drawText(painter,
                 tickPoint,
                 Qt::AlignVCenter | Qt::AlignHCenter,
                 QString::number(i) + "0:00");
    }


    setMinimumHeight(currentMargin + 20);
    painter.end();
}
*/
