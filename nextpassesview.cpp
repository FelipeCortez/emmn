#include "nextpassesview.h"
#include "helpers.h"
#include <QPainter>
#include <QDebug>

NextPassesView::NextPassesView(QWidget *parent)
    : QWidget(parent)
    , trackers(nullptr)
{
    setMouseTracking(true);
    setMinimumHeight(1);
}

void NextPassesView::setTrackers(QList<Tracker>* trackers) {
    this->trackers = trackers;
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
    if(trackers != nullptr) {
        QPainter painter;
        int totalHeight = 0;
        const int margins = 10;
        const int trackerHeight = 16;

        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);
        int textMaxWidth = 0;
        for(QList<Tracker>::iterator it = trackers->begin(); it != trackers->end(); ++it) {
            Tracker t = *it;
            if(textMaxWidth < painter.fontMetrics().width(t.getTitle())) {
                textMaxWidth = painter.fontMetrics().width(t.getTitle());
            }
        }

        const int hours = 12;
        auto now = DateTime::Now(false);
        auto later = DateTime::Now(false).AddHours(hours);
        auto totalTicks = (later - now).Ticks();

        QRectF xAxisRect(QPointF(margins + textMaxWidth + margins, totalHeight),
                         QPointF(width() - 1 - margins, totalHeight + trackerHeight * trackers->length()));
        auto hourNow = now.AddMinutes(60 - now.Minute());
        auto dateIt = hourNow;

        // Desenha linhas de referência (eixo X)
        painter.setPen(Qt::gray);
        while(dateIt < later) {
            auto linePercentage = (float) (dateIt - now).Ticks() / totalTicks;
            float lineX = floor(xAxisRect.left() + (xAxisRect.width() * linePercentage)) + 0.5;
            painter.drawLine(QPointF(lineX, xAxisRect.top()),
                             QPointF(lineX, xAxisRect.bottom()));

            drawText(painter,
                     QPointF(lineX, xAxisRect.bottom() + margins),
                     Qt::AlignVCenter | Qt::AlignHCenter,
                     Helpers::betterTime(dateIt));
            dateIt = dateIt.AddHours(1);
        }

        // Desenha linha vertical para hora atual
        QString nowStr(QString::number(now.Hour()) + ":" + QString::number(now.Minute()));
        QRectF boundingRect = painter.fontMetrics().boundingRect(nowStr);
        boundingRect.setWidth(boundingRect.width() + 10);
        boundingRect.moveCenter(QPointF(xAxisRect.left() + margins * 2, xAxisRect.top() + margins / 2 + 1));
        painter.drawLine(QPointF(xAxisRect.left() + 0.5, xAxisRect.top()),
                         QPointF(xAxisRect.left() + 0.5, xAxisRect.bottom()));

        // Desenha blocos
        for(QList<Tracker>::iterator it = trackers->begin(); it != trackers->end(); ++it) {
            QRectF itemRect(0, totalHeight, width(), trackerHeight);

            auto tracker = *it;
            painter.setPen(Qt::black);
            drawText(painter,
                     QPointF(margins, itemRect.center().y()),
                     Qt::AlignVCenter,
                     tracker.getTitle());

            auto passes = tracker.GeneratePassListQt(now, later);
            for(QList<PassDetails>::iterator it2 = passes.begin(); it2 != passes.end(); ++it2) {
                auto pass = *it2;
                auto leftPercentage = (float) (pass.aos - now).Ticks() / totalTicks;
                auto rightPercentage = (float) (pass.los - now).Ticks() / totalTicks;
                QRectF barRect(itemRect);
                barRect.setTop(itemRect.top() + 3);
                barRect.setBottom(itemRect.bottom() - 3);
                barRect.setLeft(xAxisRect.left() + xAxisRect.width() * leftPercentage);
                barRect.setRight(xAxisRect.left() + xAxisRect.width() * rightPercentage);
                painter.setPen(Qt::NoPen);
                painter.setBrush(Qt::black);
                painter.setOpacity(0.7);
                //painter.setBrush(Qt::black);
                painter.drawRoundedRect(barRect, 3, 3);
                painter.setBrush(Qt::NoBrush);
                painter.setOpacity(1);
            }

            totalHeight += trackerHeight;
        }

        // Desenha hora atual
        painter.setPen(Qt::NoPen);
        QColor transpBgColor = palette().color(QPalette::Window);
        transpBgColor.setAlphaF(0.7);
        painter.setBrush(transpBgColor);
        QRectF textBgRect(boundingRect);
        painter.drawRect(textBgRect);
        painter.setPen(Qt::gray);
        drawText(painter,
                 QPointF(xAxisRect.left() + margins, xAxisRect.top()),
                 Qt::AlignTop,
                 Helpers::betterTime(now));

        totalHeight += margins * 2;

        painter.end();

        setMinimumHeight(totalHeight);
    }
}
