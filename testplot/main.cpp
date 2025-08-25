#include <QApplication>

#include "mainwindow.h"
#include "qcommandlineparser.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
        defaultFont.setPointSize(static_cast<int>(defaultFont.pointSize() * 1.5));
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

