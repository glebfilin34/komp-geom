#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <vector>
#include <algorithm>
#include <cmath>

class Point {
public:
    QPointF pos;
    bool isDragging = false;
    Point(const QPointF& p) : pos(p) {}
};

class ConvexHullWidget : public QWidget {
    Q_OBJECT

public:
    ConvexHullWidget(QWidget *parent = nullptr);
    void clearPoints();
    void computeConvexHull();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    std::vector<Point> points;
    std::vector<QPointF> convexHull;
    bool onlineMode;

    int orientation(const QPointF& p, const QPointF& q, const QPointF& r);

public slots:
    void setOnlineMode(bool enabled);
};

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private:
    ConvexHullWidget *convexHullWidget;
};

#endif
