#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QColor>
#include <vector>
#include <map>

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
    explicit PlotWidget(QWidget *parent = nullptr);
    
    void setData(const std::vector<double>& x, const std::vector<double>& y);
    void setData(const std::vector<double>& data);
    void setData(const std::vector<std::vector<double>>& allData, int xCol, int yCol);
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
    
    bool showGrid = true;
    bool showLegend = false;
    int lineWidth = 3;
    int pointSize = 5;
    double titleFontSize = 16.0;
    double labelFontSize = 10.0;
    double axisFontSize = 9.0;
    
    QMap<QString, QString> statistics;
    
    double zoomFactor;
    QPointF panOffset;
    bool isDragging;
    QPoint lastPanPoint;
};

#endif // PLOTWIDGET_H