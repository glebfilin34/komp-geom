#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpression>
#include <cmath>
#include <string>
#include <vector>

class BigNumber {
private:
    std::string value;
    bool negative;

    void removeLeadingZeros() {
        size_t pos = value.find_first_not_of('0');
        if (pos == std::string::npos) {
            value = "0";
            negative = false;
        } else {
            value = value.substr(pos);
        }
    }

public:
    BigNumber() : value("0"), negative(false) {}

    BigNumber(const char* str) {
        std::string s = str;
        negative = false;

        if (!s.empty() && s[0] == '-') {
            negative = true;
            s = s.substr(1);
        }

        size_t dotPos = s.find('.');
        if (dotPos == std::string::npos) {
            value = s;
        } else {
            value = s.substr(0, dotPos) + s.substr(dotPos + 1);
        }

        removeLeadingZeros();
        if (value == "0") negative = false;
    }

    BigNumber(const std::string& str) : BigNumber(str.c_str()) {}

    std::string toString() const {
        if (value == "0") return "0";

        std::string result;
        if (negative) result += "-";

        if (value.length() <= 1) {
            result += value;
            return result;
        }

        result += value.substr(0, 1);
        result += ".";
        result += value.substr(1);

        return result;
    }

    int compare(const BigNumber& other) const {
        if (negative && !other.negative) return -1;
        if (!negative && other.negative) return 1;

        int len1 = value.length();
        int len2 = other.value.length();

        if (len1 > len2) return negative ? -1 : 1;
        if (len1 < len2) return negative ? 1 : -1;

        for (int i = 0; i < len1; i++) {
            if (value[i] > other.value[i]) return negative ? -1 : 1;
            if (value[i] < other.value[i]) return negative ? 1 : -1;
        }

        return 0;
    }

    bool isZero() const {
        return value == "0";
    }

    BigNumber subtract(const BigNumber& other) const {
        if (negative != other.negative) {
            BigNumber temp = other;
            temp.negative = !temp.negative;
            return add(temp);
        }

        std::string a = value;
        std::string b = other.value;

        while (a.length() < b.length()) a = "0" + a;
        while (b.length() < a.length()) b = "0" + b;

        bool resultNegative = false;
        if (a < b) {
            std::swap(a, b);
            resultNegative = true;
        }

        std::string result;
        int borrow = 0;

        for (int i = a.length() - 1; i >= 0; i--) {
            int digitA = a[i] - '0';
            int digitB = b[i] - '0';

            int diff = digitA - digitB - borrow;
            if (diff < 0) {
                diff += 10;
                borrow = 1;
            } else {
                borrow = 0;
            }

            result = char(diff + '0') + result;
        }

        BigNumber res;
        res.value = result;
        res.negative = resultNegative;
        res.removeLeadingZeros();
        return res;
    }

    BigNumber add(const BigNumber& other) const {
        if (negative != other.negative) {
            BigNumber temp = other;
            temp.negative = !temp.negative;
            return subtract(temp);
        }

        std::string a = value;
        std::string b = other.value;

        while (a.length() < b.length()) a = "0" + a;
        while (b.length() < a.length()) b = "0" + b;

        std::string result;
        int carry = 0;

        for (int i = a.length() - 1; i >= 0; i--) {
            int sum = (a[i] - '0') + (b[i] - '0') + carry;
            carry = sum / 10;
            result = char((sum % 10) + '0') + result;
        }

        if (carry > 0) {
            result = char(carry + '0') + result;
        }

        BigNumber res;
        res.value = result;
        res.negative = negative;
        res.removeLeadingZeros();
        return res;
    }

    BigNumber multiply(const BigNumber& other) const {
        if (isZero() || other.isZero()) return BigNumber("0");

        int len1 = value.length();
        int len2 = other.value.length();
        std::vector<int> result(len1 + len2, 0);

        for (int i = len1 - 1; i >= 0; i--) {
            for (int j = len2 - 1; j >= 0; j--) {
                int mul = (value[i] - '0') * (other.value[j] - '0');
                int sum = mul + result[i + j + 1];

                result[i + j + 1] = sum % 10;
                result[i + j] += sum / 10;
            }
        }

        std::string resStr;
        for (int num : result) {
            if (!(resStr.empty() && num == 0)) {
                resStr += char(num + '0');
            }
        }

        if (resStr.empty()) resStr = "0";

        BigNumber res;
        res.value = resStr;
        res.negative = (negative != other.negative);
        return res;
    }
};

class SegmentWidget : public QWidget
{
    Q_OBJECT

public:
    SegmentWidget(QWidget *parent = nullptr) : QWidget(parent), result(0)
    {
        setupUI();
        connect(setSegmentBtn, &QPushButton::clicked, this, &SegmentWidget::setSegmentFromText);
        connect(setPointBtn, &QPushButton::clicked, this, &SegmentWidget::setPointFromText);
        connect(clearBtn, &QPushButton::clicked, this, &SegmentWidget::clearAll);
    }

private slots:
    void setSegmentFromText() {
        QString startText = segmentStartEdit->text().trimmed();
        QString endText = segmentEndEdit->text().trimmed();

        if (!parsePoint(startText, segAx, segAy) || !parsePoint(endText, segBx, segBy)) {
            QMessageBox::warning(this, "Error", "Invalid segment coordinates!");
            return;
        }

        resultLabel->setText("Segment set");
        update();
    }

    void setPointFromText() {
        if (segAx.empty() || segAy.empty() || segBx.empty() || segBy.empty()) {
            QMessageBox::warning(this, "Error", "Please set segment first!");
            return;
        }

        QString pointText = pointEdit->text().trimmed();
        if (!parsePoint(pointText, pointX, pointY)) {
            QMessageBox::warning(this, "Error", "Invalid point coordinates!");
            return;
        }

        calculateExactPosition();
        update();
    }

    void clearAll() {
        segAx = segAy = segBx = segBy = pointX = pointY = "";
        result = 0;
        resultLabel->setText("0");
        segmentStartEdit->clear();
        segmentEndEdit->clear();
        pointEdit->clear();
        update();
    }

private:
    void setupUI() {
        QHBoxLayout *mainLayout = new QHBoxLayout(this);

        QWidget *inputPanel = new QWidget();
        inputPanel->setMaximumWidth(300);
        QVBoxLayout *inputLayout = new QVBoxLayout(inputPanel);

        resultLabel = new QLabel("0");
        resultLabel->setAlignment(Qt::AlignCenter);
        resultLabel->setStyleSheet("QLabel { background-color: #e0e0e0; font-size: 24px; font-weight: bold; border: 2px solid #333; padding: 15px; border-radius: 5px; }");
        resultLabel->setMinimumHeight(50);
        inputLayout->addWidget(resultLabel);

        QGroupBox *inputGroup = new QGroupBox("Input Coordinates");
        QVBoxLayout *groupLayout = new QVBoxLayout(inputGroup);

        QHBoxLayout *segmentLayout = new QHBoxLayout();
        segmentLayout->addWidget(new QLabel("Segment:"));

        segmentStartEdit = new QLineEdit();
        segmentStartEdit->setPlaceholderText("x1,y1");
        segmentLayout->addWidget(segmentStartEdit);

        segmentLayout->addWidget(new QLabel("to"));

        segmentEndEdit = new QLineEdit();
        segmentEndEdit->setPlaceholderText("x2,y2");
        segmentLayout->addWidget(segmentEndEdit);
        groupLayout->addLayout(segmentLayout);

        setSegmentBtn = new QPushButton("Set Segment");
        groupLayout->addWidget(setSegmentBtn);

        QHBoxLayout *pointLayout = new QHBoxLayout();
        pointLayout->addWidget(new QLabel("Point:"));

        pointEdit = new QLineEdit();
        pointEdit->setPlaceholderText("x,y");
        pointLayout->addWidget(pointEdit);
        groupLayout->addLayout(pointLayout);

        setPointBtn = new QPushButton("Set Point");
        groupLayout->addWidget(setPointBtn);

        clearBtn = new QPushButton("Clear All");
        groupLayout->addWidget(clearBtn);

        inputLayout->addWidget(inputGroup);
        inputLayout->addStretch();

        mainLayout->addWidget(inputPanel);
        mainLayout->addStretch();

        inputGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid gray; border-radius: 5px; margin-top: 10px; padding-top: 10px; }");

        setMinimumSize(900, 600);
    }

    bool parsePoint(const QString& text, std::string& x, std::string& y) {
        if (text.isEmpty()) return false;

        QString input = text.trimmed();
        input.remove('(');
        input.remove(')');
        input = input.simplified();
        input.replace(" ", "");

        QStringList parts = input.split(',');
        if (parts.size() != 2) {
            return false;
        }

        QString xStr = parts[0];
        QString yStr = parts[1];

        QRegularExpression numRegex("^-?\\d*\\.?\\d+$");
        if (!numRegex.match(xStr).hasMatch() || !numRegex.match(yStr).hasMatch()) {
            return false;
        }

        x = xStr.toStdString();
        y = yStr.toStdString();
        return true;
    }

    void calculateExactPosition() {
        BigNumber x1(segAx.c_str()), y1(segAy.c_str());
        BigNumber x2(segBx.c_str()), y2(segBy.c_str());
        BigNumber xp(pointX.c_str()), yp(pointY.c_str());

        BigNumber term1 = (x2.subtract(x1)).multiply(yp.subtract(y1));
        BigNumber term2 = (y2.subtract(y1)).multiply(xp.subtract(x1));
        BigNumber cross = term1.subtract(term2);

        if (cross.isZero()) {
            BigNumber minX = (x1.compare(x2) <= 0) ? x1 : x2;
            BigNumber maxX = (x1.compare(x2) <= 0) ? x2 : x1;
            BigNumber minY = (y1.compare(y2) <= 0) ? y1 : y2;
            BigNumber maxY = (y1.compare(y2) <= 0) ? y2 : y1;

            if (xp.compare(minX) >= 0 && xp.compare(maxX) <= 0 &&
                yp.compare(minY) >= 0 && yp.compare(maxY) <= 0) {
                result = 0;
            } else {
                result = 1;
            }
        } else {
            BigNumber zero("0");
            if (cross.compare(zero) > 0) {
                result = 1;
            } else {
                result = -1;
            }
        }

        resultLabel->setText(QString::number(result));
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(0, 0, width(), height(), Qt::white);

        if (!segAx.empty() && !segAy.empty() && !segBx.empty() && !segBy.empty()) {
            try {
                double ax = std::stod(segAx);
                double ay = std::stod(segAy);
                double bx = std::stod(segBx);
                double by = std::stod(segBy);

                QPoint A = scalePoint(QPointF(ax, ay));
                QPoint B = scalePoint(QPointF(bx, by));

                painter.setPen(QPen(Qt::blue, 3));
                painter.drawLine(A, B);

                painter.setPen(QPen(Qt::darkBlue, 10));
                painter.drawPoint(A);
                painter.drawPoint(B);

                painter.setPen(Qt::darkBlue);
                painter.drawText(A + QPoint(15, -15), QString("A(%1,%2)").arg(segAx.c_str()).arg(segAy.c_str()));
                painter.drawText(B + QPoint(15, -15), QString("B(%1,%2)").arg(segBx.c_str()).arg(segBy.c_str()));
            } catch (...) {}
        }

        if (!pointX.empty() && !pointY.empty()) {
            try {
                double px = std::stod(pointX);
                double py = std::stod(pointY);

                QPoint P = scalePoint(QPointF(px, py));

                painter.setPen(QPen(Qt::red, 10));
                painter.drawPoint(P);

                painter.setPen(Qt::red);
                painter.drawText(P + QPoint(15, -15), QString("P(%1,%2)").arg(pointX.c_str()).arg(pointY.c_str()));
            } catch (...) {}
        }
    }

    QPoint scalePoint(const QPointF& p) {
        return QPoint(width()/2 + p.x() * 30, height()/2 - p.y() * 30);
    }

    std::string segAx, segAy, segBx, segBy, pointX, pointY;
    int result;

    QLabel *resultLabel;
    QLineEdit *segmentStartEdit;
    QLineEdit *segmentEndEdit;
    QLineEdit *pointEdit;
    QPushButton *setSegmentBtn;
    QPushButton *setPointBtn;
    QPushButton *clearBtn;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SegmentWidget widget;
    widget.setWindowTitle("Point Position Relative to Segment");
    widget.resize(1000, 600);
    widget.show();

    return app.exec();
}

#include "main.moc"
