#ifndef DELAUNAY_H
#define DELAUNAY_H

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
#include <set>

class Point {
public:
    QPointF pos;
    bool isDragging = false;
    Point(const QPointF& p) : pos(p) {}
};

struct Triangle {
    int p1, p2, p3;
    Triangle(int a, int b, int c) : p1(a), p2(b), p3(c) {}
    bool operator==(const Triangle& other) const {
        return p1 == other.p1 && p2 == other.p2 && p3 == other.p3;
    }
};

struct Edge {
    int p1, p2;
    Edge(int a, int b) : p1(std::min(a, b)), p2(std::max(a, b)) {}
    bool operator==(const Edge& other) const {
        return p1 == other.p1 && p2 == other.p2;
    }
    bool operator<(const Edge& other) const {
        if (p1 != other.p1) return p1 < other.p1;
        return p2 < other.p2;
    }
};

class DelaunayWidget : public QWidget {
    Q_OBJECT

public:
    DelaunayWidget(QWidget *parent = nullptr);
    void clearPoints();
    void computeDelaunay();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    std::vector<Point> points;
    std::vector<Triangle> triangles;
    bool onlineMode;

    bool isPointInCircumcircle(const QPointF& a, const QPointF& b, const QPointF& c, const QPointF& p);

public slots:
    void setOnlineMode(bool enabled);
};

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private:
    DelaunayWidget *delaunayWidget;
};

#endif
