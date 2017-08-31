#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QTimer>
#include "helpers.h"

enum class ControlAxes {
    azimuthOnly, elevationOnly, free
};

class JoystickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JoystickWidget(QWidget *parent = 0);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    AzEle getDeltas();

private:
    const int widgetSize;
    const int margins;
    const double joyCircleProportion;
    QPointF joyCircleCenter;
    QRectF limitCircle;
    bool dragging;
    QPointF mouseOffset;
    ControlAxes controlAxes;

    double getJoyCircleWidth();
    QPointF getJoyCircleRealCenter();

signals:

public slots:
    void refreshSlot();
    void setFreeSlot(bool toggled);
    void setAzimuthSlot(bool toggled);
    void setElevationSlot(bool toggled);
};

#endif // JOYSTICKWIDGET_H
