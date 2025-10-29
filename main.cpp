#include <QApplication>
#include <QWidget>
#include <QPoint>
#include <QVector>
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr) : QWidget(parent), isInteractiveMode(false), selectedPointIndex(-1), selectedSegmentIndex(-1)
    {
        segments.resize(2);

        calculateButton = new QPushButton("Рассчитать пересечение", this);
        interactiveCheckbox = new QCheckBox("Интерактивный режим", this);
        QLabel *infoLabel = new QLabel("Кликните 4 точки для создания двух отрезков", this);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QHBoxLayout *controlLayout = new QHBoxLayout();

        controlLayout->addWidget(calculateButton);
        controlLayout->addWidget(interactiveCheckbox);
        controlLayout->addStretch();

        mainLayout->addLayout(controlLayout);
        mainLayout->addWidget(infoLabel);
        mainLayout->addStretch();

        connect(calculateButton, &QPushButton::clicked, this, &Widget::calculateIntersection);
        connect(interactiveCheckbox, &QCheckBox::toggled, this, &Widget::toggleInteractiveMode);

        setMinimumSize(800, 600);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPen segmentPen(Qt::black, 2);
        QPen pointPen(Qt::blue, 2);

        for (int i = 0; i < segments.size(); ++i) {
            const Segment &seg = segments[i];
            if (seg.isValid()) {
                painter.setPen(segmentPen);
                painter.drawLine(seg.p1, seg.p2);

                painter.setPen(pointPen);
                painter.setBrush(Qt::white);
                painter.drawEllipse(seg.p1, POINT_RADIUS, POINT_RADIUS);
                painter.drawEllipse(seg.p2, POINT_RADIUS, POINT_RADIUS);

                painter.drawText(seg.p1 + QPoint(-10, -10), QString("P%1-1").arg(i+1));
                painter.drawText(seg.p2 + QPoint(-10, -10), QString("P%1-2").arg(i+1));
            }
        }

        painter.setPen(QPen(Qt::red, 3));
        painter.setBrush(Qt::red);
        for (const QPoint &p : intersectionPoints) {
            painter.drawEllipse(p, POINT_RADIUS, POINT_RADIUS);
        }

        painter.setPen(Qt::darkRed);
        for (const QPoint &p : intersectionPoints) {
            painter.drawText(p + QPoint(15, -5),
                             QString("(%1, %2)").arg(p.x()).arg(p.y()));
        }
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (isInteractiveMode) {
            QPoint clickPos = event->pos();
            QPoint closestPoint = findClosestPoint(clickPos);

            if (!closestPoint.isNull()) {
                for (int i = 0; i < segments.size(); ++i) {
                    if (segments[i].p1 == closestPoint) {
                        selectedPointIndex = 0;
                        selectedSegmentIndex = i;
                        break;
                    } else if (segments[i].p2 == closestPoint) {
                        selectedPointIndex = 1;
                        selectedSegmentIndex = i;
                        break;
                    }
                }
            }
        } else {
            if (event->button() == Qt::LeftButton) {
                for (Segment &seg : segments) {
                    if (!seg.isValid()) {
                        if (seg.p1.isNull()) {
                            seg.p1 = event->pos();
                        } else {
                            seg.p2 = event->pos();
                            break;
                        }
                        break;
                    }
                }
                update();
            }
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (isInteractiveMode && selectedPointIndex != -1 && selectedSegmentIndex != -1) {
            if (selectedPointIndex == 0) {
                segments[selectedSegmentIndex].p1 = event->pos();
            } else {
                segments[selectedSegmentIndex].p2 = event->pos();
            }

            updateIntersection();
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        Q_UNUSED(event);
        if (isInteractiveMode) {
            selectedPointIndex = -1;
            selectedSegmentIndex = -1;
        }
    }

private slots:
    void calculateIntersection()
    {
        if (segments[0].isValid() && segments[1].isValid()) {
            updateIntersection();
            update();
        }
    }

    void toggleInteractiveMode(bool enabled)
    {
        isInteractiveMode = enabled;
        calculateButton->setEnabled(!enabled);

        if (enabled && segments[0].isValid() && segments[1].isValid()) {
            updateIntersection();
            update();
        }
    }

private:
    struct Segment {
        QPoint p1, p2;
        bool isValid() const { return p1 != QPoint() && p2 != QPoint(); }
        void reset() { p1 = p2 = QPoint(); }
    };

    QPoint calculateIntersectionPoint(const Segment &s1, const Segment &s2) const
    {
        if (!doSegmentsIntersect(s1, s2)) {
            return QPoint();
        }

        QPoint p1 = s1.p1;
        QPoint p2 = s1.p2;
        QPoint p3 = s2.p1;
        QPoint p4 = s2.p2;

        int x1 = p1.x(), y1 = p1.y();
        int x2 = p2.x(), y2 = p2.y();
        int x3 = p3.x(), y3 = p3.y();
        int x4 = p4.x(), y4 = p4.y();

        int denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

        if (denom == 0) {
            return QPoint();
        }

        int x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
        int y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;

        return QPoint(x, y);
    }

    bool isPointOnSegment(const QPoint &p, const Segment &s) const
    {
        return (p.x() <= std::max(s.p1.x(), s.p2.x()) &&
                p.x() >= std::min(s.p1.x(), s.p2.x()) &&
                p.y() <= std::max(s.p1.y(), s.p2.y()) &&
                p.y() >= std::min(s.p1.y(), s.p2.y()));
    }

    int orientation(const QPoint &p, const QPoint &q, const QPoint &r) const
    {
        int val = (q.y() - p.y()) * (r.x() - q.x()) - (q.x() - p.x()) * (r.y() - q.y());
        if (val == 0) return 0;
        return (val > 0) ? 1 : 2;
    }

    bool doSegmentsIntersect(const Segment &s1, const Segment &s2) const
    {
        QPoint p1 = s1.p1, q1 = s1.p2;
        QPoint p2 = s2.p1, q2 = s2.p2;

        int o1 = orientation(p1, q1, p2);
        int o2 = orientation(p1, q1, q2);
        int o3 = orientation(p2, q2, p1);
        int o4 = orientation(p2, q2, q1);

        if (o1 != o2 && o3 != o4) {
            return true;
        }

        if (o1 == 0 && isPointOnSegment(p2, s1)) return true;
        if (o2 == 0 && isPointOnSegment(q2, s1)) return true;
        if (o3 == 0 && isPointOnSegment(p1, s2)) return true;
        if (o4 == 0 && isPointOnSegment(q1, s2)) return true;

        return false;
    }

    QPoint findClosestPoint(const QPoint &point) const
    {
        const int MAX_DISTANCE = 15;

        for (const Segment &seg : segments) {
            if (!seg.isValid()) continue;

            if (QLineF(point, seg.p1).length() <= MAX_DISTANCE) {
                return seg.p1;
            }
            if (QLineF(point, seg.p2).length() <= MAX_DISTANCE) {
                return seg.p2;
            }
        }

        return QPoint();
    }

    void updateIntersection()
    {
        intersectionPoints.clear();

        if (segments[0].isValid() && segments[1].isValid()) {
            QPoint intersection = calculateIntersectionPoint(segments[0], segments[1]);
            if (!intersection.isNull()) {
                intersectionPoints.append(intersection);
            }
        }
    }

    QVector<Segment> segments;
    QVector<QPoint> intersectionPoints;

    QPushButton *calculateButton;
    QCheckBox *interactiveCheckbox;

    bool isInteractiveMode;
    int selectedPointIndex;
    int selectedSegmentIndex;

    static const int POINT_RADIUS = 6;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.setWindowTitle("Пересечение отрезков");
    w.show();

    return a.exec();
}

#include "main.moc"
