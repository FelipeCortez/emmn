#include "joystickwidget.h"
#include "helpers.h"
#include <QPainter>
#include <QDebug>
#include <cmath>

JoystickWidget::JoystickWidget(QWidget *parent)
    : QWidget(parent)
    , widgetSize(200)
    , margins(10)
    , joyCircleProportion(0.5)
    , joyCircleCenter(0, 0)
    , limitCircle(QPointF(0, 0),
                  QSizeF(widgetSize - margins * 2,
                         widgetSize - margins * 2))
    , dragging(false)
    , mouseOffset(0, 0)
    , refreshTimer(this)
    , controlAxes(ControlAxes::free)
{
    setMouseTracking(true);
    joyCircleCenter = QPointF(-1, -1);
    refreshTimer.start(1000.0 / 60);

    //if(joyCircleCenter == QPointF(-1, -1)) {
        joyCircleCenter = QPointF(99, 99);
    //}

    connect(&refreshTimer,
            SIGNAL(timeout()),
            this,
            SLOT(refreshSlot()));
}

void JoystickWidget::mouseMoveEvent(QMouseEvent *event) {
    if(dragging) {
        double dx;
        double dy;

        if(controlAxes != ControlAxes::elevationOnly) {
            dx = event->pos().x() - rect().center().x() - mouseOffset.x();
        }

        if(controlAxes != ControlAxes::azimuthOnly) {
            dy = rect().center().y() - event->pos().y() + mouseOffset.y();
        }

        // atan meio confuso porque a coordenada y está invertida
        const double angle = atan2(dy, dx);
        const double radius = sqrt(dx * dx + dy * dy);
        const double maxRadius = (limitCircle.width() / 2.0) - (getJoyCircleWidth() / 2.0);
        const double joyRadius = Helpers::clip(radius, maxRadius);
        joyCircleCenter = QPointF(rect().center().x() + joyRadius * cos(angle),
                                  rect().center().y() - joyRadius * sin(angle));

        repaint();
    }
}

void JoystickWidget::setFreeSlot(bool toggled) {
    if(toggled) {
        controlAxes = ControlAxes::free;
    }
}

void JoystickWidget::setAzimuthSlot(bool toggled) {
    if(toggled) {
        controlAxes = ControlAxes::azimuthOnly;
    }
}

void JoystickWidget::setElevationSlot(bool toggled) {
    if(toggled) {
        controlAxes = ControlAxes::elevationOnly;
    }
}

void JoystickWidget::refreshSlot() {
    if(!dragging) {
        joyCircleCenter.setX(rect().center().x() + (-rect().center().x() + joyCircleCenter.x()) * 0.995);
        joyCircleCenter.setY(rect().center().y() + (-rect().center().y() + joyCircleCenter.y()) * 0.995);
    }

    update();
}

void JoystickWidget::mousePressEvent(QMouseEvent *event) {
    const double dx = event->pos().x() - joyCircleCenter.x();
    const double dy = joyCircleCenter.y() - event->pos().y();
    double mouseToJoyDistance = sqrt(dx * dx + dy * dy);
    if(mouseToJoyDistance < (getJoyCircleWidth() / 2.0)) {
        dragging = true;
        mouseOffset = event->pos() - joyCircleCenter;
        qDebug() << mouseOffset;
    }
}

void JoystickWidget::mouseReleaseEvent(QMouseEvent *event) {
    dragging = false;
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
