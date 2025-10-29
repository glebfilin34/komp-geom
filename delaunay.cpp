#include "delaunay.h"

DelaunayWidget::DelaunayWidget(QWidget *parent) : QWidget(parent), onlineMode(false) {
    setMouseTracking(true);
    setMinimumSize(800, 600);
}

void DelaunayWidget::clearPoints() {
    points.clear();
    triangles.clear();
    update();
}

bool DelaunayWidget::isPointInCircumcircle(const QPointF& a, const QPointF& b, const QPointF& c, const QPointF& p) {
    double d = 2 * (a.x() * (b.y() - c.y()) +
                    b.x() * (c.y() - a.y()) +
                    c.x() * (a.y() - b.y()));

    double ux = ((a.x() * a.x() + a.y() * a.y()) * (b.y() - c.y()) +
                 (b.x() * b.x() + b.y() * b.y()) * (c.y() - a.y()) +
                 (c.x() * c.x() + c.y() * c.y()) * (a.y() - b.y())) / d;

    double uy = ((a.x() * a.x() + a.y() * a.y()) * (c.x() - b.x()) +
                 (b.x() * b.x() + b.y() * b.y()) * (a.x() - c.x()) +
                 (c.x() * c.x() + c.y() * c.y()) * (b.x() - a.x())) / d;

    QPointF center(ux, uy);
    double radius = std::sqrt((a.x() - center.x()) * (a.x() - center.x()) +
                              (a.y() - center.y()) * (a.y() - center.y()));

    double distance = std::sqrt((p.x() - center.x()) * (p.x() - center.x()) +
                                (p.y() - center.y()) * (p.y() - center.y()));

    return distance <= radius;
}

void DelaunayWidget::computeDelaunay() {
    if (points.size() < 3) {
        triangles.clear();
        update();
        return;
    }

    triangles.clear();

    double minX = points[0].pos.x(), maxX = points[0].pos.x();
    double minY = points[0].pos.y(), maxY = points[0].pos.y();

    for (const auto& point : points) {
        minX = std::min(minX, point.pos.x());
        maxX = std::max(maxX, point.pos.x());
        minY = std::min(minY, point.pos.y());
        maxY = std::max(maxY, point.pos.y());
    }

    double dx = maxX - minX;
    double dy = maxY - minY;
    double deltaMax = std::max(dx, dy);
    double midX = (minX + maxX) / 2.0;
    double midY = (minY + maxY) / 2.0;

    int p1 = points.size();
    int p2 = p1 + 1;
    int p3 = p1 + 2;

    std::vector<Triangle> triangleList;
    triangleList.emplace_back(p1, p2, p3);

    std::vector<QPointF> tempPoints;
    for (const auto& point : points) {
        tempPoints.push_back(point.pos);
    }
    tempPoints.emplace_back(midX - 20 * deltaMax, midY - deltaMax);
    tempPoints.emplace_back(midX, midY + 20 * deltaMax);
    tempPoints.emplace_back(midX + 20 * deltaMax, midY - deltaMax);

    for (int i = 0; i < points.size(); i++) {
        std::vector<Edge> polygon;
        std::vector<Triangle> toRemove;

        for (const auto& triangle : triangleList) {
            if (isPointInCircumcircle(tempPoints[triangle.p1],
                                      tempPoints[triangle.p2],
                                      tempPoints[triangle.p3],
                                      points[i].pos)) {
                toRemove.push_back(triangle);

                polygon.emplace_back(triangle.p1, triangle.p2);
                polygon.emplace_back(triangle.p2, triangle.p3);
                polygon.emplace_back(triangle.p3, triangle.p1);
            }
        }

        for (const auto& triangle : toRemove) {
            triangleList.erase(
                std::remove(triangleList.begin(), triangleList.end(), triangle),
                triangleList.end()
                );
        }

        std::vector<Edge> uniqueEdges;
        for (const auto& edge : polygon) {
            int count = std::count(polygon.begin(), polygon.end(), edge);
            if (count == 1) {
                uniqueEdges.push_back(edge);
            }
        }

        for (const auto& edge : uniqueEdges) {
            triangleList.emplace_back(edge.p1, edge.p2, i);
        }
    }

    for (auto it = triangleList.begin(); it != triangleList.end();) {
        if (it->p1 >= points.size() || it->p2 >= points.size() || it->p3 >= points.size()) {
            it = triangleList.erase(it);
        } else {
            ++it;
        }
    }

    triangles = triangleList;
    update();
}

void DelaunayWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::white);

    if (!triangles.empty()) {
        painter.setPen(QPen(Qt::blue, 1));
        painter.setBrush(QBrush(QColor(200, 200, 255, 100)));

        for (const auto& triangle : triangles) {
            QPolygonF polygon;
            polygon << points[triangle.p1].pos
                    << points[triangle.p2].pos
                    << points[triangle.p3].pos;
            painter.drawPolygon(polygon);
        }
    }

    painter.setPen(Qt::black);
    painter.setBrush(Qt::red);
    for (const auto& point : points) {
        painter.drawEllipse(point.pos, 4, 4);
    }

    painter.setPen(QPen(Qt::darkBlue, 2));
    for (const auto& triangle : triangles) {
        painter.drawLine(points[triangle.p1].pos, points[triangle.p2].pos);
        painter.drawLine(points[triangle.p2].pos, points[triangle.p3].pos);
        painter.drawLine(points[triangle.p3].pos, points[triangle.p1].pos);
    }

    painter.setPen(Qt::black);
    painter.drawText(10, 20, QString("Точек: %1").arg(points.size()));
    painter.drawText(10, 40, QString("Треугольников: %1").arg(triangles.size()));
    painter.drawText(10, 60, onlineMode ? "Режим: Онлайн" : "Режим: Обычный");
}

void DelaunayWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPointF pos = event->position();

        for (auto& point : points) {
            QPointF diff = point.pos - pos;
            if (diff.x() * diff.x() + diff.y() * diff.y() <= 100) {
                point.isDragging = true;
                if (onlineMode) {
                    computeDelaunay();
                }
                return;
            }
        }

        points.emplace_back(pos);
        if (onlineMode) {
            computeDelaunay();
        }
        update();
    }
}

void DelaunayWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        QPointF pos = event->position();

        for (auto& point : points) {
            if (point.isDragging) {
                point.pos = pos;
                if (onlineMode) {
                    computeDelaunay();
                }
                update();
                return;
            }
        }
    }
}

void DelaunayWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        for (auto& point : points) {
            point.isDragging = false;
        }
        if (!onlineMode) {
            computeDelaunay();
        }
        update();
    }
}

void DelaunayWidget::setOnlineMode(bool enabled) {
    onlineMode = enabled;
    if (onlineMode && points.size() >= 3) {
        computeDelaunay();
    }
    update();
}

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    delaunayWidget = new DelaunayWidget(this);
    mainLayout->addWidget(delaunayWidget);

    QHBoxLayout *controlLayout = new QHBoxLayout();

    QPushButton *clearButton = new QPushButton("Очистить", this);
    QPushButton *computeButton = new QPushButton("Триангуляция Делоне", this);
    QCheckBox *onlineCheckbox = new QCheckBox("Онлайн режим", this);
    QLabel *infoLabel = new QLabel("ЛКМ: добавить точку | Перетащить: двигать точку", this);

    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(computeButton);
    controlLayout->addWidget(onlineCheckbox);
    controlLayout->addWidget(infoLabel);
    controlLayout->addStretch();

    mainLayout->addLayout(controlLayout);

    connect(clearButton, &QPushButton::clicked, delaunayWidget, &DelaunayWidget::clearPoints);
    connect(computeButton, &QPushButton::clicked, delaunayWidget, &DelaunayWidget::computeDelaunay);
    connect(onlineCheckbox, &QCheckBox::toggled, delaunayWidget, &DelaunayWidget::setOnlineMode);

    setWindowTitle("Триангуляция Делоне");
    resize(900, 700);
}
