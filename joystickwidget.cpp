#include "joystickwidget.h"
#include "helpers.h"
#include <QPainter>
#include <QDebug>
#include <cmath>

JoystickWidget::JoystickWidget(QWidget *parent)
    : QWidget(parent)
    , mouseCoords(0, 0)
    , widgetSize(200)
    , margins(10)
    , joyCircleProportion(0.5)
    , joyCircleCenter(0, 0)
    , limitCircle(QPointF(0, 0),
                  QSizeF(widgetSize - margins * 2,
                         widgetSize - margins * 2))
{
    //setMouseTracking(true);
}

void JoystickWidget::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << event->pos();
    qDebug() << rect().center();
    // atan meio confuso porque a coordenada y está invertida
    const double dx = event->pos().x() - rect().center().x();
    const double dy = rect().center().y() - event->pos().y();
    const double angle = atan2(dy, dx);
    const double radius = sqrt(dx * dx + dy * dy);
    const double maxRadius = (limitCircle.width() / 2.0) - (getJoyCircleWidth() / 2.0);
    const double joyRadius = Helpers::clip(radius, maxRadius);

    joyCircleCenter = QPointF(rect().center().x() + joyRadius * cos(angle),
                              rect().center().y() - joyRadius * sin(angle));
    mouseCoords = event->pos();
    repaint();
}

double JoystickWidget::getJoyCircleWidth() {
    return limitCircle.width() * joyCircleProportion;
}

void JoystickWidget::paintEvent(QPaintEvent *) {
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF joyCircle(QPointF(0, 0),
                     QSizeF(getJoyCircleWidth(),
                            getJoyCircleWidth()));

    limitCircle.moveCenter(rect().center());
    joyCircle.moveCenter(joyCircleCenter);

    painter.drawEllipse(limitCircle);
    painter.drawEllipse(joyCircle);
    painter.end();
}
