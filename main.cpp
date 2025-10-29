#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QLabel>
#include <cmath>

class SegmentWidget : public QWidget
{
    Q_OBJECT

public:
    SegmentWidget(QWidget *parent = nullptr) : QWidget(parent),
        segmentComplete(false), pointComplete(false), result(0)
    {
        setMouseTracking(true);
        resultLabel = new QLabel("0", this);
        resultLabel->setAlignment(Qt::AlignCenter);
        resultLabel->setStyleSheet("QLabel { background-color: white; font-size: 24px; border: 1px solid black; }");
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            if (!segmentComplete) {
                if (segmentPoints.size() < 2) {
                    segmentPoints.append(event->pos());
                    if (segmentPoints.size() == 2) {
                        segmentComplete = true;
                    }
                }
            } else if (!pointComplete) {
                point = event->pos();
                pointComplete = true;
                calculatePosition();
            } else {
                segmentPoints.clear();
                segmentComplete = false;
                pointComplete = false;
                result = 0;
                resultLabel->setText("0");
            }
            update();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if ((segmentPoints.size() == 1 && !segmentComplete) ||
            (segmentComplete && !pointComplete)) {
            tempPoint = event->pos();
            update();
        }
    }

    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        if (segmentPoints.size() >= 1) {
            painter.setPen(QPen(Qt::blue, 2));
            QPoint endPoint = (segmentPoints.size() == 2) ? segmentPoints[1] : tempPoint;
            painter.drawLine(segmentPoints[0], endPoint);

            painter.setPen(QPen(Qt::darkBlue, 6));
            for (const QPoint& p : segmentPoints) {
                painter.drawPoint(p);
            }
        }

        if (pointComplete) {
            painter.setPen(QPen(Qt::red, 4));
            painter.drawPoint(point);
        } else if (segmentComplete) {
            painter.setPen(QPen(Qt::red, 2));
            painter.drawEllipse(tempPoint, 3, 3);
        }
    }

    void resizeEvent(QResizeEvent *) override {
        resultLabel->setGeometry(width()/2 - 25, 10, 50, 40);
    }

private:
    void calculatePosition() {
        if (segmentPoints.size() != 2) return;

        QPoint A = segmentPoints[0];
        QPoint B = segmentPoints[1];
        QPoint P = point;

        int cross = (B.x() - A.x()) * (P.y() - A.y()) -
                    (B.y() - A.y()) * (P.x() - A.x());

        int ABx = B.x() - A.x();
        int ABy = B.y() - A.y();
        int APx = P.x() - A.x();
        int APy = P.y() - A.y();

        double dotAB = ABx*ABx + ABy*ABy;
        double dotAP = ABx*APx + ABy*APy;
        double t = (dotAB != 0) ? dotAP / dotAB : 0;

        double distance = abs(cross) / sqrt(ABx*ABx + ABy*ABy);

        if (distance <= 5 && t >= -0.1 && t <= 1.1) {
            result = 0;
        } else if (cross > 0) {
            result = 1;
        } else {
            result = -1;
        }

        resultLabel->setText(QString::number(result));
        update();
    }

    QVector<QPoint> segmentPoints;
    QPoint point, tempPoint;
    bool segmentComplete, pointComplete;
    int result;
    QLabel *resultLabel;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SegmentWidget widget;
    widget.setWindowTitle("Position Relative to Segment");
    widget.resize(800, 600);
    widget.show();

    return app.exec();
}

#include "main.moc"
