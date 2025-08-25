#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QTextEdit>
#include <QPainter>
#include <QWidget>
#include <QSizePolicy>
#include <QComboBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QFrame>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainterPath>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QLineEdit>
#include <QInputDialog>
#include <QSplitter>
#include <QTabWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QColorDialog>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <numeric>
#include <map>
#include <random>
#include <regex>
#include <locale>
#include <codecvt>
#include <QTextCodec>
#include <QTextStream>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QScrollArea>
#include <QTextBrowser>
#include <QJsonParseError>
#include <QScrollBar>
#include <QDateTime>
#include <QFileInfo>
#include "deepseek_dialog.h"
#include "plotwidget_new.h"
#include "mainwindow.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declaration for simplified AI assistant
class SimpleConfigDialog;
    {
        xData = x;
        yData = y;
        calculateStatistics();
        update();
    }
    
    void setData(const std::vector<double>& data)
    {
        yData = data;
        xData.clear();
        for (size_t i = 0; i < data.size(); ++i) {
            xData.push_back(static_cast<double>(i + 1));
        }
        calculateStatistics();
        update();
    }
    
    void setData(const std::vector<std::vector<double>>& allData, int xCol, int yCol)
    {
        xData.clear();
        yData.clear();
        
        if (allData.empty() || xCol < 0 || yCol < 0) return;
        
        for (const auto& row : allData) {
            if (xCol < (int)row.size() && yCol < (int)row.size()) {
                xData.push_back(row[xCol]);
                yData.push_back(row[yCol]);
            }
        }
        calculateStatistics();
        update();
    }
    
    void setLabels(const std::vector<QString>& labels)
    {
        dataLabels = labels;
        update();
    }

    void clearData()
    {
        xData.clear();
        yData.clear();
        dataLabels.clear();
        statistics.clear();
        update();
    }
    
    void setChartType(ChartType type)
    {
        chartType = type;
        update();
    }
    
    void setTitle(const QString& title)
    {
        chartTitle = title;
        update();
    }
    
    void setAxisLabels(const QString& xLabel, const QString& yLabel)
    {
        xAxisLabel = xLabel;
        yAxisLabel = yLabel;
        update();
    }
    
    // 新增的配置方法
    void setColors(const QColor& bg, const QColor& grid, const QColor& axis, const QColor& text)
    {
        backgroundColor = bg;
        gridColor = grid;
        axisColor = axis;
        textColor = text;
        update();
    }
    
    void setGridVisible(bool visible)
    {
        showGrid = visible;
        update();
    }
    
    void setLineWidth(int width)
    {
        lineWidth = width;
        update();
    }
    
    void setPointSize(int size)
    {
        pointSize = size;
        update();
    }
    
    void setFontSizes(double title, double label, double axis)
    {
        titleFontSize = title;
        labelFontSize = label;
        axisFontSize = axis;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Apply zoom and pan transformations
        painter.translate(panOffset);
        painter.scale(zoomFactor, zoomFactor);

        // 设置美化的背景
        QLinearGradient bgGradient(0, 0, 0, height());
        bgGradient.setColorAt(0, backgroundColor);
        bgGradient.setColorAt(1, backgroundColor.lighter(105));
        painter.fillRect(rect(), bgGradient);

        if (xData.empty() || yData.empty()) {
            drawNoDataMessage(painter);
            return;
        }

        // 根据图表类型绘制
        switch (chartType) {
            case ChartType::Line:
                drawLineChart(painter);
                break;
            case ChartType::Bar:
                drawBarChart(painter);
                break;
            case ChartType::Pie:
                drawPieChart(painter);
                break;
            case ChartType::Scatter:
                drawScatterChart(painter);
                break;
            case ChartType::Histogram:
                drawHistogram(painter);
                break;
            case ChartType::BoxPlot:
                drawBoxPlot(painter);
                break;
            case ChartType::ViolinPlot:
                drawViolinPlot(painter);
                break;
            case ChartType::DensityPlot:
                drawDensityPlot(painter);
                break;
            case ChartType::AreaChart:
                drawAreaChart(painter);
                break;
        }
    }

private:
    void calculateStatistics()
    {
        if (yData.empty()) return;
        
        statistics.clear();
        
        // 计算基本统计信息
        double sum = std::accumulate(yData.begin(), yData.end(), 0.0);
        double mean = sum / yData.size();
        
        double variance = 0.0;
        for (double value : yData) {
            variance += (value - mean) * (value - mean);
        }
        variance /= yData.size();
        double stddev = std::sqrt(variance);
        
        auto minmax = std::minmax_element(yData.begin(), yData.end());
        
        statistics["数量"] = QString::number(yData.size());
        statistics["总和"] = QString::number(sum, 'f', 2);
        statistics["平均值"] = QString::number(mean, 'f', 2);
        statistics["标准差"] = QString::number(stddev, 'f', 2);
        statistics["最小值"] = QString::number(*minmax.first, 'f', 2);
        statistics["最大值"] = QString::number(*minmax.second, 'f', 2);
        
        // 计算中位数
        std::vector<double> sortedData = yData;
        std::sort(sortedData.begin(), sortedData.end());
        double median;
        if (sortedData.size() % 2 == 0) {
            median = (sortedData[sortedData.size()/2 - 1] + sortedData[sortedData.size()/2]) / 2.0;
        } else {
            median = sortedData[sortedData.size()/2];
        }
        statistics["中位数"] = QString::number(median, 'f', 2);
    }
    
    void drawNoDataMessage(QPainter& painter)
    {
        painter.setPen(textColor);
        QFont font("Microsoft YaHei", static_cast<int>(labelFontSize + 4));
        font.setPointSizeF((labelFontSize + 4) * devicePixelRatio());
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "暂无数据显示\n\n请加载 TXT 文件查看图表");
    }
    
    void drawTitle(QPainter& painter, const QRect& plotRect)
    {
        if (!chartTitle.isEmpty()) {
            painter.setPen(textColor);
            QFont titleFont("Microsoft YaHei", static_cast<int>(titleFontSize), QFont::Bold);
        titleFont.setPointSizeF(titleFontSize * devicePixelRatio());
        painter.setFont(titleFont);
            QRect titleRect = QRect(plotRect.left(), 10, plotRect.width(), 30);
            painter.drawText(titleRect, Qt::AlignCenter, chartTitle);
        }
    }
    
    void drawAxes(QPainter& painter, const QRect& plotRect)
    {
        if (chartType == ChartType::Pie) return;
        
        painter.setPen(QPen(axisColor, 2));
        painter.drawLine(plotRect.bottomLeft(), plotRect.bottomRight()); // X轴
        painter.drawLine(plotRect.bottomLeft(), plotRect.topLeft());     // Y轴
        
        // 绘制轴标签
        painter.setPen(textColor);
        QFont labelFont("Microsoft YaHei", static_cast<int>(labelFontSize));
        labelFont.setPointSizeF(labelFontSize * devicePixelRatio());
        painter.setFont(labelFont);
        if (!xAxisLabel.isEmpty()) {
            painter.drawText(plotRect.center().x() - 50, height() - 10, xAxisLabel);
        }
        if (!yAxisLabel.isEmpty()) {
            painter.save();
            painter.translate(15, plotRect.center().y());
            painter.rotate(-90);
            painter.drawText(-50, 0, yAxisLabel);
            painter.restore();
        }
    }
    
    void drawGrid(QPainter& painter, const QRect& plotRect)
    {
        if (chartType == ChartType::Pie || !showGrid) return;
        
        painter.setPen(QPen(gridColor, 1, Qt::DotLine));
        for (int i = 1; i < 10; ++i) {
            int x = plotRect.left() + (plotRect.width() * i) / 10;
            painter.drawLine(x, plotRect.top(), x, plotRect.bottom());
            
            int y = plotRect.top() + (plotRect.height() * i) / 10;
            painter.drawLine(plotRect.left(), y, plotRect.right(), y);
        }
    }
    
    void drawLineChart(QPainter& painter)
    {
        int margin = 80;
        QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
        
        drawTitle(painter, plotRect);
        drawGrid(painter, plotRect);
        drawAxes(painter, plotRect);
        
        if (xData.size() < 2) return;
        
        auto xMinMax = std::minmax_element(xData.begin(), xData.end());
        auto yMinMax = std::minmax_element(yData.begin(), yData.end());
        
        double xMin = *xMinMax.first;
        double xMax = *xMinMax.second;
        double yMin = *yMinMax.first;
        double yMax = *yMinMax.second;
        
        double xRange = xMax - xMin;
        double yRange = yMax - yMin;
        if (xRange == 0) xRange = 1;
        if (yRange == 0) yRange = 1;
        
        xMin -= xRange * 0.05;
        xMax += xRange * 0.05;
        yMin -= yRange * 0.05;
        yMax += yRange * 0.05;
        
        // 绘制渐变填充区域
        if (xData.size() > 2) {
            QPolygonF polygon;
            for (size_t i = 0; i < xData.size(); ++i) {
                int x = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
                int y = plotRect.bottom() - (int)((yData[i] - yMin) / (yMax - yMin) * plotRect.height());
                polygon << QPointF(x, y);
            }
            polygon << QPointF(plotRect.right(), plotRect.bottom());
            polygon << QPointF(plotRect.left(), plotRect.bottom());
            
            QLinearGradient gradient(0, plotRect.top(), 0, plotRect.bottom());
            gradient.setColorAt(0, colors[0].lighter(150));
            gradient.setColorAt(0.3, colors[0].lighter(120));
            gradient.setColorAt(1, colors[0].lighter(180));
            painter.setBrush(gradient);
            painter.setPen(Qt::NoPen);
            painter.drawPolygon(polygon);
        }
        
        // 绘制连线
        painter.setPen(QPen(colors[0], 3));
        painter.setBrush(Qt::NoBrush);
        for (size_t i = 0; i < xData.size() - 1; ++i) {
            int x1 = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
            int y1 = plotRect.bottom() - (int)((yData[i] - yMin) / (yMax - yMin) * plotRect.height());
            int x2 = plotRect.left() + (int)((xData[i+1] - xMin) / (xMax - xMin) * plotRect.width());
            int y2 = plotRect.bottom() - (int)((yData[i+1] - yMin) / (yMax - yMin) * plotRect.height());
            
            painter.drawLine(x1, y1, x2, y2);
        }
        
        // 绘制数据点
        for (size_t i = 0; i < xData.size(); ++i) {
            int x = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
            int y = plotRect.bottom() - (int)((yData[i] - yMin) / (yMax - yMin) * plotRect.height());
            
            painter.setPen(QPen(colors[0].darker(), 2));
            painter.setBrush(colors[0]);
            painter.drawEllipse(x - pointSize, y - pointSize, pointSize * 2, pointSize * 2);
            
            painter.setPen(Qt::white);
            painter.setBrush(Qt::white);
            painter.drawEllipse(x - pointSize/2, y - pointSize/2, pointSize, pointSize);
        }
        
        drawAxisLabels(painter, plotRect, xMin, xMax, yMin, yMax);
    }
    
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            isDragging = true;
            lastPanPoint = event->pos();
            setCursor(Qt::ClosedHandCursor);
        }
    }
    
    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (isDragging) {
            QPoint delta = event->pos() - lastPanPoint;
            panOffset += delta;
            lastPanPoint = event->pos();
            update();
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
    
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            isDragging = false;
            setCursor(Qt::ArrowCursor);
        }
    }
    
    void wheelEvent(QWheelEvent *event) override
    {
        const double zoomInFactor = 1.15;
        const double zoomOutFactor = 1.0 / zoomInFactor;
        
        // Save the scene position that will be under the mouse after scaling
        QPointF oldPos = event->position();
        QPointF scenePos = (oldPos - panOffset) / zoomFactor;
        
        // Scale
        if (event->angleDelta().y() > 0) {
            zoomFactor *= zoomInFactor;
        } else {
            zoomFactor *= zoomOutFactor;
        }
        
        // Limit zoom range
        zoomFactor = qBound(0.1, zoomFactor, 10.0);
        
        // Calculate new pan offset to keep mouse position fixed
        QPointF newPos = scenePos * zoomFactor + panOffset;
        panOffset += oldPos - newPos;
        
        update();
        event->accept();
    }
    
    void keyPressEvent(QKeyEvent *event) override
    {
        switch (event->key()) {
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            zoomFactor *= 1.2;
            zoomFactor = qMin(zoomFactor, 10.0);
            update();
            break;
        case Qt::Key_Minus:
            zoomFactor /= 1.2;
            zoomFactor = qMax(zoomFactor, 0.1);
            update();
            break;
        case Qt::Key_0:
            // Reset zoom and pan
            zoomFactor = 1.0;
            panOffset = QPointF(0, 0);
            update();
            break;
        case Qt::Key_Home:
            // Center the view
            panOffset = QPointF(0, 0);
            update();
            break;
        default:
            QWidget::keyPressEvent(event);
        }
    }
    
    void drawBarChart(QPainter& painter)
    {
        int margin = 80;
        QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
        
        drawTitle(painter, plotRect);
        drawGrid(painter, plotRect);
        drawAxes(painter, plotRect);
        
        if (yData.empty()) return;
        
        double yMin = *std::min_element(yData.begin(), yData.end());
        double yMax = *std::max_element(yData.begin(), yData.end());
        
        if (yMin > 0) yMin = 0;
        
        double yRange = yMax - yMin;
        if (yRange == 0) yRange = 1;
        
        int barWidth = std::max(5, plotRect.width() / (int)(yData.size() * 2));
        int spacing = std::max(2, barWidth / 4);
        
        for (size_t i = 0; i < yData.size(); ++i) {
            int x = plotRect.left() + (int)(i * (barWidth + spacing)) + spacing;
            int barHeight = (int)((yData[i] - yMin) / yRange * plotRect.height());
            int y = plotRect.bottom() - barHeight;
            
            QRect barRect(x, y, barWidth, barHeight);
            
            // 多层渐变填充 + 高光效果
            QLinearGradient gradient(x, y, x, y + barHeight);
            QColor color = colors[i % colors.size()];
            gradient.setColorAt(0, color.lighter(140));
            gradient.setColorAt(0.3, color.lighter(110));
            gradient.setColorAt(0.7, color);
            gradient.setColorAt(1, color.darker(110));
            
            painter.setBrush(gradient);
            painter.setPen(QPen(color.darker(120), 2));
            painter.drawRoundedRect(barRect, 4, 4);  // 圆角矩形
            
            // 添加高光效果
            if (barHeight > 10) {
                QRect highlightRect(x + 2, y + 2, barWidth - 4, barHeight / 4);
                QLinearGradient highlightGradient(x, y, x, y + barHeight / 4);
                highlightGradient.setColorAt(0, QColor(255, 255, 255, 100));
                highlightGradient.setColorAt(1, QColor(255, 255, 255, 20));
                painter.setBrush(highlightGradient);
                painter.setPen(Qt::NoPen);
                painter.drawRoundedRect(highlightRect, 2, 2);
            }
            
            // 美化的数值标签
            painter.setPen(textColor);
            QFont valueFont("Microsoft YaHei", static_cast<int>(axisFontSize + 1), QFont::Bold);
            valueFont.setPointSizeF((axisFontSize + 1) * devicePixelRatio());
            painter.setFont(valueFont);
            QString valueText = QString::number(yData[i], 'f', 1);
            QRect textRect(x, y - 25, barWidth, 20);
            
            // 添加文字阴影效果
            painter.setPen(QColor(0, 0, 0, 50));
            painter.drawText(textRect.adjusted(1, 1, 1, 1), Qt::AlignCenter, valueText);
            painter.setPen(textColor);
            painter.drawText(textRect, Qt::AlignCenter, valueText);
            
            // X轴标签
            if (i < dataLabels.size()) {
                QFont labelFont("Microsoft YaHei", static_cast<int>(axisFontSize));
                labelFont.setPointSizeF(axisFontSize * devicePixelRatio());
                painter.setFont(labelFont);
                QRect labelRect(x, plotRect.bottom() + 5, barWidth, 20);
                painter.drawText(labelRect, Qt::AlignCenter, dataLabels[i]);
            }
        }
    }
    
    void drawPieChart(QPainter& painter)
    {
        if (yData.empty()) return;
        
        drawTitle(painter, rect());
        
        int size = std::min(width(), height()) - 200;
        QRect pieRect((width() - size) / 2, (height() - size) / 2, size, size);
        
        double total = std::accumulate(yData.begin(), yData.end(), 0.0);
        if (total <= 0) return;
        
        int startAngle = 0;
        
        for (size_t i = 0; i < yData.size(); ++i) {
            int spanAngle = (int)(yData[i] / total * 5760); // 5760 = 360 * 16
            
            QColor color = colors[i % colors.size()];
            
            // 创建更复杂的径向渐变
            QRadialGradient gradient(pieRect.center(), size / 2);
            gradient.setColorAt(0, color.lighter(150));
            gradient.setColorAt(0.6, color);
            gradient.setColorAt(1, color.darker(120));
            
            painter.setBrush(gradient);
            painter.setPen(QPen(Qt::white, 3));
            painter.drawPie(pieRect, startAngle, spanAngle);
            
            // 添加内部高光效果
            QRadialGradient innerGradient(pieRect.center(), size / 6);
            innerGradient.setColorAt(0, QColor(255, 255, 255, 80));
            innerGradient.setColorAt(1, QColor(255, 255, 255, 0));
            
            painter.setBrush(innerGradient);
            painter.setPen(Qt::NoPen);
            painter.drawPie(pieRect, startAngle, spanAngle);
            
            // 绘制连接线和标签
            double midAngle = (startAngle + spanAngle / 2.0) / 16.0 * M_PI / 180.0;
            int innerRadius = size / 2 - 10;
            int outerRadius = size / 2 + 40;
            
            int innerX = pieRect.center().x() + (int)(innerRadius * cos(midAngle));
            int innerY = pieRect.center().y() - (int)(innerRadius * sin(midAngle));
            int outerX = pieRect.center().x() + (int)(outerRadius * cos(midAngle));
            int outerY = pieRect.center().y() - (int)(outerRadius * sin(midAngle));
            
            // 绘制指示线
            painter.setPen(QPen(color.darker(), 2));
            painter.drawLine(innerX, innerY, outerX, outerY);
            
            // 水平延伸线
            int extendX = outerX + (cos(midAngle) > 0 ? 20 : -20);
            painter.drawLine(outerX, outerY, extendX, outerY);
            
            // 美化标签
            painter.setPen(textColor);
            QFont valueFont("Microsoft YaHei", static_cast<int>(axisFontSize + 1), QFont::Bold);
            valueFont.setPointSizeF((axisFontSize + 1) * devicePixelRatio());
            painter.setFont(valueFont);
            
            QString labelText;
            if (i < dataLabels.size()) {
                labelText = QString("%1 (%2%)")
                           .arg(dataLabels[i])
                           .arg(yData[i] / total * 100, 0, 'f', 1);
            } else {
                labelText = QString("分段 %1 (%2%)")
                           .arg(i + 1)
                           .arg(yData[i] / total * 100, 0, 'f', 1);
            }
            
            QRect textRect;
            if (cos(midAngle) > 0) {
                textRect = QRect(extendX + 5, outerY - 10, 100, 20);
            } else {
                textRect = QRect(extendX - 105, outerY - 10, 100, 20);
            }
            
            // 添加文字背景
            painter.setBrush(QColor(255, 255, 255, 220));
            painter.setPen(QPen(color, 1));
            painter.drawRoundedRect(textRect.adjusted(-3, -2, 3, 2), 3, 3);
            
            painter.setPen(textColor);
            painter.drawText(textRect, Qt::AlignCenter, labelText);
            
            startAngle += spanAngle;
        }
    }
    
    void drawScatterChart(QPainter& painter)
    {
        int margin = 80;
        QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
        
        drawTitle(painter, plotRect);
        drawGrid(painter, plotRect);
        drawAxes(painter, plotRect);
        
        if (xData.empty() || yData.empty()) return;
        
        auto xMinMax = std::minmax_element(xData.begin(), xData.end());
        auto yMinMax = std::minmax_element(yData.begin(), yData.end());
        
        double xMin = *xMinMax.first;
        double xMax = *xMinMax.second;
        double yMin = *yMinMax.first;
        double yMax = *yMinMax.second;
        
        double xRange = xMax - xMin;
        double yRange = yMax - yMin;
        if (xRange == 0) xRange = 1;
        if (yRange == 0) yRange = 1;
        
        xMin -= xRange * 0.05;
        xMax += xRange * 0.05;
        yMin -= yRange * 0.05;
        yMax += yRange * 0.05;
        
        for (size_t i = 0; i < xData.size(); ++i) {
            int x = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
            int y = plotRect.bottom() - (int)((yData[i] - yMin) / (yMax - yMin) * plotRect.height());
            
            QColor color = colors[i % colors.size()];
            
            // 绘制带阴影的散点
            painter.setPen(QPen(color.darker(), 2));
            painter.setBrush(QBrush(color));
            painter.drawEllipse(x - 6, y - 6, 12, 12);
            
            painter.setPen(QPen(color.lighter(), 1));
            painter.setBrush(QBrush(color.lighter()));
            painter.drawEllipse(x - 3, y - 3, 6, 6);
        }
        
        drawAxisLabels(painter, plotRect, xMin, xMax, yMin, yMax);
    }
    
    void drawHistogram(QPainter& painter)
    {
        if (yData.empty()) return;
        
        // 计算直方图数据
        int bins = std::min(20, (int)std::sqrt(yData.size()));
        if (bins < 5) bins = 5;
        
        double minVal = *std::min_element(yData.begin(), yData.end());
        double maxVal = *std::max_element(yData.begin(), yData.end());
        double binWidth = (maxVal - minVal) / bins;
        
        std::vector<int> histogram(bins, 0);
        for (double value : yData) {
            int bin = std::min(bins - 1, (int)((value - minVal) / binWidth));
            histogram[bin]++;
        }
        
        int margin = 80;
        QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
        
        drawTitle(painter, plotRect);
        drawGrid(painter, plotRect);
        drawAxes(painter, plotRect);
        
        int maxCount = *std::max_element(histogram.begin(), histogram.end());
        if (maxCount == 0) return;
        
        int barWidth = plotRect.width() / bins;
        
        for (int i = 0; i < bins; ++i) {
            int x = plotRect.left() + i * barWidth;
            int barHeight = (int)((double)histogram[i] / maxCount * plotRect.height());
            int y = plotRect.bottom() - barHeight;
            
            QRect barRect(x, y, barWidth - 1, barHeight);
            
            QLinearGradient gradient(x, y, x, y + barHeight);
            QColor color = colors[0];
            gradient.setColorAt(0, color.lighter(120));
            gradient.setColorAt(1, color);
            
            painter.setBrush(gradient);
            painter.setPen(QPen(color.darker(), 1));
            painter.drawRect(barRect);
            
            // 频次标签
            if (histogram[i] > 0) {
                painter.setPen(textColor);
                QFont labelFont("Microsoft YaHei", static_cast<int>(axisFontSize));
                labelFont.setPointSizeF(axisFontSize * devicePixelRatio());
                painter.setFont(labelFont);
                QRect textRect(x, y - 15, barWidth, 12);
                painter.drawText(textRect, Qt::AlignCenter, QString::number(histogram[i]));
            }
        }
    }
    
    void drawAxisLabels(QPainter& painter, const QRect& plotRect, double xMin, double xMax, double yMin, double yMax)
    {
        painter.setPen(textColor);
        QFont axisFont("Microsoft YaHei", static_cast<int>(axisFontSize));
        axisFont.setPointSizeF(axisFontSize * devicePixelRatio());
        painter.setFont(axisFont);
        
        // Y轴标签
        for (int i = 0; i <= 5; ++i) {
            double value = yMin + (yMax - yMin) * i / 5.0;
            int y = plotRect.bottom() - (int)(plotRect.height() * i / 5.0);
            painter.drawText(plotRect.left() - 50, y + 5, QString::number(value, 'f', 1));
        }
        
        // X轴标签
        for (int i = 0; i <= 5; ++i) {
            double value = xMin + (xMax - xMin) * i / 5.0;
            int x = plotRect.left() + (int)(plotRect.width() * i / 5.0);
            painter.drawText(x - 20, plotRect.bottom() + 20, QString::number(value, 'f', 1));
        }
    }
    
    void drawBoxPlot(QPainter& painter)
    {
        if (yData.empty()) return;
        
        int margin = 80;
        QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
        
        drawTitle(painter, plotRect);
        drawAxes(painter, plotRect);
        
        // 计算四分位数
        std::vector<double> sortedData = yData;
        std::sort(sortedData.begin(), sortedData.end());
        
        size_t n = sortedData.size();
        double q1 = sortedData[n / 4];
        double median = (n % 2 == 0) ? (sortedData[n/2-1] + sortedData[n/2]) / 2.0 : sortedData[n/2];
        double q3 = sortedData[3 * n / 4];
        double iqr = q3 - q1;
        double lowerFence = q1 - 1.5 * iqr;
        double upperFence = q3 + 1.5 * iqr;
        
        // 找到实际的边界值
        double actualLower = *std::find_if(sortedData.begin(), sortedData.end(), 
                                         [lowerFence](double x) { return x >= lowerFence; });
        double actualUpper = *std::find_if(sortedData.rbegin(), sortedData.rend(), 
                                         [upperFence](double x) { return x <= upperFence; }).base() - 1;
        
        double yMin = std::min(actualLower, *std::min_element(sortedData.begin(), sortedData.end()));
        double yMax = std::max(actualUpper, *std::max_element(sortedData.begin(), sortedData.end()));
        double yRange = yMax - yMin;
        if (yRange == 0) yRange = 1;
        
        // 添加一些边距
        yMin -= yRange * 0.1;
        yMax += yRange * 0.1;
        yRange = yMax - yMin;
        
        int boxWidth = plotRect.width() / 3;
        int boxLeft = plotRect.left() + plotRect.width() / 2 - boxWidth / 2;
        
        auto scaleY = [&](double value) -> int {
            return plotRect.bottom() - (int)((value - yMin) / yRange * plotRect.height());
        };
        
        // 绘制箱线图
        int q1Y = scaleY(q1);
        int medianY = scaleY(median);
        int q3Y = scaleY(q3);
        int lowerY = scaleY(actualLower);
        int upperY = scaleY(actualUpper);
        
        // 渐变填充的箱体
        QRect boxRect(boxLeft, q3Y, boxWidth, q1Y - q3Y);
        QLinearGradient boxGradient(boxLeft, q3Y, boxLeft + boxWidth, q1Y);
        boxGradient.setColorAt(0, colors[0].lighter(140));
        boxGradient.setColorAt(1, colors[0].lighter(110));
        
        painter.setBrush(boxGradient);
        painter.setPen(QPen(colors[0], 2));
        painter.drawRect(boxRect);
        
        // 中位数线
        painter.setPen(QPen(colors[1], 3));
        painter.drawLine(boxLeft, medianY, boxLeft + boxWidth, medianY);
        
        // 上下须线
        painter.setPen(QPen(colors[0], 2));
        int centerX = boxLeft + boxWidth / 2;
        painter.drawLine(centerX, q3Y, centerX, upperY);  // 上须
        painter.drawLine(centerX, q1Y, centerX, lowerY);  // 下须
        painter.drawLine(centerX - 10, upperY, centerX + 10, upperY);  // 上边界
        painter.drawLine(centerX - 10, lowerY, centerX + 10, lowerY);  // 下边界
        
        // 异常值
        painter.setPen(QPen(colors[2], 1));
        painter.setBrush(colors[2]);
        for (double value : sortedData) {
            if (value < lowerFence || value > upperFence) {
                int y = scaleY(value);
                painter.drawEllipse(centerX - 3, y - 3, 6, 6);
            }
        }
        
        // 统计信息标签
        painter.setPen(textColor);
        QFont labelFont("Microsoft YaHei", static_cast<int>(labelFontSize));
        labelFont.setPointSizeF(labelFontSize * devicePixelRatio());
        painter.setFont(labelFont);
        QString statsLabel = QString("第一四分位数: %1  中位数: %2  第三四分位数: %3")
                           .arg(q1, 0, 'f', 2)
                           .arg(median, 0, 'f', 2)
                           .arg(q3, 0, 'f', 2);
        painter.drawText(plotRect.left(), plotRect.top() - 10, statsLabel);
        
        drawAxisLabels(painter, plotRect, -1, 1, yMin, yMax);
    }
    
    void drawViolinPlot(QPainter& painter)
    {
        if (yData.empty()) return;
        
        int margin = 80;
        QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
        
        drawTitle(painter, plotRect);
        drawAxes(painter, plotRect);
        
        // 计算密度估计
        std::vector<double> sortedData = yData;
        std::sort(sortedData.begin(), sortedData.end());
        
        double yMin = sortedData.front();
        double yMax = sortedData.back();
        double yRange = yMax - yMin;
        if (yRange == 0) yRange = 1;
        
        yMin -= yRange * 0.1;
        yMax += yRange * 0.1;
        yRange = yMax - yMin;
        
        int steps = 50;
        std::vector<double> densityY(steps);
        std::vector<double> densityValues(steps);
        
        double bandwidth = yRange / 20.0;  // 简化的带宽选择
        
        for (int i = 0; i < steps; ++i) {
            double y = yMin + (yRange * i) / (steps - 1);
            densityY[i] = y;
            
            double density = 0.0;
            for (double value : sortedData) {
                double diff = (y - value) / bandwidth;
                density += std::exp(-0.5 * diff * diff);  // 高斯核
            }
            density /= (sortedData.size() * bandwidth * std::sqrt(2 * M_PI));
            densityValues[i] = density;
        }
        
        // 找到最大密度用于缩放
        double maxDensity = *std::max_element(densityValues.begin(), densityValues.end());
        if (maxDensity == 0) maxDensity = 1;
        
        int centerX = plotRect.left() + plotRect.width() / 2;
        int maxWidth = plotRect.width() / 4;
        
        // 绘制小提琴形状
        QPolygonF leftSide, rightSide;
        for (int i = 0; i < steps; ++i) {
            double y = densityY[i];
            int plotY = plotRect.bottom() - (int)((y - yMin) / yRange * plotRect.height());
            int width = (int)(maxWidth * densityValues[i] / maxDensity);
            
            leftSide << QPointF(centerX - width, plotY);
            rightSide << QPointF(centerX + width, plotY);
        }
        
        // 组合左右两侧形成完整的小提琴形状
        QPolygonF violin = leftSide;
        for (int i = rightSide.size() - 1; i >= 0; --i) {
            violin << rightSide[i];
        }
        
        // 渐变填充
        QLinearGradient violinGradient(centerX - maxWidth, plotRect.top(), 
                                     centerX + maxWidth, plotRect.bottom());
        violinGradient.setColorAt(0, colors[0].lighter(150));
        violinGradient.setColorAt(0.5, colors[0]);
        violinGradient.setColorAt(1, colors[0].darker(110));
        
        painter.setBrush(violinGradient);
        painter.setPen(QPen(colors[0].darker(), 2));
        painter.drawPolygon(violin);
        
        // 添加箱线图叠加
        double median = (sortedData.size() % 2 == 0) ? 
                       (sortedData[sortedData.size()/2-1] + sortedData[sortedData.size()/2]) / 2.0 : 
                       sortedData[sortedData.size()/2];
        
        int medianY = plotRect.bottom() - (int)((median - yMin) / yRange * plotRect.height());
        painter.setPen(QPen(Qt::white, 3));
        painter.drawLine(centerX - 20, medianY, centerX + 20, medianY);
        
        drawAxisLabels(painter, plotRect, -1, 1, yMin, yMax);
    }
    
    void drawDensityPlot(QPainter& painter)
    {
        if (yData.empty()) return;
        
        int margin = 80;
        QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
        
        drawTitle(painter, plotRect);
        drawGrid(painter, plotRect);
        drawAxes(painter, plotRect);
        
        std::vector<double> sortedData = yData;
        std::sort(sortedData.begin(), sortedData.end());
        
        double yMin = sortedData.front();
        double yMax = sortedData.back();
        double yRange = yMax - yMin;
        if (yRange == 0) yRange = 1;
        
        double xMin = yMin - yRange * 0.1;
        double xMax = yMax + yRange * 0.1;
        double xRange = xMax - xMin;
        
        // 计算密度曲线
        int steps = 200;
        std::vector<QPointF> densityCurve;
        double bandwidth = yRange / 15.0;
        
        double maxDensity = 0;
        for (int i = 0; i < steps; ++i) {
            double x = xMin + (xRange * i) / (steps - 1);
            
            double density = 0.0;
            for (double value : sortedData) {
                double diff = (x - value) / bandwidth;
                density += std::exp(-0.5 * diff * diff);
            }
            density /= (sortedData.size() * bandwidth * std::sqrt(2 * M_PI));
            
            if (density > maxDensity) maxDensity = density;
            densityCurve.push_back(QPointF(x, density));
        }
        
        if (maxDensity == 0) maxDensity = 1;
        
        // 转换坐标并绘制
        QPolygonF curve;
        QPolygonF fillArea;
        
        for (const auto& point : densityCurve) {
            int plotX = plotRect.left() + (int)((point.x() - xMin) / xRange * plotRect.width());
            int plotY = plotRect.bottom() - (int)(point.y() / maxDensity * plotRect.height());
            curve << QPointF(plotX, plotY);
        }
        
        // 创建填充区域
        fillArea = curve;
        fillArea << QPointF(plotRect.right(), plotRect.bottom());
        fillArea << QPointF(plotRect.left(), plotRect.bottom());
        
        // 渐变填充
        QLinearGradient fillGradient(0, plotRect.top(), 0, plotRect.bottom());
        fillGradient.setColorAt(0, colors[0].lighter(170));
        fillGradient.setColorAt(0.7, colors[0].lighter(130));
        fillGradient.setColorAt(1, colors[0].lighter(180));
        
        painter.setBrush(fillGradient);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(fillArea);
        
        // 绘制密度曲线
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(colors[0], 3));
        painter.drawPolyline(curve);
        
        drawAxisLabels(painter, plotRect, xMin, xMax, 0, maxDensity);
    }
    
    void drawAreaChart(QPainter& painter)
    {
        if (xData.empty() || yData.empty()) return;
        
        int margin = 80;
        QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
        
        drawTitle(painter, plotRect);
        drawGrid(painter, plotRect);
        drawAxes(painter, plotRect);
        
        auto xMinMax = std::minmax_element(xData.begin(), xData.end());
        auto yMinMax = std::minmax_element(yData.begin(), yData.end());
        
        double xMin = *xMinMax.first;
        double xMax = *xMinMax.second;
        double yMin = std::min(0.0, *yMinMax.first);
        double yMax = *yMinMax.second;
        
        double xRange = xMax - xMin;
        double yRange = yMax - yMin;
        if (xRange == 0) xRange = 1;
        if (yRange == 0) yRange = 1;
        
        xMin -= xRange * 0.02;
        xMax += xRange * 0.02;
        yMin -= yRange * 0.05;
        yMax += yRange * 0.05;
        
        // 创建面积多边形
        QPolygonF areaPolygon;
        
        // 添加数据点
        for (size_t i = 0; i < xData.size(); ++i) {
            int x = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
            int y = plotRect.bottom() - (int)((yData[i] - yMin) / (yMax - yMin) * plotRect.height());
            areaPolygon << QPointF(x, y);
        }
        
        // 闭合多边形到x轴
        if (!areaPolygon.isEmpty()) {
            int baselineY = plotRect.bottom() - (int)((0 - yMin) / (yMax - yMin) * plotRect.height());
            areaPolygon << QPointF(areaPolygon.last().x(), baselineY);
            areaPolygon << QPointF(areaPolygon.first().x(), baselineY);
        }
        
        // 多层渐变填充
        QLinearGradient areaGradient(0, plotRect.top(), 0, plotRect.bottom());
        areaGradient.setColorAt(0, colors[0].lighter(160));
        areaGradient.setColorAt(0.3, colors[0].lighter(130));
        areaGradient.setColorAt(0.7, colors[0]);
        areaGradient.setColorAt(1, colors[0].lighter(180));
        
        painter.setBrush(areaGradient);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(areaPolygon);
        
        // 绘制边界线
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(colors[0].darker(110), 2));
        
        QPolygonF borderLine;
        for (size_t i = 0; i < xData.size(); ++i) {
            int x = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
            int y = plotRect.bottom() - (int)((yData[i] - yMin) / (yMax - yMin) * plotRect.height());
            borderLine << QPointF(x, y);
        }
        painter.drawPolyline(borderLine);
        
        // 绘制数据点
        for (size_t i = 0; i < xData.size(); ++i) {
            int x = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
            int y = plotRect.bottom() - (int)((yData[i] - yMin) / (yMax - yMin) * plotRect.height());
            
            // 外圈
            painter.setPen(QPen(colors[0], 2));
            painter.setBrush(colors[0]);
            painter.drawEllipse(x - 4, y - 4, 8, 8);
            
            // 内圈
            painter.setPen(QPen(Qt::white, 1));
            painter.setBrush(Qt::white);
            painter.drawEllipse(x - pointSize/2, y - pointSize/2, pointSize, pointSize);
        }
        
        drawAxisLabels(painter, plotRect, xMin, xMax, yMin, yMax);
    }

private:
    ChartType chartType;
    std::vector<double> xData;
    std::vector<double> yData;
    std::vector<QString> dataLabels;
    std::vector<QColor> colors;
    
    QString chartTitle;
    QString xAxisLabel;
    QString yAxisLabel;
    
    QColor backgroundColor;
    QColor gridColor;
    QColor axisColor;
    QColor textColor;
    
    // 可配置属性
    bool showGrid = true;
    bool showLegend = false;
    int lineWidth = 3;
    int pointSize = 5;
    double titleFontSize = 16.0;
    double labelFontSize = 10.0;
    double axisFontSize = 9.0;
    
    QMap<QString, QString> statistics;
    
    // Zoom and pan variables
    double zoomFactor;
    QPointF panOffset;
    bool isDragging;
    QPoint lastPanPoint;
    
public slots:
    void resetView()
    {
        zoomFactor = 1.0;
        panOffset = QPointF(0, 0);
        update();
    }
};

// Forward declaration for simplified AI assistant
class SimpleConfigDialog;  

int main(int argc, char *argv[])
{
    // 设置高DPI支持
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton, true);
    
    QApplication app(argc, argv);
    
    // 检测系统的缩放因子
    qreal scaleFactor = app.devicePixelRatio();
    if (scaleFactor > 1.0) {
        // 对于高DPI显示器，调整字体大小
        QFont defaultFont = app.font();
        defaultFont.setPointSize(static_cast<int>(defaultFont.pointSize() * 1.2));
        app.setFont(defaultFont);
    }
    
    app.setApplicationName("TXT数据绘图工具");
    app.setApplicationVersion("2.1");
    app.setApplicationDisplayName("TXT数据绘图工具 - 中文增强版");

    // 设置命令行解析器
    QCommandLineParser parser;
    parser.setApplicationDescription("增强TXT数据绘图工具，支持多种图表类型和统计可视化");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加文件参数选项
    QCommandLineOption fileOption(QStringList() << "f" << "file",
                                  "启动时从 TXT 文件加载数据。",
                                  "filename");
    parser.addOption(fileOption);
    
    // 添加图表类型选项
    QCommandLineOption chartTypeOption(QStringList() << "t" << "type",
                                     "设置初始图表类型 (线图line, 柱状图bar, 饼图pie, 散点图scatter, 直方图histogram, 箱线图box, 小提琴图violin, 密度图density, 面积图area)。",
                                     "type", "line");
    parser.addOption(chartTypeOption);
    
    // 处理命令行参数
    parser.process(app);
    
    // 获取参数值
    QString fileName = parser.value(fileOption);
    QString chartType = parser.value(chartTypeOption).toLower();
    
    // 创建主窗口
    MainWindow window(fileName);
    
    // 设置初始图表类型
    if (!chartType.isEmpty() && chartType != "line") {
        if (chartType == "bar") window.setInitialChartType(ChartType::Bar);
        else if (chartType == "pie") window.setInitialChartType(ChartType::Pie);
        else if (chartType == "scatter") window.setInitialChartType(ChartType::Scatter);
        else if (chartType == "histogram") window.setInitialChartType(ChartType::Histogram);
        else if (chartType == "box") window.setInitialChartType(ChartType::BoxPlot);
        else if (chartType == "violin") window.setInitialChartType(ChartType::ViolinPlot);
        else if (chartType == "density") window.setInitialChartType(ChartType::DensityPlot);
        else if (chartType == "area") window.setInitialChartType(ChartType::AreaChart);
    }
    
    window.show();

    return app.exec();
}

