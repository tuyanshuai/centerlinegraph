#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QSizePolicy>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QPolygonF>
#include <vector>
#include <map>
#include <QString>
#include <QMap>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum class ChartType {
    Line,
    Bar,
    Pie,
    Scatter,
    Histogram,
    BoxPlot,
    ViolinPlot,
    DensityPlot,
    AreaChart
};

class PlotWidget : public QWidget
{
    Q_OBJECT

public:
    PlotWidget(QWidget *parent = nullptr);
    
    void setData(const std::vector<double>& x, const std::vector<double>& y);
    void setData(const std::vector<double>& data);
    void setData(const std::vector<std::vector<double>>& allData, int xCol, int yCol);
    void setMultiSeriesData(const std::vector<double>& x, const std::vector<std::vector<double>>& ySeries, const std::vector<QString>& seriesNames);
    void setLabels(const std::vector<QString>& labels);
    void clearData();
    void setChartType(ChartType type);
    void setTitle(const QString& title);
    void setAxisLabels(const QString& xLabel, const QString& yLabel);
    void setColors(const QColor& bg, const QColor& grid, const QColor& axis, const QColor& text);
    void setGridVisible(bool visible);
    void setLineWidth(int width);
    void setPointSize(int size);
    void setFontSizes(double title, double label, double axis);
    void setPolynomialFitting(int degree, const std::vector<double>& coefficients, const std::vector<double>& residuals, const QString& typeName = "多项式拟合");
    void clearFitting();

public slots:
    void resetView();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void calculateStatistics();
    void drawNoDataMessage(QPainter& painter);
    void drawTitle(QPainter& painter, const QRect& plotRect);
    void drawAxes(QPainter& painter, const QRect& plotRect);
    void drawGrid(QPainter& painter, const QRect& plotRect);
    void drawLineChart(QPainter& painter);
    void drawBarChart(QPainter& painter);
    void drawPieChart(QPainter& painter);
    void drawScatterChart(QPainter& painter);
    void drawHistogram(QPainter& painter);
    void drawBoxPlot(QPainter& painter);
    void drawViolinPlot(QPainter& painter);
    void drawDensityPlot(QPainter& painter);
    void drawAreaChart(QPainter& painter);
    void drawAxisLabels(QPainter& painter, const QRect& plotRect, double xMin, double xMax, double yMin, double yMax);
    void drawLegend(QPainter& painter, const QRect& plotRect);
    void drawMultiSeriesLineChart(QPainter& painter);
    void drawMultiSeriesScatterChart(QPainter& painter);
    void drawFittingLine(QPainter& painter, const QRect& plotRect, double xMin, double xMax, double yMin, double yMax);
    void drawResidualChart(QPainter& painter, const QRect& plotRect);
    void drawMainChart(QPainter& painter);

private:
    ChartType chartType;
    std::vector<double> xData;
    std::vector<double> yData;
    std::vector<QString> dataLabels;
    
    // 多系列数据支持
    bool isMultiSeries;
    std::vector<std::vector<double>> ySeriesData;
    std::vector<QString> seriesNames;
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
    
    // Polynomial fitting variables
    bool hasFitting;
    int fittingDegree;
    std::vector<double> fittingCoefficients;
    std::vector<double> residuals;
    bool showResidualChart;
    QString fittingTypeName; // 拟合类型名称，用于legend显示
};

#endif // PLOTWIDGET_H