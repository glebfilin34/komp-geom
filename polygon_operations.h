#ifndef POLYGON_OPERATIONS_H
#define POLYGON_OPERATIONS_H

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMouseEvent>
#include <QPainter>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stack>

struct Point {
    double x, y;
    Point() : x(0), y(0) {}
    Point(double x, double y) : x(x), y(y) {}

    Point operator+(const Point& other) const { return Point(x + other.x, y + other.y); }
    Point operator-(const Point& other) const { return Point(x - other.x, y - other.y); }
    Point operator*(double scalar) const { return Point(x * scalar, y * scalar); }
    double dot(const Point& other) const { return x * other.x + y * other.y; }
    double cross(const Point& other) const { return x * other.y - y * other.x; }
    double dist2() const { return x*x + y*y; }
};

class Polygon {
public:
    std::vector<Point> points;

    void addPoint(const Point& p) { points.push_back(p); }
    void clear() { points.clear(); }
    bool empty() const { return points.empty(); }
    size_t size() const { return points.size(); }

    void computeConvexHull();
};

class PolygonCanvas : public QWidget {
    Q_OBJECT

public:
    enum Mode { FIRST_POLYGON, SECOND_POLYGON, RESULT };
    enum Operation { INTERSECTION, UNION, DIFFERENCE };

    PolygonCanvas(QWidget *parent = nullptr);
    void setOperation(Operation op);

public slots:
    void nextPolygon();
    void reset();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    double distance2(const Point& a, const Point& b);
    void drawPolygon(QPainter& painter, const Polygon& poly, const QColor& color, bool active);
    void computeResult();
    void computeIntersection();
    void computeUnion();
    void computeDifference();
    bool isPointInsidePolygon(const Point& p, const std::vector<Point>& polygon);
    bool lineSegmentIntersection(const Point& a1, const Point& a2,
                                 const Point& b1, const Point& b2, Point& result);

    Polygon poly1, poly2, result;
    Mode mode;
    Operation operation;
    int movingPoint;
    int currentPolygon;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private:
    PolygonCanvas *canvas;
};

#endif
