#include "convex_hull.h"

ConvexHullWidget::ConvexHullWidget(QWidget *parent) : QWidget(parent), onlineMode(false) {
    setMouseTracking(true);
    setMinimumSize(800, 600);
}

void ConvexHullWidget::clearPoints() {
    points.clear();
    convexHull.clear();
    update();
}

int ConvexHullWidget::orientation(const QPointF& p, const QPointF& q, const QPointF& r) {
    double val = (q.y() - p.y()) * (r.x() - q.x()) - (q.x() - p.x()) * (r.y() - q.y());
    if (val == 0) return 0;
    return (val > 0) ? 1 : 2;
}

void ConvexHullWidget::computeConvexHull() {
    if (points.size() < 3) {
        convexHull.clear();
        update();
        return;
    }

    convexHull.clear();

    int startIndex = 0;
    for (int i = 1; i < points.size(); i++) {
        if (points[i].pos.y() < points[startIndex].pos.y() ||
            (points[i].pos.y() == points[startIndex].pos.y() &&
             points[i].pos.x() < points[startIndex].pos.x())) {
            startIndex = i;
        }
    }

    std::vector<int> hullIndices;
    int current = startIndex;

    do {
        hullIndices.push_back(current);
        int next = (current + 1) % points.size();

        for (int i = 0; i < points.size(); i++) {
            if (orientation(points[current].pos, points[i].pos, points[next].pos) == 2) {
                next = i;
            }
        }

        current = next;
    } while (current != startIndex);

    for (int idx : hullIndices) {
        convexHull.push_back(points[idx].pos);
    }

    update();
}

void ConvexHullWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);

    painter.setPen(Qt::black);
    painter.setBrush(Qt::blue);
    for (const auto& point : points) {
        painter.drawEllipse(point.pos, 5, 5);
    }

    if (convexHull.size() >= 3) {
        painter.setPen(QPen(Qt::red, 2));
        painter.setBrush(QBrush(QColor(255, 0, 0, 50)));
        QPolygonF polygon;
        for (const auto& point : convexHull) {
            polygon << point;
        }
        painter.drawPolygon(polygon);
    }

    painter.setPen(Qt::black);
    painter.drawText(10, 20, QString("Точек: %1").arg(points.size()));
    painter.drawText(10, 40, QString("Вершин оболочки: %1").arg(convexHull.size()));
    painter.drawText(10, 60, onlineMode ? "Режим: Онлайн" : "Режим: Обычный");
}

void ConvexHullWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPointF pos = event->position();
        for (auto& point : points) {
            QPointF diff = point.pos - pos;
            if (diff.x() * diff.x() + diff.y() * diff.y() <= 100) {
                point.isDragging = true;
                if (onlineMode) computeConvexHull();
                return;
            }
        }
        points.emplace_back(pos);
        if (onlineMode) computeConvexHull();
        update();
    }
}

void ConvexHullWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        QPointF pos = event->position();
        for (auto& point : points) {
            if (point.isDragging) {
                point.pos = pos;
                if (onlineMode) computeConvexHull();
                update();
                return;
            }
        }
    }
}

void ConvexHullWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        for (auto& point : points) {
            point.isDragging = false;
        }
        if (!onlineMode) computeConvexHull();
        update();
    }
}

void ConvexHullWidget::setOnlineMode(bool enabled) {
    onlineMode = enabled;
    if (onlineMode) computeConvexHull();
    update();
}

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    convexHullWidget = new ConvexHullWidget(this);
    mainLayout->addWidget(convexHullWidget);

    QHBoxLayout *controlLayout = new QHBoxLayout();
    QPushButton *clearButton = new QPushButton("Очистить", this);
    QPushButton *computeButton = new QPushButton("Построить оболочку", this);
    QCheckBox *onlineCheckbox = new QCheckBox("Онлайн режим", this);
    QLabel *infoLabel = new QLabel("ЛКМ: добавить точку | Перетащить: двигать точку", this);

    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(computeButton);
    controlLayout->addWidget(onlineCheckbox);
    controlLayout->addWidget(infoLabel);
    controlLayout->addStretch();
    mainLayout->addLayout(controlLayout);

    connect(clearButton, &QPushButton::clicked, convexHullWidget, &ConvexHullWidget::clearPoints);
    connect(computeButton, &QPushButton::clicked, convexHullWidget, &ConvexHullWidget::computeConvexHull);
    connect(onlineCheckbox, &QCheckBox::toggled, convexHullWidget, &ConvexHullWidget::setOnlineMode);

    setWindowTitle("Выпуклая оболочка");
    resize(900, 700);
}
