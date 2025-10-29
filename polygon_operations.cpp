#include "polygon_operations.h"

void Polygon::computeConvexHull() {
    if (points.size() < 3) return;

    int n = points.size();
    int minIdx = 0;
    for (int i = 1; i < n; i++) {
        if (points[i].y < points[minIdx].y ||
            (points[i].y == points[minIdx].y && points[i].x < points[minIdx].x)) {
            minIdx = i;
        }
    }
    std::swap(points[0], points[minIdx]);

    Point pivot = points[0];
    std::sort(points.begin() + 1, points.end(), [pivot](const Point& a, const Point& b) {
        Point vecA = a - pivot;
        Point vecB = b - pivot;
        double cross = vecA.cross(vecB);
        if (std::abs(cross) > 1e-9) return cross > 0;
        return vecA.dist2() < vecB.dist2();
    });

    std::vector<Point> hull;
    hull.push_back(points[0]);
    hull.push_back(points[1]);

    for (int i = 2; i < n; i++) {
        while (hull.size() >= 2) {
            Point& p1 = hull[hull.size() - 2];
            Point& p2 = hull[hull.size() - 1];
            Point& p3 = points[i];

            if ((p2 - p1).cross(p3 - p1) <= 0) {
                hull.pop_back();
            } else {
                break;
            }
        }
        hull.push_back(points[i]);
    }

    points = hull;
}

PolygonCanvas::PolygonCanvas(QWidget *parent) : QWidget(parent), mode(FIRST_POLYGON),
    movingPoint(-1), currentPolygon(-1), operation(INTERSECTION) {
    setMouseTracking(true);
}

void PolygonCanvas::setOperation(Operation op) { operation = op; }

void PolygonCanvas::nextPolygon() {
    if (mode == FIRST_POLYGON) {
        poly1.computeConvexHull();
        mode = SECOND_POLYGON;
    } else if (mode == SECOND_POLYGON) {
        poly2.computeConvexHull();
        computeResult();
        mode = RESULT;
    }
    update();
}

void PolygonCanvas::reset() {
    poly1.clear();
    poly2.clear();
    result.clear();
    mode = FIRST_POLYGON;
    movingPoint = -1;
    currentPolygon = -1;
    update();
}

void PolygonCanvas::mousePressEvent(QMouseEvent *event) {
    if (mode == RESULT) return;

    Point p(event->pos().x(), event->pos().y());

    if (event->button() == Qt::RightButton) {
        for (int i = 0; i < poly1.size(); i++) {
            if (distance2(p, poly1.points[i]) < 100) {
                poly1.points.erase(poly1.points.begin() + i);
                update();
                return;
            }
        }
        for (int i = 0; i < poly2.size(); i++) {
            if (distance2(p, poly2.points[i]) < 100) {
                poly2.points.erase(poly2.points.begin() + i);
                update();
                return;
            }
        }
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (mode == FIRST_POLYGON) {
            for (int i = 0; i < poly1.size(); i++) {
                if (distance2(p, poly1.points[i]) < 100) {
                    movingPoint = i;
                    currentPolygon = 1;
                    return;
                }
            }
            poly1.addPoint(p);
        } else if (mode == SECOND_POLYGON) {
            for (int i = 0; i < poly2.size(); i++) {
                if (distance2(p, poly2.points[i]) < 100) {
                    movingPoint = i;
                    currentPolygon = 2;
                    return;
                }
            }
            poly2.addPoint(p);
        }
        update();
    }
}

void PolygonCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (movingPoint >= 0) {
        Point p(event->pos().x(), event->pos().y());
        if (currentPolygon == 1) {
            poly1.points[movingPoint] = p;
        } else if (currentPolygon == 2) {
            poly2.points[movingPoint] = p;
        }
        update();
    }
}

void PolygonCanvas::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        movingPoint = -1;
        currentPolygon = -1;
    }
}

void PolygonCanvas::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        nextPolygon();
    }
}

void PolygonCanvas::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::white);

    if (mode != RESULT) {
        drawPolygon(painter, poly1, Qt::blue, mode == FIRST_POLYGON);
        drawPolygon(painter, poly2, Qt::red, mode == SECOND_POLYGON);
    } else {
        drawPolygon(painter, poly1, Qt::blue, false);
        drawPolygon(painter, poly2, Qt::red, false);
        drawPolygon(painter, result, Qt::green, true);
    }
}

double PolygonCanvas::distance2(const Point& a, const Point& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return dx*dx + dy*dy;
}

void PolygonCanvas::drawPolygon(QPainter& painter, const Polygon& poly, const QColor& color, bool active) {
    if (poly.empty()) return;

    QPen pen(color);
    pen.setWidth(active ? 3 : 2);
    painter.setPen(pen);

    for (size_t i = 0; i < poly.points.size(); i++) {
        size_t next = (i + 1) % poly.points.size();
        painter.drawLine(QPointF(poly.points[i].x, poly.points[i].y),
                         QPointF(poly.points[next].x, poly.points[next].y));
    }

    painter.setBrush(active ? color.lighter(150) : Qt::NoBrush);
    if (poly.points.size() >= 3) {
        QPolygonF qpoly;
        for (const auto& p : poly.points) {
            qpoly << QPointF(p.x, p.y);
        }
        painter.drawPolygon(qpoly);
    }

    for (const auto& p : poly.points) {
        painter.setBrush(color);
        painter.drawEllipse(QPointF(p.x, p.y), 5, 5);
    }
}

void PolygonCanvas::computeResult() {
    result.clear();

    switch (operation) {
    case INTERSECTION:
        computeIntersection();
        break;
    case UNION:
        computeUnion();
        break;
    case DIFFERENCE:
        computeDifference();
        break;
    }
}

void PolygonCanvas::computeIntersection() {
    std::vector<Point> points1 = poly1.points;
    std::vector<Point> points2 = poly2.points;

    for (const auto& p : points1) {
        if (isPointInsidePolygon(p, points2)) {
            result.addPoint(p);
        }
    }

    for (const auto& p : points2) {
        if (isPointInsidePolygon(p, points1)) {
            result.addPoint(p);
        }
    }

    for (size_t i = 0; i < points1.size(); i++) {
        size_t next_i = (i + 1) % points1.size();
        for (size_t j = 0; j < points2.size(); j++) {
            size_t next_j = (j + 1) % points2.size();

            Point intersect;
            if (lineSegmentIntersection(points1[i], points1[next_i],
                                        points2[j], points2[next_j], intersect)) {
                result.addPoint(intersect);
            }
        }
    }

    if (!result.empty()) {
        result.computeConvexHull();
    }
}

void PolygonCanvas::computeUnion() {
    std::vector<Point> allPoints;
    allPoints.insert(allPoints.end(), poly1.points.begin(), poly1.points.end());
    allPoints.insert(allPoints.end(), poly2.points.begin(), poly2.points.end());

    for (const auto& p : allPoints) {
        result.addPoint(p);
    }

    if (!result.empty()) {
        result.computeConvexHull();
    }
}

void PolygonCanvas::computeDifference() {
    std::vector<Point> points1 = poly1.points;
    std::vector<Point> points2 = poly2.points;

    for (const auto& p : points1) {
        if (!isPointInsidePolygon(p, points2)) {
            result.addPoint(p);
        }
    }

    for (size_t i = 0; i < points1.size(); i++) {
        size_t next_i = (i + 1) % points1.size();
        for (size_t j = 0; j < points2.size(); j++) {
            size_t next_j = (j + 1) % points2.size();

            Point intersect;
            if (lineSegmentIntersection(points1[i], points1[next_i],
                                        points2[j], points2[next_j], intersect)) {
                result.addPoint(intersect);
            }
        }
    }

    if (!result.empty()) {
        result.computeConvexHull();
    }
}

bool PolygonCanvas::isPointInsidePolygon(const Point& p, const std::vector<Point>& polygon) {
    if (polygon.size() < 3) return false;

    bool inside = false;
    for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        if (((polygon[i].y > p.y) != (polygon[j].y > p.y)) &&
            (p.x < (polygon[j].x - polygon[i].x) * (p.y - polygon[i].y) /
                           (polygon[j].y - polygon[i].y) + polygon[i].x)) {
            inside = !inside;
        }
    }
    return inside;
}

bool PolygonCanvas::lineSegmentIntersection(const Point& a1, const Point& a2,
                             const Point& b1, const Point& b2, Point& result) {
    Point d1 = a2 - a1;
    Point d2 = b2 - b1;

    double cross = d1.cross(d2);
    if (std::abs(cross) < 1e-9) return false;

    double t = (b1 - a1).cross(d2) / cross;
    double u = (b1 - a1).cross(d1) / cross;

    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        result = a1 + d1 * t;
        return true;
    }
    return false;
}

MainWindow::MainWindow() {
    setWindowTitle("Polygon Operations");
    setFixedSize(800, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    canvas = new PolygonCanvas(this);
    mainLayout->addWidget(canvas);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *nextButton = new QPushButton("Next Polygon", this);
    QPushButton *resetButton = new QPushButton("Reset", this);

    buttonLayout->addWidget(nextButton);
    buttonLayout->addWidget(resetButton);

    QRadioButton *intersectionRadio = new QRadioButton("Intersection", this);
    QRadioButton *unionRadio = new QRadioButton("Union", this);
    QRadioButton *differenceRadio = new QRadioButton("Difference", this);

    intersectionRadio->setChecked(true);

    buttonLayout->addWidget(intersectionRadio);
    buttonLayout->addWidget(unionRadio);
    buttonLayout->addWidget(differenceRadio);

    mainLayout->addLayout(buttonLayout);

    connect(nextButton, &QPushButton::clicked, canvas, &PolygonCanvas::nextPolygon);
    connect(resetButton, &QPushButton::clicked, canvas, &PolygonCanvas::reset);

    connect(intersectionRadio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) canvas->setOperation(PolygonCanvas::INTERSECTION);
    });
    connect(unionRadio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) canvas->setOperation(PolygonCanvas::UNION);
    });
    connect(differenceRadio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) canvas->setOperation(PolygonCanvas::DIFFERENCE);
    });
}
