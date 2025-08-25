#include "plotwidget_new.h"
#include <QApplication>
#include <QTextCodec>
#include <random>
#include <regex>

PlotWidget::PlotWidget(QWidget *parent) 
    : QWidget(parent), chartType(ChartType::Line),
      zoomFactor(1.0), panOffset(0, 0), isDragging(false), isMultiSeries(false),
      hasFitting(false), fittingDegree(0), showResidualChart(false)
{
    setMinimumSize(static_cast<int>(600 * 1.5), 
                   static_cast<int>(400 * 1.5));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    
    // 设置美化的现代颜色方案
    colors = {
        QColor(56, 142, 142),   // Modern Teal
        QColor(235, 77, 75),    // Sunset Orange
        QColor(106, 176, 76),   // Soft Green
        QColor(247, 183, 49),   // Warm Yellow
        QColor(162, 155, 254),  // Lavender
        QColor(255, 118, 117),  // Light Coral
        QColor(116, 185, 255),  // Sky Blue
        QColor(253, 203, 110),  // Peach
        QColor(123, 237, 159),  // Mint Green
        QColor(255, 107, 107)   // Light Red
    };
    
    // 设置默认样式
    backgroundColor = QColor(248, 249, 250);
    gridColor = QColor(220, 220, 220);
    axisColor = QColor(52, 58, 64);
    textColor = QColor(52, 58, 64);
    
    // 设置阴影效果
    auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(15);
    shadowEffect->setXOffset(3);
    shadowEffect->setYOffset(3);
    shadowEffect->setColor(QColor(0, 0, 0, 80));
    setGraphicsEffect(shadowEffect);
    
    // 设置鼠标跟踪和缩放参数
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_AcceptTouchEvents);
}

void PlotWidget::setData(const std::vector<double>& x, const std::vector<double>& y)
{
    xData = x;
    yData = y;
    calculateStatistics();
    update();
}

void PlotWidget::setData(const std::vector<double>& data)
{
    yData = data;
    xData.clear();
    for (size_t i = 0; i < data.size(); ++i) {
        xData.push_back(static_cast<double>(i + 1));
    }
    calculateStatistics();
    update();
}

void PlotWidget::setData(const std::vector<std::vector<double>>& allData, int xCol, int yCol)
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

void PlotWidget::setMultiSeriesData(const std::vector<double>& x, const std::vector<std::vector<double>>& ySeries, const std::vector<QString>& seriesNames)
{
    xData = x;
    ySeriesData = ySeries;
    this->seriesNames = seriesNames;
    isMultiSeries = true;
    
    // Use first series for statistics
    if (!ySeries.empty()) {
        yData = ySeries[0];
        calculateStatistics();
    }
    update();
}

void PlotWidget::setLabels(const std::vector<QString>& labels)
{
    dataLabels = labels;
    update();
}

void PlotWidget::clearData()
{
    xData.clear();
    yData.clear();
    dataLabels.clear();
    statistics.clear();
    ySeriesData.clear();
    seriesNames.clear();
    isMultiSeries = false;
    hasFitting = false;
    fittingCoefficients.clear();
    residuals.clear();
    showResidualChart = false;
    update();
}

void PlotWidget::setChartType(ChartType type)
{
    chartType = type;
    update();
}

void PlotWidget::setTitle(const QString& title)
{
    chartTitle = title;
    update();
}

void PlotWidget::setAxisLabels(const QString& xLabel, const QString& yLabel)
{
    xAxisLabel = xLabel;
    yAxisLabel = yLabel;
    update();
}

void PlotWidget::setColors(const QColor& bg, const QColor& grid, const QColor& axis, const QColor& text)
{
    backgroundColor = bg;
    gridColor = grid;
    axisColor = axis;
    textColor = text;
    update();
}

void PlotWidget::setGridVisible(bool visible)
{
    showGrid = visible;
    update();
}

void PlotWidget::setLineWidth(int width)
{
    lineWidth = width;
    update();
}

void PlotWidget::setPointSize(int size)
{
    pointSize = size;
    update();
}

void PlotWidget::setFontSizes(double title, double label, double axis)
{
    titleFontSize = title;
    labelFontSize = label;
    axisFontSize = axis;
    update();
}

void PlotWidget::resetView()
{
    zoomFactor = 1.0;
    panOffset = QPointF(0, 0);
    update();
}

void PlotWidget::setPolynomialFitting(int degree, const std::vector<double>& coefficients, const std::vector<double>& residuals, const QString& typeName)
{
    hasFitting = true;
    fittingDegree = degree;
    fittingCoefficients = coefficients;
    this->residuals = residuals;
    fittingTypeName = typeName;
    showResidualChart = true;
    update();
}

void PlotWidget::clearFitting()
{
    hasFitting = false;
    fittingCoefficients.clear();
    residuals.clear();
    fittingTypeName.clear();
    showResidualChart = false;
    update();
}

void PlotWidget::paintEvent(QPaintEvent *event)
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
    if (showResidualChart && hasFitting) {
        // 分割绘图区域：上部分显示原图表+拟合线，下部分显示残差图
        // 增加间距分离
        int separatorHeight = 20;
        QRect topRect = rect().adjusted(0, 0, 0, -height()/3 - separatorHeight/2);
        QRect bottomRect = rect().adjusted(0, height()*2/3 + separatorHeight/2, 0, 0);
        
        // 临时设置绘图区域并绘制主图表
        painter.setClipRect(topRect);
        drawMainChart(painter);
        
        // 绘制分割线
        painter.setClipping(false);
        painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
        int separatorY = height()*2/3;
        painter.drawLine(0, separatorY, width(), separatorY);
        
        // 绘制残差图
        painter.setClipRect(bottomRect);
        drawResidualChart(painter, bottomRect);
        painter.setClipping(false);
    } else {
        // 正常绘制
        drawMainChart(painter);
    }
}

void PlotWidget::calculateStatistics()
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

void PlotWidget::drawNoDataMessage(QPainter& painter)
{
    painter.setPen(textColor);
    QFont font("Microsoft YaHei", static_cast<int>(labelFontSize + 4));
    font.setPointSizeF((labelFontSize + 4) * 1.5);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter, "暂无数据显示\n\n请加载 TXT 文件查看图表");
}

void PlotWidget::drawTitle(QPainter& painter, const QRect& plotRect)
{
    if (!chartTitle.isEmpty()) {
        painter.setPen(textColor);
        QFont titleFont("Microsoft YaHei", static_cast<int>(titleFontSize), QFont::Bold);
        titleFont.setPointSizeF(titleFontSize * 1.5);
        painter.setFont(titleFont);
        QRect titleRect = QRect(plotRect.left(), 10, plotRect.width(), 30);
        painter.drawText(titleRect, Qt::AlignCenter, chartTitle);
    }
}

void PlotWidget::drawAxes(QPainter& painter, const QRect& plotRect)
{
    if (chartType == ChartType::Pie) return;
    
    painter.setPen(QPen(axisColor, 2));
    painter.drawLine(plotRect.bottomLeft(), plotRect.bottomRight()); // X轴
    painter.drawLine(plotRect.bottomLeft(), plotRect.topLeft());     // Y轴
    
    // 绘制轴标签
    painter.setPen(textColor);
    QFont labelFont("Microsoft YaHei", static_cast<int>(labelFontSize));
    labelFont.setPointSizeF(labelFontSize * 1.5);
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

void PlotWidget::drawGrid(QPainter& painter, const QRect& plotRect)
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

// 鼠标和键盘事件处理
void PlotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void PlotWidget::mouseMoveEvent(QMouseEvent *event)
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

void PlotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

void PlotWidget::wheelEvent(QWheelEvent *event)
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

void PlotWidget::keyPressEvent(QKeyEvent *event)
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

void PlotWidget::drawLineChart(QPainter& painter)
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
    }
    
    drawAxisLabels(painter, plotRect, xMin, xMax, yMin, yMax);
    drawLegend(painter, plotRect); // 也为单系列图表绘制legend（如果有拟合线）
}

void PlotWidget::drawBarChart(QPainter& painter)
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
        
        QLinearGradient gradient(x, y, x, y + barHeight);
        QColor color = colors[i % colors.size()];
        gradient.setColorAt(0, color.lighter(140));
        gradient.setColorAt(1, color);
        
        painter.setBrush(gradient);
        painter.setPen(QPen(color.darker(), 2));
        painter.drawRoundedRect(barRect, 4, 4);
        
        // 数值标签
        painter.setPen(textColor);
        QFont valueFont("Microsoft YaHei", static_cast<int>(axisFontSize + 1));
        painter.setFont(valueFont);
        QString valueText = QString::number(yData[i], 'f', 1);
        QRect textRect(x, y - 25, barWidth, 20);
        painter.drawText(textRect, Qt::AlignCenter, valueText);
    }
}

void PlotWidget::drawPieChart(QPainter& painter)
{
    if (yData.empty()) return;
    
    drawTitle(painter, rect());
    
    int size = std::min(width(), height()) - 200;
    QRect pieRect((width() - size) / 2, (height() - size) / 2, size, size);
    
    double total = std::accumulate(yData.begin(), yData.end(), 0.0);
    if (total <= 0) return;
    
    int startAngle = 0;
    
    for (size_t i = 0; i < yData.size(); ++i) {
        int spanAngle = (int)(yData[i] / total * 5760);
        
        QColor color = colors[i % colors.size()];
        QRadialGradient gradient(pieRect.center(), size / 2);
        gradient.setColorAt(0, color.lighter(150));
        gradient.setColorAt(1, color.darker(120));
        
        painter.setBrush(gradient);
        painter.setPen(QPen(Qt::white, 3));
        painter.drawPie(pieRect, startAngle, spanAngle);
        
        startAngle += spanAngle;
    }
}

void PlotWidget::drawScatterChart(QPainter& painter)
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
        painter.setPen(QPen(color.darker(), 2));
        painter.setBrush(QBrush(color));
        painter.drawEllipse(x - 6, y - 6, 12, 12);
    }
    
    drawAxisLabels(painter, plotRect, xMin, xMax, yMin, yMax);
    drawLegend(painter, plotRect); // 也为单系列图表绘制legend（如果有拟合线）
}

void PlotWidget::drawHistogram(QPainter& painter)
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
    }
}

void PlotWidget::drawBoxPlot(QPainter& painter)
{
    // 简化的箱线图实现
    int margin = 80;
    QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
    
    drawTitle(painter, plotRect);
    drawAxes(painter, plotRect);
    
    if (yData.empty()) return;
    
    std::vector<double> sortedData = yData;
    std::sort(sortedData.begin(), sortedData.end());
    
    size_t n = sortedData.size();
    double q1 = sortedData[n / 4];
    double median = (n % 2 == 0) ? (sortedData[n/2-1] + sortedData[n/2]) / 2.0 : sortedData[n/2];
    double q3 = sortedData[3 * n / 4];
    
    double yMin = sortedData.front();
    double yMax = sortedData.back();
    double yRange = yMax - yMin;
    if (yRange == 0) yRange = 1;
    
    yMin -= yRange * 0.1;
    yMax += yRange * 0.1;
    yRange = yMax - yMin;
    
    int boxWidth = plotRect.width() / 3;
    int boxLeft = plotRect.left() + plotRect.width() / 2 - boxWidth / 2;
    
    auto scaleY = [&](double value) -> int {
        return plotRect.bottom() - (int)((value - yMin) / yRange * plotRect.height());
    };
    
    int q1Y = scaleY(q1);
    int medianY = scaleY(median);
    int q3Y = scaleY(q3);
    
    // 箱体
    QRect boxRect(boxLeft, q3Y, boxWidth, q1Y - q3Y);
    painter.setBrush(colors[0].lighter(140));
    painter.setPen(QPen(colors[0], 2));
    painter.drawRect(boxRect);
    
    // 中位数线
    painter.setPen(QPen(colors[1], 3));
    painter.drawLine(boxLeft, medianY, boxLeft + boxWidth, medianY);
}

void PlotWidget::drawViolinPlot(QPainter& painter)
{
    // 简化的小提琴图实现
    drawBoxPlot(painter);
}

void PlotWidget::drawDensityPlot(QPainter& painter)
{
    // 简化的密度图实现
    drawLineChart(painter);
}

void PlotWidget::drawAreaChart(QPainter& painter)
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
    
    for (size_t i = 0; i < xData.size(); ++i) {
        int x = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
        int y = plotRect.bottom() - (int)((yData[i] - yMin) / (yMax - yMin) * plotRect.height());
        areaPolygon << QPointF(x, y);
    }
    
    if (!areaPolygon.isEmpty()) {
        int baselineY = plotRect.bottom();
        areaPolygon << QPointF(areaPolygon.last().x(), baselineY);
        areaPolygon << QPointF(areaPolygon.first().x(), baselineY);
    }
    
    QLinearGradient areaGradient(0, plotRect.top(), 0, plotRect.bottom());
    areaGradient.setColorAt(0, colors[0].lighter(160));
    areaGradient.setColorAt(1, colors[0].lighter(180));
    
    painter.setBrush(areaGradient);
    painter.setPen(Qt::NoPen);
    painter.drawPolygon(areaPolygon);
    
    drawAxisLabels(painter, plotRect, xMin, xMax, yMin, yMax);
}

void PlotWidget::drawAxisLabels(QPainter& painter, const QRect& plotRect, double xMin, double xMax, double yMin, double yMax)
{
    painter.setPen(textColor);
    QFont axisFont("Microsoft YaHei", static_cast<int>(axisFontSize));
    painter.setFont(axisFont);
    QFontMetrics fm(axisFont);
    
    // Y轴标签 - 确保标签不超出边界
    for (int i = 0; i <= 5; ++i) {
        double value = yMin + (yMax - yMin) * i / 5.0;
        int y = plotRect.bottom() - (int)(plotRect.height() * i / 5.0);
        QString valueText = QString::number(value, 'f', 1);
        int textWidth = fm.width(valueText);
        
        // 确保Y轴标签在左边界内
        int textX = std::max(5, plotRect.left() - textWidth - 8);
        painter.drawText(textX, y + 4, valueText);
    }
    
    // X轴标签 - 确保标签不超出边界并且居中对齐
    for (int i = 0; i <= 5; ++i) {
        double value = xMin + (xMax - xMin) * i / 5.0;
        int x = plotRect.left() + (int)(plotRect.width() * i / 5.0);
        QString valueText = QString::number(value, 'f', 1);
        int textWidth = fm.width(valueText);
        
        // 居中对齐X轴标签，确保不超出边界
        int textX = x - textWidth / 2;
        textX = std::max(0, std::min(textX, width() - textWidth));
        
        // 确保Y位置在底部边界内
        int textY = std::min(plotRect.bottom() + 18, height() - 5);
        painter.drawText(textX, textY, valueText);
    }
}

void PlotWidget::drawLegend(QPainter& painter, const QRect& plotRect)
{
    // 计算需要显示的项目
    QStringList legendItems;
    
    // 添加多系列数据名称
    if (isMultiSeries && !seriesNames.empty()) {
        for (const QString& name : seriesNames) {
            legendItems.append(name);
        }
    }
    
    // 添加拟合线
    if (hasFitting && !fittingTypeName.isEmpty()) {
        legendItems.append(fittingTypeName);
    }
    
    // 如果没有任何项目，直接返回
    if (legendItems.isEmpty()) return;
    
    // Calculate legend dimensions
    QFont legendFont("Microsoft YaHei", static_cast<int>(axisFontSize - 1));
    painter.setFont(legendFont);
    QFontMetrics fm(legendFont);
    
    int maxTextWidth = 0;
    for (const QString& name : legendItems) {
        maxTextWidth = std::max(maxTextWidth, fm.width(name));
    }
    
    int legendWidth = maxTextWidth + 40; // icon + spacing + text + margins
    int itemHeight = 18;
    int legendHeight = legendItems.size() * itemHeight + 10;
    int legendX = plotRect.right() - legendWidth - 15;
    int legendY = plotRect.top() + 15;
    
    // Draw modern legend with subtle shadow
    QRect legendRect(legendX, legendY, legendWidth, legendHeight);
    
    // Drop shadow
    painter.fillRect(legendRect.adjusted(2, 2, 2, 2), QColor(0, 0, 0, 30));
    
    // Main background - no background, just items
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::NoBrush);
    
    // Draw legend items with modern styling
    int itemIndex = 0;
    
    // Draw multi-series items
    if (isMultiSeries && !seriesNames.empty()) {
        for (size_t i = 0; i < seriesNames.size() && i < colors.size(); ++i) {
            int itemY = legendY + 5 + itemIndex * itemHeight;
            QColor seriesColor = colors[i % colors.size()];
            
            // Draw modern line indicator
            painter.setPen(QPen(seriesColor, 3));
            painter.drawLine(legendX + 8, itemY + 7, legendX + 20, itemY + 7);
            
            // Draw small circle on the line
            painter.setBrush(seriesColor);
            painter.setPen(QPen(seriesColor.darker(120), 1));
            painter.drawEllipse(legendX + 12, itemY + 4, 6, 6);
            
            // Draw series name
            painter.setPen(textColor);
            painter.setBrush(Qt::NoBrush);
            painter.drawText(legendX + 28, itemY + 11, seriesNames[i]);
            itemIndex++;
        }
    }
    
    // Draw fitting line item
    if (hasFitting && !fittingTypeName.isEmpty()) {
        int itemY = legendY + 5 + itemIndex * itemHeight;
        QColor fittingColor = QColor(255, 0, 0); // 红色拟合线
        
        // Draw dashed line indicator for fitting
        painter.setPen(QPen(fittingColor, 3, Qt::DashLine));
        painter.drawLine(legendX + 8, itemY + 7, legendX + 20, itemY + 7);
        
        // Draw fitting line name
        painter.setPen(textColor);
        painter.setBrush(Qt::NoBrush);
        painter.drawText(legendX + 28, itemY + 11, fittingTypeName);
    }
}

void PlotWidget::drawMultiSeriesLineChart(QPainter& painter)
{
    if (xData.empty() || ySeriesData.empty()) return;
    
    int margin = 80;
    QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
    
    drawTitle(painter, plotRect);
    drawGrid(painter, plotRect);
    drawAxes(painter, plotRect);
    
    // Calculate combined data range
    double xMin = *std::min_element(xData.begin(), xData.end());
    double xMax = *std::max_element(xData.begin(), xData.end());
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    
    for (const auto& series : ySeriesData) {
        if (!series.empty()) {
            auto minmax = std::minmax_element(series.begin(), series.end());
            yMin = std::min(yMin, *minmax.first);
            yMax = std::max(yMax, *minmax.second);
        }
    }
    
    double xRange = xMax - xMin;
    double yRange = yMax - yMin;
    if (xRange == 0) xRange = 1;
    if (yRange == 0) yRange = 1;
    
    xMin -= xRange * 0.05;
    xMax += xRange * 0.05;
    yMin -= yRange * 0.05;
    yMax += yRange * 0.05;
    
    // Draw each series
    for (size_t seriesIdx = 0; seriesIdx < ySeriesData.size(); ++seriesIdx) {
        const auto& ySeriesVec = ySeriesData[seriesIdx];
        if (ySeriesVec.empty() || xData.size() != ySeriesVec.size()) continue;
        
        QColor seriesColor = colors[seriesIdx % colors.size()];
        
        // Draw lines
        painter.setPen(QPen(seriesColor, 3));
        painter.setBrush(Qt::NoBrush);
        for (size_t i = 0; i < xData.size() - 1; ++i) {
            int x1 = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
            int y1 = plotRect.bottom() - (int)((ySeriesVec[i] - yMin) / (yMax - yMin) * plotRect.height());
            int x2 = plotRect.left() + (int)((xData[i+1] - xMin) / (xMax - xMin) * plotRect.width());
            int y2 = plotRect.bottom() - (int)((ySeriesVec[i+1] - yMin) / (yMax - yMin) * plotRect.height());
            
            painter.drawLine(x1, y1, x2, y2);
        }
        
        // Draw points
        for (size_t i = 0; i < xData.size(); ++i) {
            int x = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
            int y = plotRect.bottom() - (int)((ySeriesVec[i] - yMin) / (yMax - yMin) * plotRect.height());
            
            painter.setPen(QPen(seriesColor.darker(), 2));
            painter.setBrush(seriesColor);
            painter.drawEllipse(x - pointSize, y - pointSize, pointSize * 2, pointSize * 2);
        }
    }
    
    drawAxisLabels(painter, plotRect, xMin, xMax, yMin, yMax);
    drawLegend(painter, plotRect);
}

void PlotWidget::drawMultiSeriesScatterChart(QPainter& painter)
{
    if (xData.empty() || ySeriesData.empty()) return;
    
    int margin = 80;
    QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
    
    drawTitle(painter, plotRect);
    drawGrid(painter, plotRect);
    drawAxes(painter, plotRect);
    
    // Calculate combined data range
    double xMin = *std::min_element(xData.begin(), xData.end());
    double xMax = *std::max_element(xData.begin(), xData.end());
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    
    for (const auto& series : ySeriesData) {
        if (!series.empty()) {
            auto minmax = std::minmax_element(series.begin(), series.end());
            yMin = std::min(yMin, *minmax.first);
            yMax = std::max(yMax, *minmax.second);
        }
    }
    
    double xRange = xMax - xMin;
    double yRange = yMax - yMin;
    if (xRange == 0) xRange = 1;
    if (yRange == 0) yRange = 1;
    
    xMin -= xRange * 0.05;
    xMax += xRange * 0.05;
    yMin -= yRange * 0.05;
    yMax += yRange * 0.05;
    
    // Draw each series
    for (size_t seriesIdx = 0; seriesIdx < ySeriesData.size(); ++seriesIdx) {
        const auto& ySeriesVec = ySeriesData[seriesIdx];
        if (ySeriesVec.empty() || xData.size() != ySeriesVec.size()) continue;
        
        QColor seriesColor = colors[seriesIdx % colors.size()];
        
        for (size_t i = 0; i < xData.size(); ++i) {
            int x = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
            int y = plotRect.bottom() - (int)((ySeriesVec[i] - yMin) / (yMax - yMin) * plotRect.height());
            
            painter.setPen(QPen(seriesColor.darker(), 2));
            painter.setBrush(QBrush(seriesColor));
            painter.drawEllipse(x - 6, y - 6, 12, 12);
        }
    }
    
    drawAxisLabels(painter, plotRect, xMin, xMax, yMin, yMax);
    drawLegend(painter, plotRect);
}

// Additional functions for PlotWidget - append to plotwidget_new.cpp

void PlotWidget::drawMainChart(QPainter& painter)
{
    if (isMultiSeries) {
        switch (chartType) {
            case ChartType::Line:
                drawMultiSeriesLineChart(painter);
                break;
            case ChartType::Scatter:
                drawMultiSeriesScatterChart(painter);
                break;
            default:
                drawLineChart(painter);
                break;
        }
    } else {
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
    
    // 绘制拟合线 (如果有)
    if (hasFitting && (chartType == ChartType::Line || chartType == ChartType::Scatter)) {
        int margin = 80;
        QRect plotRect = rect().adjusted(margin, margin, -margin, -margin);
        
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
        
        drawFittingLine(painter, plotRect, xMin, xMax, yMin, yMax);
    }
}

void PlotWidget::drawFittingLine(QPainter& painter, const QRect& plotRect, double xMin, double xMax, double yMin, double yMax)
{
    if (!hasFitting || fittingCoefficients.empty()) return;
    
    painter.setPen(QPen(QColor(255, 0, 0), 3, Qt::DashLine));
    
    int numPoints = 100;
    QPointF prevPoint;
    bool firstPoint = true;
    
    for (int i = 0; i <= numPoints; ++i) {
        double x = xMin + (xMax - xMin) * i / numPoints;
        double y = 0.0;
        
        // 根据拟合类型计算y值
        if (fittingDegree <= 2) {
            // 多项式拟合
            for (size_t j = 0; j < fittingCoefficients.size(); ++j) {
                y += fittingCoefficients[j] * std::pow(x, j);
            }
        } else if (fittingDegree == 3 && fittingCoefficients.size() >= 4) {
            // 正弦拟合: y = A*sin(B*x + C) + D
            y = fittingCoefficients[0] * std::sin(fittingCoefficients[1] * x + fittingCoefficients[2]) + fittingCoefficients[3];
        } else if (fittingDegree == 4 && fittingCoefficients.size() >= 3) {
            // 高斯拟合: y = A*exp(-((x-B)/C)^2)
            double arg = (x - fittingCoefficients[1]) / fittingCoefficients[2];
            y = fittingCoefficients[0] * std::exp(-arg * arg);
        }
        
        int screenX = plotRect.left() + (int)((x - xMin) / (xMax - xMin) * plotRect.width());
        int screenY = plotRect.bottom() - (int)((y - yMin) / (yMax - yMin) * plotRect.height());
        
        QPointF currentPoint(screenX, screenY);
        
        if (!firstPoint && screenY >= plotRect.top() && screenY <= plotRect.bottom()) {
            painter.drawLine(prevPoint, currentPoint);
        }
        
        prevPoint = currentPoint;
        firstPoint = false;
    }
}

void PlotWidget::drawResidualChart(QPainter& painter, const QRect& chartRect)
{
    if (!hasFitting || residuals.empty() || xData.size() != residuals.size()) return;
    
    // 缩短残差图左右长度 - 增加左右边距
    int horizontalMargin = chartRect.width() / 6; // 左右各留1/6宽度
    int verticalMargin = 20;
    QRect plotRect = chartRect.adjusted(horizontalMargin, verticalMargin, -horizontalMargin, -verticalMargin);
    
    // 移除背景方框，保持透明背景
    
    painter.setPen(textColor);
    QFont titleFont("Microsoft YaHei", static_cast<int>(axisFontSize + 1), QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(plotRect.center().x() - 30, plotRect.top() - 5, QString("残差图"));
    
    if (residuals.empty()) return;
    
    auto residualMinMax = std::minmax_element(residuals.begin(), residuals.end());
    double residualMin = *residualMinMax.first;
    double residualMax = *residualMinMax.second;
    
    if (std::abs(residualMax - residualMin) < 1e-10) {
        residualMin -= 0.1;
        residualMax += 0.1;
    }
    
    double xMin = *std::min_element(xData.begin(), xData.end());
    double xMax = *std::max_element(xData.begin(), xData.end());
    
    double zeroY = plotRect.bottom() - ((0.0 - residualMin) / (residualMax - residualMin) * plotRect.height());
    painter.setPen(QPen(Qt::gray, 1, Qt::DotLine));
    painter.drawLine(plotRect.left(), (int)zeroY, plotRect.right(), (int)zeroY);
    
    int barWidth = std::max(2, plotRect.width() / (int)residuals.size());
    
    for (size_t i = 0; i < residuals.size() && i < xData.size(); ++i) {
        int x = plotRect.left() + (int)((xData[i] - xMin) / (xMax - xMin) * plotRect.width());
        double residual = residuals[i];
        
        int barHeight = (int)(std::abs(residual) / (residualMax - residualMin) * plotRect.height());
        int barY = (int)zeroY;
        
        if (residual > 0) {
            barY -= barHeight;
        }
        
        QColor barColor = (residual > 0) ? QColor(100, 149, 237, 150) : QColor(220, 20, 60, 150);
        painter.setBrush(barColor);
        painter.setPen(barColor.darker());
        painter.drawRect(x - barWidth/2, barY, barWidth, barHeight);
    }
    
    // R²值计算
    double ssRes = 0.0, ssTot = 0.0;
    double yMean = std::accumulate(yData.begin(), yData.end(), 0.0) / yData.size();
    
    for (size_t i = 0; i < residuals.size(); ++i) {
        ssRes += residuals[i] * residuals[i];
        if (i < yData.size()) {
            ssTot += (yData[i] - yMean) * (yData[i] - yMean);
        }
    }
    
    double rSquared = (ssTot > 0) ? 1.0 - (ssRes / ssTot) : 0.0;
    
    painter.setPen(textColor);
    QFont labelFont("Microsoft YaHei", static_cast<int>(axisFontSize));
    painter.setFont(labelFont);
    painter.drawText(plotRect.right() - 80, plotRect.top() + 15, QString("R² = %1").arg(rSquared, 0, 'f', 4));
}