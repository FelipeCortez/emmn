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
    if (trackers != nullptr) {
        QPainter painter;
        int totalHeight = 0;
        const int margins = 10;
        const int trackerHeight = 16;

        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);
        int textMaxWidth = 0;
        for (QList<Tracker>::iterator it = trackers->begin(); it != trackers->end(); ++it) {
            Tracker t = *it;
            if (textMaxWidth < painter.fontMetrics().width(t.getCommonName())) {
                textMaxWidth = painter.fontMetrics().width(t.getCommonName());
            }
        }

        const int hours = 12;
        const auto now = DateTime::Now(false);
        const auto later = DateTime::Now(false).AddHours(hours);
        const auto totalTicks = (later - now).Ticks();

        QRectF xAxisRect(QPointF(margins + textMaxWidth + margins, totalHeight),
                         QPointF(width() - 1 - margins, totalHeight + trackerHeight * trackers->length()));

        // Hora atual (12:25 -> 12:00)
        auto dateIt = now.AddMinutes(60 - now.Minute());

        // Desenha barras com cores alternadas
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(230, 230, 230));

        bool drawBar = true;
        for (QList<Tracker>::iterator it = trackers->begin(); it != trackers->end(); ++it) {
            if (drawBar) {
                const QRectF itemRect(0, totalHeight, width(), trackerHeight);
                painter.drawRect(itemRect);
            }

            drawBar = !drawBar;
            totalHeight += trackerHeight;
        }
        totalHeight = 0;

        // Desenha linhas de referência (eixo X)
        painter.setPen(Qt::gray);
        while (dateIt < later) {
            const auto linePercentage = (float) (dateIt - now).Ticks() / totalTicks;
            float lineX = floor(xAxisRect.left() + (xAxisRect.width() * linePercentage)) + 0.5;
            painter.setOpacity(0.5);
            painter.drawLine(QPointF(lineX, xAxisRect.top()),
                             QPointF(lineX, xAxisRect.bottom()));

            const auto linePercentage2 = (float) (dateIt.AddMinutes(30) - now).Ticks() / totalTicks;
            float lineX2 = floor(xAxisRect.left() + (xAxisRect.width() * linePercentage2)) + 0.5;
            painter.setOpacity(0.2);
            painter.drawLine(QPointF(lineX2, xAxisRect.top()),
                             QPointF(lineX2, xAxisRect.bottom()));

            //dateIt.AddMinutes(30);

            painter.setOpacity(1);
            drawText(painter,
                     QPointF(lineX, xAxisRect.bottom() + margins),
                     Qt::AlignVCenter | Qt::AlignHCenter,
                     Helpers::betterTime(dateIt));
            dateIt = dateIt.AddHours(1);
        }

        // Desenha linha vertical para hora atual
        const QString nowStr(QString::number(now.Hour()) + ":" + QString::number(now.Minute()));
        QRectF boundingRect = painter.fontMetrics().boundingRect(nowStr);
        boundingRect.setWidth(boundingRect.width() + 10);
        boundingRect.moveCenter(QPointF(xAxisRect.left() + margins * 2, xAxisRect.top() + margins / 2 + 1));
        painter.setOpacity(0.5);
        painter.setPen(Qt::red);
        painter.drawLine(QPointF(xAxisRect.left() + 0.5, xAxisRect.top()),
                         QPointF(xAxisRect.left() + 0.5, xAxisRect.bottom()));
        painter.setOpacity(1);

        // Desenha blocos
        for (QList<Tracker>::iterator it = trackers->begin(); it != trackers->end(); ++it) {
            const QRectF itemRect(0, totalHeight, width(), trackerHeight);

            auto tracker = *it;
            painter.setPen(Qt::black);
            drawText(painter,
                     QPointF(margins, itemRect.center().y()),
                     Qt::AlignVCenter,
                     tracker.getCommonName());

            auto passes = tracker.generatePassList(now, later);
            for (QList<PassDetails>::iterator it2 = passes.begin(); it2 != passes.end(); ++it2) {
                auto pass = *it2;
                const auto leftPercentage = (float) (pass.aos - now).Ticks() / totalTicks;
                const auto rightPercentage = (float) (pass.los - now).Ticks() / totalTicks;
                QRectF barRect(itemRect);
                barRect.setTop(itemRect.top() + 3);
                barRect.setBottom(itemRect.bottom() - 3);
                barRect.setLeft(xAxisRect.left() + xAxisRect.width() * leftPercentage);
                barRect.setRight(xAxisRect.left() + xAxisRect.width() * rightPercentage);
                painter.setPen(Qt::NoPen);
                painter.setBrush(Qt::black);
                painter.setOpacity(0.7);
                painter.drawRoundedRect(barRect, 2, 2);
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
