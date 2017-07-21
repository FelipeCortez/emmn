#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

#include <QWidget>
#include <QMouseEvent>

class JoystickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JoystickWidget(QWidget *parent = 0);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);

private:
    QPointF mouseCoords;
    const int widgetSize;
    const int margins;
    const double joyCircleProportion;
    QPointF joyCircleCenter;
    QRectF limitCircle;

    double getJoyCircleWidth();

signals:

public slots:
};

#endif // JOYSTICKWIDGET_H
