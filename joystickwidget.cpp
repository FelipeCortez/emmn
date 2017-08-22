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
    , dragging(false)
    , mouseOffset(0, 0)
    , controlAxes(ControlAxes::free)
{
    limitCircle.setTopLeft(QPointF(0, 0));
    limitCircle.setSize(QSizeF(widgetSize - margins * 2,
                               widgetSize - margins * 2));

    setMouseTracking(true);
    joyCircleCenter = QPointF(0, 0);
}

void JoystickWidget::mouseMoveEvent(QMouseEvent *event) {
    if(dragging) {
        double dx = joyCircleCenter.x();
        double dy = -joyCircleCenter.y();

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
        joyCircleCenter = QPointF(joyRadius *  cos(angle),
                                  joyRadius * -sin(angle));

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
    if(!dragging || controlAxes == ControlAxes::elevationOnly) {
        joyCircleCenter.setX(joyCircleCenter.x() * 0.95);
    }

    if(!dragging || controlAxes == ControlAxes::azimuthOnly) {
        joyCircleCenter.setY(joyCircleCenter.y() * 0.95);
    }

    if(fabs(joyCircleCenter.x()) < 0.1) {
        joyCircleCenter.setX(0);
    }

    if(fabs(joyCircleCenter.y()) < 0.1) {
        joyCircleCenter.setY(0);
    }

    update();
}

AzEle JoystickWidget::getDeltas() {
    AzEle antennaInfo;
    const double maxMov = 1;
    double maxRadius = (limitCircle.width() / 2.0) - (getJoyCircleWidth() / 2.0);
    antennaInfo.azimuth = maxMov * joyCircleCenter.x() / maxRadius;
    antennaInfo.elevation = maxMov * joyCircleCenter.y() / maxRadius;
    return antennaInfo;
}

void JoystickWidget::mousePressEvent(QMouseEvent *event) {
    const double dx = event->pos().x() - getJoyCircleRealCenter().x();
    const double dy = event->pos().y() - getJoyCircleRealCenter().y();
    double mouseToJoyDistance = sqrt(dx * dx + dy * dy);
    if(mouseToJoyDistance < (getJoyCircleWidth() / 2.0)) {
        dragging = true;
        mouseOffset = event->pos() - getJoyCircleRealCenter();
    }
}

void JoystickWidget::mouseReleaseEvent(QMouseEvent *) {
    dragging = false;
}

double JoystickWidget::getJoyCircleWidth() {
    return limitCircle.width() * joyCircleProportion;
}

QPointF JoystickWidget::getJoyCircleRealCenter() {
    return rect().center() + joyCircleCenter;
}

void JoystickWidget::paintEvent(QPaintEvent *) {
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QPointF(0,              rect().center().y() + 0.5),
                     QPointF(rect().width(), rect().center().y() + 0.5));

    painter.drawLine(QPointF(rect().center().x() + 0.5, 0),
                     QPointF(rect().center().x() + 0.5, rect().height()));

    QRectF joyCircle(QPointF(0, 0),
                     QSizeF(getJoyCircleWidth(),
                            getJoyCircleWidth()));

    limitCircle.moveCenter(rect().center());
    joyCircle.moveCenter(getJoyCircleRealCenter());

    painter.drawEllipse(limitCircle);

    painter.setOpacity(0.7);
    painter.setBrush(Qt::gray);
    painter.drawEllipse(joyCircle);
    painter.setOpacity(1);
    painter.end();
}
