#include "mainwindow.h"
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
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <sstream>
#include <vector>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <numeric>
#include <regex>
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
#include "plotwidget_new.h"
#include "deepseek_dialog.h"
MainWindow::MainWindow(const QString& initialFile , QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("TXT数据绘图工具 - 中文增强版");
    setMinimumSize(static_cast<int>(1000 * 1.5),
                   static_cast<int>(550 * 1.5));

    setupUI();
    connectSignals();
    setupDeepSeekDialog();

    // Load initial file if provided
    if (!initialFile.isEmpty()) {
        loadDataFromFile(initialFile);
    }
}

void MainWindow::setInitialChartType(ChartType type)
{
    int index = static_cast<int>(type);
    if (index >= 0 && index < chartTypeCombo->count()) {
        chartTypeCombo->setCurrentIndex(index);
    }
    plotWidget->setChartType(type);
}

void MainWindow::loadFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "打开数据文件",
                                                    "",
                                                    "文本文件 (*.txt);;所有文件 (*)"); // Chinese filter

    if (!fileName.isEmpty()) {
        loadDataFromFile(fileName);
    }
}

void MainWindow::applyColumnSelection()
{
    if (rawData.empty()) {
        statusLabel->setText("❌ 错误：未加载数据");
        return;
    }

    // Validate column indices before proceeding
    if (!validateColumnIndices()) {
        statusLabel->setText("❌ 错误：列索引无效，请重新选择数据文件");
        return;
    }

    std::vector<double> xData;
    
    // 检查是否使用多列模式
    if (multiColumnCheckbox->isChecked() && multiColumnCheckbox->isVisible()) {
        // 验证多列模式的前提条件
        if (columnCheckboxes.empty()) {
            statusLabel->setText("❌ 错误：多列选择未初始化，切换回单列模式");
            multiColumnCheckbox->setChecked(false);
            columnCheckboxArea->setVisible(false);
            yColumnCombo->setVisible(true);
        } else {
            // 多列模式
            applyMultiColumnSelection();
            return;
        }
    }
    
    // 单列模式 - 原有逻辑
    if (xColumnCombo->currentIndex() < 0 || yColumnCombo->currentIndex() < 0) {
        statusLabel->setText("❌ 错误：列选择无效");
        return;
    }

    int xCol = xColumnCombo->currentIndex();
    int yCol = yColumnCombo->currentIndex();

    // Validate column indices
    if (xCol >= columnHeaders.size() || yCol >= columnHeaders.size()) {
        statusLabel->setText("❌ 错误：列选择无效");
        return;
    }

    std::vector<double> yData;

    // Handle X column selection
    if (xCol == 0) {
        // Virtual row index column selected
        for (size_t i = 0; i < rawData.size(); ++i) {
            xData.push_back(static_cast<double>(i + 1));
        }
    } else {
        // Real data column selected (adjust index since virtual column is at 0)
        int realXCol = xCol - 1;
        if (realXCol < 0 || realXCol >= (int)rawData[0].size()) {
            statusLabel->setText("❌ 错误：X列选择无效");
            return;
        }
        for (const auto& row : rawData) {
            if (realXCol < (int)row.size()) {
                xData.push_back(row[realXCol]);
            } else {
                xData.push_back(0.0); // Default value for missing data
            }
        }
    }

    // Handle Y column selection
    if (yCol == 0) {
        // Virtual row index column selected for Y (unusual but allowed)
        for (size_t i = 0; i < rawData.size(); ++i) {
            yData.push_back(static_cast<double>(i + 1));
        }
    } else {
        // Real data column selected (adjust index since virtual column is at 0)
        int realYCol = yCol - 1;
        if (realYCol < 0 || realYCol >= (int)rawData[0].size()) {
            statusLabel->setText("❌ 错误：Y列选择无效");
            return;
        }
        for (const auto& row : rawData) {
            if (realYCol < (int)row.size()) {
                yData.push_back(row[realYCol]);
            } else {
                yData.push_back(0.0); // Default value for missing data
            }
        }
    }

    // Set the plot data
    plotWidget->setData(xData, yData);

    // Update statistics for the Y column
    updateDetailedStatistics(yData);

    // Update axis labels
    xLabelEdit->setText(columnHeaders[xCol]);
    yLabelEdit->setText(columnHeaders[yCol]);

    // Update status message
    statusLabel->setText(QString("✅ 正在绘制 %1 (X轴) vs %2 (Y轴)")
                             .arg(columnHeaders[xCol])
                             .arg(columnHeaders[yCol]));

    // Update chart title if empty or default
    if (titleEdit->text().isEmpty() || titleEdit->text().contains("数据可视化")) {
        titleEdit->setText(QString("%1 对比 %2")
                               .arg(columnHeaders[xCol])
                               .arg(columnHeaders[yCol]));
    }
}

void MainWindow::clearPlot()
{
    plotWidget->clearData();
    statsText->clear();
    statusLabel->setText("✅ 图表已清空");
    infoText->clear();
}

void MainWindow::onChartTypeChanged(int buttonId)
{
    ChartType type = static_cast<ChartType>(buttonId);
    plotWidget->setChartType(type);
    updateStatistics();
}

void MainWindow::updateChartTitle()
{
    plotWidget->setTitle(titleEdit->text());
}

void MainWindow::updateAxisLabels()
{
    plotWidget->setAxisLabels(xLabelEdit->text(), yLabelEdit->text());
}

void MainWindow::updateStatistics()
{
    // This will be implemented when we enhance the statistics functionality
    if (plotWidget && !plotWidget->property("isEmpty").toBool()) {
        QString statsInfo = "📈 统计总结:\n\n";
        // Add more detailed statistics here
        statsText->setPlainText(statsInfo);
    }
}

void MainWindow::openDeepSeekDialog()
{
    // 设置当前图表信息
    QString plotInfo = generateCurrentPlotInfo();
    deepseekDialog->setCurrentPlotInfo(plotInfo);

    // 显示对话框
    deepseekDialog->show();
    deepseekDialog->raise();
    deepseekDialog->activateWindow();
}

// DPI缩放辅助函数
int MainWindow::scaledSize(int baseSize) const {
    return static_cast<int>(baseSize * 1.5);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // 文件菜单
    QMenu *fileMenu = menuBar->addMenu("文件(&F)");
    
    // 打开文件
    QAction *openAction = new QAction("打开(&O)", this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("打开数据文件");
    connect(openAction, &QAction::triggered, this, &MainWindow::loadFile);
    fileMenu->addAction(openAction);
    
    fileMenu->addSeparator();
    
    // 导出配置
    QAction *exportConfigAction = new QAction("导出配置(&E)", this);
    exportConfigAction->setStatusTip("导出当前图表配置为JSON文件");
    connect(exportConfigAction, &QAction::triggered, this, &MainWindow::exportConfiguration);
    fileMenu->addAction(exportConfigAction);
    
    // 导入配置
    QAction *importConfigAction = new QAction("导入配置(&I)", this);
    importConfigAction->setStatusTip("从JSON文件导入图表配置");
    connect(importConfigAction, &QAction::triggered, this, &MainWindow::importConfiguration);
    fileMenu->addAction(importConfigAction);
    
    fileMenu->addSeparator();
    
    // 退出
    QAction *exitAction = new QAction("退出(&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("退出应用程序");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);
    
    // 视图菜单
    QMenu *viewMenu = menuBar->addMenu("视图(&V)");
    
    // 重置视图
    QAction *resetViewAction = new QAction("重置视图(&R)", this);
    resetViewAction->setShortcut(QKeySequence("Ctrl+0"));
    resetViewAction->setStatusTip("重置缩放和平移到默认视图");
    connect(resetViewAction, &QAction::triggered, plotWidget, &PlotWidget::resetView);
    viewMenu->addAction(resetViewAction);
    
    // 清空图表
    QAction *clearAction = new QAction("清空图表(&C)", this);
    clearAction->setShortcut(QKeySequence("Ctrl+D"));
    clearAction->setStatusTip("清空当前图表数据");
    connect(clearAction, &QAction::triggered, this, &MainWindow::clearPlot);
    viewMenu->addAction(clearAction);
    
    viewMenu->addSeparator();
    
    // AI助手
    QAction *aiDialogAction = new QAction("AI助手(&A)", this);
    aiDialogAction->setShortcut(QKeySequence("F2"));
    aiDialogAction->setStatusTip("打开DeepSeek AI助手对话");
    connect(aiDialogAction, &QAction::triggered, this, &MainWindow::openDeepSeekDialog);
    viewMenu->addAction(aiDialogAction);
    
    // 帮助菜单
    QMenu *helpMenu = menuBar->addMenu("帮助(&H)");
    
    // 关于
    QAction *aboutAction = new QAction("关于(&A)", this);
    aboutAction->setShortcut(QKeySequence("F1"));
    aboutAction->setStatusTip("关于此应用程序");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "关于 TXT数据绘图工具", 
            "TXT数据绘图工具 v2.1\n\n"
            "支持多种图表类型的数据可视化工具\n"
            "• 线图、柱状图、饼图、散点图\n"
            "• 直方图、箱线图、小提琴图等\n"
            "• 内置AI智能助手\n\n"
            "使用Qt 5.15.2构建");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Plot area on the left
    QWidget *plotArea = new QWidget();
    plotArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Right panel with scroll area for controls
    QScrollArea *rightScrollArea = new QScrollArea();
    rightScrollArea->setMinimumWidth(scaledSize(300)); // 减小最小宽度
    rightScrollArea->setMaximumWidth(scaledSize(350)); // 减小最大宽度
    rightScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    rightScrollArea->setWidgetResizable(true);
    rightScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rightScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    rightScrollArea->setFrameShape(QFrame::NoFrame);
    rightScrollArea->setStyleSheet("QScrollArea { background-color: #f8f9fa; border-left: 1px solid #dee2e6; } QScrollBar:vertical { background-color: #e9ecef; width: 12px; border-radius: 6px; } QScrollBar::handle:vertical { background-color: #6c757d; border-radius: 6px; min-height: 20px; } QScrollBar::handle:vertical:hover { background-color: #495057; } QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { border: none; background: none; } QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }");

    // 内部内容组件
    QWidget *rightPanel = new QWidget();
    rightPanel->setStyleSheet("QWidget { background-color: #f8f9fa; }");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(8); // 减少间距
    rightLayout->setContentsMargins(8, 8, 8, 8); // 减小边距
    rightLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);


    // Chart type selection group with combo box
    QGroupBox *chartGroup = new QGroupBox("图表类型");
    chartGroup->setStyleSheet("QGroupBox { font-weight: bold; padding-top: 10px; font-size: 11px; }");
    QVBoxLayout *chartLayout = new QVBoxLayout(chartGroup);
    chartLayout->setSpacing(6);
    chartLayout->setContentsMargins(8, 10, 8, 8);

    chartTypeCombo = new QComboBox();
    chartTypeCombo->addItem("📈 线图", 0);
    chartTypeCombo->addItem("📊 柱状图", 1);
    chartTypeCombo->addItem("🥧 饼图", 2);
    chartTypeCombo->addItem("🔸 散点图", 3);
    chartTypeCombo->addItem("📋 直方图", 4);
    chartTypeCombo->addItem("📦 箱线图", 5);
    chartTypeCombo->addItem("🎻 小提琴图", 6);
    chartTypeCombo->addItem("🌊 密度图", 7);
    chartTypeCombo->addItem("🏔️ 面积图", 8);

    chartTypeCombo->setCurrentIndex(0); // 默认选中线图
    chartTypeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    chartTypeCombo->setMinimumHeight(scaledSize(35));
    chartTypeCombo->setStyleSheet(QString("QComboBox { padding: 8px; border: 1px solid #ced4da; border-radius: 4px; background-color: white; font-size: %1px; } QComboBox:hover { border-color: #007bff; } QComboBox::drop-down { border: none; } QComboBox::down-arrow { image: none; border: none; }").arg(scaledSize(9)));

    chartLayout->addWidget(chartTypeCombo);
    chartLayout->addStretch();

    // Chart customization group
    QGroupBox *customGroup = new QGroupBox("图表定制");
    customGroup->setStyleSheet("QGroupBox { font-weight: bold; padding-top: 10px; font-size: 11px; }");
    QGridLayout *customLayout = new QGridLayout(customGroup);

    QLabel *titleLabel = new QLabel("图表标题:");
    titleLabel->setStyleSheet(QString("font-size: %1px;").arg(scaledSize(8)));
    customLayout->addWidget(titleLabel, 0, 0);
    titleEdit = new QLineEdit();
    titleEdit->setPlaceholderText("输入图表标题");
    titleEdit->setStyleSheet(QString("QLineEdit { padding: 6px; border: 1px solid #ced4da; border-radius: 4px; font-size: %1px; }").arg(scaledSize(9)));
    customLayout->addWidget(titleEdit, 0, 1);

    QLabel *xLabel = new QLabel("X轴标签:");
    xLabel->setStyleSheet(QString("font-size: %1px;").arg(scaledSize(8)));
    customLayout->addWidget(xLabel, 1, 0);
    xLabelEdit = new QLineEdit();
    xLabelEdit->setPlaceholderText("X轴标签");
    xLabelEdit->setStyleSheet(QString("QLineEdit { padding: 6px; border: 1px solid #ced4da; border-radius: 4px; font-size: %1px; }").arg(scaledSize(9)));
    customLayout->addWidget(xLabelEdit, 1, 1);

    QLabel *yLabel = new QLabel("Y轴标签:");
    yLabel->setStyleSheet(QString("font-size: %1px;").arg(scaledSize(8)));
    customLayout->addWidget(yLabel, 2, 0);
    yLabelEdit = new QLineEdit();
    yLabelEdit->setPlaceholderText("Y轴标签");
    yLabelEdit->setStyleSheet(QString("QLineEdit { padding: 6px; border: 1px solid #ced4da; border-radius: 4px; font-size: %1px; }").arg(scaledSize(9)));
    customLayout->addWidget(yLabelEdit, 2, 1);
    customLayout->setVerticalSpacing(6);
    customLayout->setHorizontalSpacing(4);
    customLayout->setContentsMargins(8, 10, 8, 8);


    // Column selection group
    columnGroup = new QGroupBox("列选择");
    columnGroup->setStyleSheet("QGroupBox { font-weight: bold; padding-top: 10px; font-size: 11px; }");
    columnGroup->setVisible(false); // Initially hidden
    QGridLayout *columnLayout = new QGridLayout(columnGroup);
    columnLayout->setVerticalSpacing(6);
    columnLayout->setHorizontalSpacing(4);
    columnLayout->setContentsMargins(8, 10, 8, 8);

    QLabel *xColLabel = new QLabel("X轴列:");
    xColLabel->setStyleSheet(QString("font-size: %1px;").arg(scaledSize(8)));
    columnLayout->addWidget(xColLabel, 0, 0);
    xColumnCombo = new QComboBox();
    xColumnCombo->setMinimumHeight(scaledSize(32));
    xColumnCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    xColumnCombo->setStyleSheet(QString("QComboBox { padding: 8px; border: 1px solid #ced4da; border-radius: 4px; font-size: %1px; }").arg(scaledSize(9)));
    columnLayout->addWidget(xColumnCombo, 0, 1);

    QLabel *yColLabel = new QLabel("Y轴列:");
    yColLabel->setStyleSheet(QString("font-size: %1px;").arg(scaledSize(8)));
    columnLayout->addWidget(yColLabel, 1, 0);
    yColumnCombo = new QComboBox();
    yColumnCombo->setMinimumHeight(scaledSize(32));
    yColumnCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    yColumnCombo->setStyleSheet(QString("QComboBox { padding: 8px; border: 1px solid #ced4da; border-radius: 4px; font-size: %1px; }").arg(scaledSize(9)));
    columnLayout->addWidget(yColumnCombo, 1, 1);

    applyColumnButton = new QPushButton("🔄 应用");
    applyColumnButton->setMinimumHeight(scaledSize(28));
    applyColumnButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    applyColumnButton->setStyleSheet(QString("QPushButton { padding: 6px 8px; font-size: %1px; background-color: #28a745; color: white; border: none; border-radius: 3px; } QPushButton:hover { background-color: #218838; }").arg(scaledSize(8)));
    applyColumnButton->setToolTip("应用选定的X和Y列进行绘图");

    // Add reset button
    QPushButton *resetColumnButton = new QPushButton("🔄 重置");
    resetColumnButton->setMinimumHeight(scaledSize(28));
    resetColumnButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    resetColumnButton->setStyleSheet(QString("QPushButton { padding: 6px 8px; font-size: %1px; background-color: #ffc107; color: black; border: none; border-radius: 3px; } QPushButton:hover { background-color: #e0a800; }").arg(scaledSize(8)));
    resetColumnButton->setToolTip("重置为默认列选择");
    
    // 将应用和重置按钮放在同一行
    columnLayout->addWidget(applyColumnButton, 2, 0);
    columnLayout->addWidget(resetColumnButton, 2, 1);

    connect(resetColumnButton, &QPushButton::clicked, [this]() {
        if (columnHeaders.size() >= 2) {
            xColumnCombo->setCurrentIndex(0); // Row Index (Virtual)
            yColumnCombo->setCurrentIndex(1); // First data column
        }
        applyColumnSelection();
    });

    columnInfoLabel = new QLabel("");
    columnInfoLabel->setStyleSheet(QString("QLabel { font-size: %1px; color: #6c757d; padding: 6px; }").arg(scaledSize(8)));
    columnLayout->addWidget(columnInfoLabel, 3, 0, 1, 2);

    // 多列显示选项
    multiColumnCheckbox = new QCheckBox("多列显示");
    multiColumnCheckbox->setStyleSheet(QString("QCheckBox { font-size: %1px; }").arg(scaledSize(8)));
    multiColumnCheckbox->setToolTip("勾选后可以选择多个Y轴列同时显示");
    multiColumnCheckbox->setVisible(false); // 初始隐藏
    columnLayout->addWidget(multiColumnCheckbox, 4, 0, 1, 2);

    // 多列选择滚动区域
    columnCheckboxArea = new QScrollArea();
    columnCheckboxArea->setMaximumHeight(scaledSize(120));
    columnCheckboxArea->setMinimumHeight(scaledSize(80));
    columnCheckboxArea->setWidgetResizable(true);
    columnCheckboxArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    columnCheckboxArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    columnCheckboxArea->setFrameShape(QFrame::StyledPanel);
    columnCheckboxArea->setStyleSheet(QString("QScrollArea { border: 1px solid #ced4da; border-radius: 4px; background-color: white; } QScrollBar:vertical { width: 12px; }"));
    columnCheckboxArea->setVisible(false); // 初始隐藏
    
    columnCheckboxWidget = new QWidget();
    columnCheckboxArea->setWidget(columnCheckboxWidget);
    columnLayout->addWidget(columnCheckboxArea, 5, 0, 1, 2);

    // AI对话面板 - 永远显示
    aiChatPanel = new QWidget();
    aiChatPanel->setVisible(true); // 永远显示
    QVBoxLayout *aiLayout = new QVBoxLayout(aiChatPanel);
    aiLayout->setContentsMargins(0, 0, 0, 0);

    // 创建AI对话界面组件
    createAIChatInterface(aiLayout);

    rightLayout->addWidget(columnGroup);
    rightLayout->addWidget(chartGroup);
    rightLayout->addWidget(customGroup);
    rightLayout->addWidget(aiChatPanel);
    rightLayout->addStretch();

    // 将内容组件设置到滚动区域中
    rightScrollArea->setWidget(rightPanel);

    // Plot area layout
    QVBoxLayout *plotAreaLayout = new QVBoxLayout(plotArea);
    plotAreaLayout->setContentsMargins(10, 10, 10, 10);

    // 视图控制提示 - 移到图表上方
    QLabel *viewHelpLabel = new QLabel("📜 视图控制：鼠标拖拽平移 | 滚轮缩放 | +/- 快捷键 | 0 重置视图");
    viewHelpLabel->setStyleSheet(QString("QLabel { font-size: %1px; color: #6c757d; padding: 6px; background-color: #e9ecef; border-radius: 3px; }").arg(scaledSize(8)));
    plotAreaLayout->addWidget(viewHelpLabel);

    // 绘图控件
    plotWidget = new PlotWidget();
    plotWidget->setStyleSheet("QWidget { background-color: white; border: 1px solid #dee2e6; border-radius: 8px; }");

    // 创建菜单栏（在plotWidget创建后）
    setupMenuBar();

    // 信息显示
    infoText = new QTextEdit();
    infoText->setMaximumHeight(scaledSize(80));
    infoText->setReadOnly(true);
    infoText->setStyleSheet(QString("QTextEdit { font-family: 'Microsoft YaHei', 'Consolas'; font-size: %1px; border: 1px solid #ced4da; border-radius: 4px; background-color: #f8f9fa; }").arg(scaledSize(8)));

    // 状态标签
    statusLabel = new QLabel("准备加载数据文件");
    statusLabel->setStyleSheet(QString("QLabel { padding: 8px; background-color: #e9ecef; border-radius: 4px; font-weight: bold; font-size: %1px; }").arg(scaledSize(9)));

    plotAreaLayout->addWidget(plotWidget, 1);
    
    // 创建水平布局来放置数据信息和统计信息
    QHBoxLayout *bottomInfoLayout = new QHBoxLayout();
    bottomInfoLayout->setSpacing(10);
    
    // 左侧：数据信息
    QWidget *dataInfoWidget = new QWidget();
    QVBoxLayout *dataInfoLayout = new QVBoxLayout(dataInfoWidget);
    dataInfoLayout->setContentsMargins(0, 0, 0, 0);
    dataInfoLayout->setSpacing(4);
    
    QLabel *dataInfoLabel = new QLabel("📊 数据信息:");
    dataInfoLabel->setStyleSheet(QString("font-size: %1px; font-weight: bold;").arg(scaledSize(9)));
    dataInfoLayout->addWidget(dataInfoLabel);
    dataInfoLayout->addWidget(infoText);
    
    // 右侧：统计信息（从右侧面板移过来）
    QWidget *statsInfoWidget = new QWidget();
    QVBoxLayout *statsInfoLayout = new QVBoxLayout(statsInfoWidget);
    statsInfoLayout->setContentsMargins(0, 0, 0, 0);
    statsInfoLayout->setSpacing(4);
    
    // 统计信息标题和拟合按钮水平布局
    QHBoxLayout *statsHeaderLayout = new QHBoxLayout();
    QLabel *statsInfoLabel = new QLabel("📈 统计信息:");
    statsInfoLabel->setStyleSheet(QString("font-size: %1px; font-weight: bold;").arg(scaledSize(9)));
    statsHeaderLayout->addWidget(statsInfoLabel);
    
    // 数据拟合下拉菜单和按钮
    QHBoxLayout *fittingLayout = new QHBoxLayout();
    fittingCombo = new QComboBox();
    fittingCombo->addItem("1次拟合 (线性)", 1);
    fittingCombo->addItem("2次拟合 (二次)", 2);
    fittingCombo->addItem("正弦拟合", 3);
    fittingCombo->addItem("高斯拟合", 4);
    fittingCombo->setMinimumHeight(scaledSize(24));
    fittingCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    fittingCombo->setStyleSheet(QString("QComboBox { padding: 4px; border: 1px solid #ced4da; border-radius: 3px; font-size: %1px; min-width: 80px; }").arg(scaledSize(8)));
    
    fittingButton = new QPushButton("数据拟合");
    fittingButton->setMinimumHeight(scaledSize(24));
    fittingButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    fittingButton->setStyleSheet(QString("QPushButton { padding: 4px 8px; font-size: %1px; background-color: #17a2b8; color: white; border: none; border-radius: 3px; } QPushButton:hover { background-color: #138496; }").arg(scaledSize(8)));
    fittingButton->setToolTip("对当前数据进行多项式拟合并显示残差图");
    
    fittingLayout->addWidget(fittingCombo);
    fittingLayout->addWidget(fittingButton);
    fittingLayout->addStretch();
    statsHeaderLayout->addLayout(fittingLayout);
    
    statsInfoLayout->addLayout(statsHeaderLayout);
    
    // 重新创建statsText，因为它原来在右侧面板
    statsText = new QTextEdit();
    statsText->setMinimumHeight(scaledSize(80));
    statsText->setMaximumHeight(scaledSize(80));
    statsText->setReadOnly(true);
    statsText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    statsText->setStyleSheet(QString("QTextEdit { font-family: 'Microsoft YaHei', 'Courier New'; font-size: %1px; border: 1px solid #ced4da; border-radius: 4px; background-color: white; }").arg(scaledSize(8)));
    statsInfoLayout->addWidget(statsText);
    
    // 添加到水平布局
    bottomInfoLayout->addWidget(dataInfoWidget, 1);
    bottomInfoLayout->addWidget(statsInfoWidget, 1);
    
    plotAreaLayout->addLayout(bottomInfoLayout);
    plotAreaLayout->addWidget(statusLabel);

    mainLayout->addWidget(plotArea, 1);
    mainLayout->addWidget(rightScrollArea);
}

void MainWindow::connectSignals()
{
    connect(chartTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onChartTypeChanged);

    connect(titleEdit, &QLineEdit::textChanged, this, &MainWindow::updateChartTitle);
    connect(xLabelEdit, &QLineEdit::textChanged, this, &MainWindow::updateAxisLabels);
    connect(yLabelEdit, &QLineEdit::textChanged, this, &MainWindow::updateAxisLabels);

    connect(applyColumnButton, &QPushButton::clicked, this, &MainWindow::applyColumnSelection);

    // Auto-apply when combo boxes change
    connect(xColumnCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyColumnSelection);
    connect(yColumnCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyColumnSelection);
    
    // Connect multi-column checkbox
    connect(multiColumnCheckbox, &QCheckBox::toggled, this, &MainWindow::onMultiColumnToggled);
    
    // Connect fitting button
    connect(fittingButton, &QPushButton::clicked, this, &MainWindow::performDataFitting);
}

std::vector<double> MainWindow::parseNumbersFromLine(const std::string& line) {
    std::vector<double> numbers;

    // Enhanced parsing with better tolerance for various formats
    std::string processedLine = line;

    // Replace common delimiters with spaces for unified processing
    std::replace(processedLine.begin(), processedLine.end(), '\t', ' ');
    std::replace(processedLine.begin(), processedLine.end(), ',', ' ');
    std::replace(processedLine.begin(), processedLine.end(), ';', ' ');
    std::replace(processedLine.begin(), processedLine.end(), '|', ' ');

    // Remove multiple spaces
    std::regex multipleSpaces("\\s+");
    processedLine = std::regex_replace(processedLine, multipleSpaces, " ");

    // Trim leading and trailing whitespace
    processedLine.erase(0, processedLine.find_first_not_of(" \n\r\t"));
    processedLine.erase(processedLine.find_last_not_of(" \n\r\t") + 1);

    if (processedLine.empty()) {
        return numbers;
    }

    // Split by spaces and parse numbers
    std::istringstream iss(processedLine);
    std::string token;

    while (iss >> token) {
        if (token.empty()) continue;

        // Clean the token - remove any remaining special characters
        token.erase(std::remove_if(token.begin(), token.end(),
                                   [](char c) { return c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}'; }),
                    token.end());

        if (token.empty()) continue;

        try {
            // Handle different number formats
            double value;
            if (token.find('e') != std::string::npos || token.find('E') != std::string::npos) {
                // Scientific notation
                value = std::stod(token);
            } else if (token.find('/') != std::string::npos) {
                // Fraction format (basic support)
                size_t slashPos = token.find('/');
                if (slashPos != std::string::npos && slashPos > 0 && slashPos < token.length() - 1) {
                    double numerator = std::stod(token.substr(0, slashPos));
                    double denominator = std::stod(token.substr(slashPos + 1));
                    if (denominator != 0) {
                        value = numerator / denominator;
                    } else {
                        continue; // Skip division by zero
                    }
                } else {
                    continue;
                }
            } else {
                // Regular number
                value = std::stod(token);
            }

            // Check for reasonable range (avoid extreme values that might be parsing errors)
            if (std::isfinite(value) && std::abs(value) < 1e15) {
                numbers.push_back(value);
            }
        } catch (const std::exception&) {
            // Skip invalid numbers but continue parsing
            continue;
        }
    }

    return numbers;
}

void MainWindow::loadDataFromFile(const QString& fileName)
{
    // Use Qt's file handling for better Unicode support
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件: " + fileName);
        return;
    }

    QTextStream stream(&file);
    // Set codec for proper Chinese character support
    stream.setCodec(QTextCodec::codecForName("UTF-8"));

    // Fallback to system locale if UTF-8 fails
    if (stream.codec() == nullptr) {
        stream.setCodec(QTextCodec::codecForLocale());
    }

    rawData.clear();
    columnHeaders.clear();

    QString line;
    int lineNumber = 0;
    int maxColumns = 0;

    // First pass: determine data structure
    while (!stream.atEnd() && lineNumber < 10000) {
        line = stream.readLine();
        lineNumber++;

        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith('#') || line.startsWith('%')) {
            continue;
        }

        std::vector<double> numbers = parseNumbersFromLine(line.toStdString());

        if (!numbers.empty()) {
            rawData.push_back(numbers);
            maxColumns = std::max(maxColumns, (int)numbers.size());
        }
    }

    file.close();

    if (rawData.empty()) {
        QMessageBox::warning(this, "警告", "文件中未找到有效数据");
        return;
    }

    // Ensure all rows have the same number of columns (pad with zeros if necessary)
    for (auto& row : rawData) {
        while ((int)row.size() < maxColumns) {
            row.push_back(0.0);
        }
    }

    hasMultipleColumns = maxColumns > 1;

    // Generate column headers
    columnHeaders.clear();
    columnHeaders.append(QString("行索引(虚拟)")); // Add virtual row index column
    for (int i = 0; i < maxColumns; ++i) {
        columnHeaders.append(QString("第%1列").arg(i + 1));
    }

    // Update column selection UI
    updateColumnSelectionUI();

    // Default plotting behavior with improved logic
    std::vector<double> xData, yData;

    if (maxColumns == 1) {
        // Single column: use index as X, data as Y
        for (size_t i = 0; i < rawData.size(); ++i) {
            xData.push_back(static_cast<double>(i + 1));
            yData.push_back(rawData[i][0]);
        }
        // Switch to scatter plot for single column by default
        chartTypeCombo->setCurrentIndex(3);
        plotWidget->setChartType(ChartType::Scatter);
    } else if (maxColumns == 2) {
        // Two columns: use first as X, second as Y (ideal case)
        for (const auto& row : rawData) {
            xData.push_back(row[0]);
            yData.push_back(row[1]);
        }
        // Switch to scatter plot for two columns by default
        chartTypeCombo->setCurrentIndex(3);
        plotWidget->setChartType(ChartType::Scatter);
    } else {
        // Multiple columns: default to first two columns
        for (const auto& row : rawData) {
            xData.push_back(row[0]);
            yData.push_back(row[1]);
        }
    }

    // Update plot using the improved default behavior
    plotWidget->setData(xData, yData);

    // Auto-apply column selection after loading
    if (maxColumns >= 1) {
        QTimer::singleShot(100, this, &MainWindow::applyColumnSelection);
    }

    // Show data info
    QString info = QString("📁 文件: %1\n").arg(QFileInfo(fileName).fileName());
    info += QString("📊 数据点: %1\n").arg(rawData.size());
    info += QString("📋 列数: %1\n").arg(maxColumns);

    if (!xData.empty()) {
        auto xMinMax = std::minmax_element(xData.begin(), xData.end());
        auto yMinMax = std::minmax_element(yData.begin(), yData.end());
        info += QString("📏 X范围: [%.3f, %.3f]\n").arg(*xMinMax.first).arg(*xMinMax.second);
        info += QString("📐 Y范围: [%.3f, %.3f]").arg(*yMinMax.first).arg(*yMinMax.second);
    }

    infoText->setPlainText(info);
    statusLabel->setText(QString("✅ 成功加载 %1 个数据点，共 %2 列")
                             .arg(rawData.size()).arg(maxColumns));

    // Update statistics
    updateDetailedStatistics(yData);

    // Set default labels
    if (titleEdit->text().isEmpty()) {
        titleEdit->setText(QFileInfo(fileName).baseName() + " - 数据可视化");
    }
    // Set proper axis labels based on actual column selection
    if (maxColumns == 1) {
        xLabelEdit->setText("行索引");
        yLabelEdit->setText("数值");
    } else if (maxColumns >= 2) {
        xLabelEdit->setText(QString("第1列"));
        yLabelEdit->setText(QString("第2列"));
    }
}

void MainWindow::updateColumnSelectionUI()
{
    if (rawData.empty()) {
        columnGroup->setVisible(false);
        return;
    }

    // Reset multi-column state when loading new data
    multiColumnCheckbox->setChecked(false);
    columnCheckboxArea->setVisible(false);
    yColumnCombo->setVisible(true);
    
    // Clear PlotWidget's multi-series state
    plotWidget->clearData();

    // Clear and populate combo boxes
    xColumnCombo->clear();
    yColumnCombo->clear();

    for (const QString& header : columnHeaders) {
        xColumnCombo->addItem(header);
        yColumnCombo->addItem(header);
    }

    // Set default selections and always show column selection
    columnGroup->setVisible(true);

    if (columnHeaders.size() >= 3) { // Row Index + at least 2 data columns
        xColumnCombo->setCurrentIndex(0); // Default to Row Index (Virtual)
        yColumnCombo->setCurrentIndex(1); // Default to first data column

        // 显示多列选项
        multiColumnCheckbox->setVisible(true);
        setupMultiColumnCheckboxes();

        columnInfoLabel->setText(QString("文件包含 %1 个数据列 + 虚拟行索引。默认: %2 (X) vs %3 (Y)。")
                                     .arg(columnHeaders.size() - 1)
                                     .arg(columnHeaders[0])
                                     .arg(columnHeaders[1]));
    } else if (columnHeaders.size() == 2) { // Row Index + 1 data column
        xColumnCombo->setCurrentIndex(0); // Row Index (Virtual)
        yColumnCombo->setCurrentIndex(1); // Single data column

        // 隐藏多列选项（只有一列数据）
        multiColumnCheckbox->setVisible(false);
        columnCheckboxArea->setVisible(false);

        columnInfoLabel->setText(QString("检测到单个数据列: %1。默认X轴使用行索引。")
                                     .arg(columnHeaders[1]));
    } else {
        columnGroup->setVisible(false);
    }
}

void MainWindow::createAIChatInterface(QVBoxLayout *layout)
{
    // 标题
    QLabel *aiTitle = new QLabel("🤖 智能助手 (快速+DeepSeek)");
    aiTitle->setStyleSheet(QString("QLabel { font-weight: bold; font-size: %1px; padding: 8px; background-color: #6f42c1; color: white; border-radius: 4px; }").arg(scaledSize(10)));
    layout->addWidget(aiTitle);

    // 对话区域
    aiChatArea = new QTextBrowser();
    aiChatArea->setMinimumHeight(scaledSize(120));
    aiChatArea->setMaximumHeight(scaledSize(200));
    aiChatArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    aiChatArea->setStyleSheet(QString("QTextBrowser { border: 1px solid #dee2e6; border-radius: 4px; background-color: #fafafa; font-size: %1px; padding: 8px; }").arg(scaledSize(8)));
    layout->addWidget(aiChatArea);

    // 输入区域
    QHBoxLayout *inputLayout = new QHBoxLayout();
    aiInputEdit = new QLineEdit();
    aiInputEdit->setPlaceholderText("输入命令，自动选择快速或DeepSeek处理...");
    aiInputEdit->setStyleSheet(QString("QLineEdit { padding: 8px; border: 1px solid #ced4da; border-radius: 4px; font-size: %1px; }").arg(scaledSize(9)));

    aiSendButton = new QPushButton("发送");

    // 添加配置按钮
    QHBoxLayout *configLayout = new QHBoxLayout();
    QPushButton *exportConfigBtn = new QPushButton("📤");
    QPushButton *importConfigBtn = new QPushButton("📥");
    exportConfigBtn->setStyleSheet(QString("QPushButton { padding: 6px 8px; background-color: #28a745; color: white; border: none; border-radius: 3px; font-size: %1px; min-width: 32px; } QPushButton:hover { background-color: #218838; }").arg(scaledSize(7)));
    importConfigBtn->setStyleSheet(QString("QPushButton { padding: 6px 8px; background-color: #ffc107; color: black; border: none; border-radius: 3px; font-size: %1px; min-width: 32px; } QPushButton:hover { background-color: #e0a800; }").arg(scaledSize(7)));
    exportConfigBtn->setToolTip("导出JSON配置");
    importConfigBtn->setToolTip("导入JSON配置");

    configLayout->addWidget(exportConfigBtn);
    configLayout->addWidget(importConfigBtn);
    configLayout->addStretch(); // 添加弹性空间
    layout->addLayout(configLayout);

    connect(exportConfigBtn, &QPushButton::clicked, this, &MainWindow::exportConfiguration);
    connect(importConfigBtn, &QPushButton::clicked, this, &MainWindow::importConfiguration);
    aiSendButton->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #007bff; color: white; border: none; border-radius: 4px; } QPushButton:hover { background-color: #0056b3; }");

    inputLayout->addWidget(aiInputEdit);
    inputLayout->addWidget(aiSendButton);
    layout->addLayout(inputLayout);

    // 显示初始快速命令链接
    showQuickCommandLinks();

    // 连接信号
    connect(aiSendButton, &QPushButton::clicked, this, &MainWindow::sendAIMessage);
    connect(aiInputEdit, &QLineEdit::returnPressed, this, &MainWindow::sendAIMessage);
    
    // 处理链接点击事件
    connect(aiChatArea, &QTextBrowser::anchorClicked, [this](const QUrl &url) {
        QString command = url.toString();
        if (command.startsWith("cmd:")) {
            QString cmd = command.mid(4); // 移除 "cmd:" 前缀
            aiInputEdit->setText(cmd);
            aiInputEdit->setFocus();
            // 可选：自动发送命令
            // sendAIMessage();
        }
    });
}

void MainWindow::setupDeepSeekDialog()
{
    deepseekDialog = new DeepSeekDialog(this);
    connect(deepseekDialog, &DeepSeekDialog::configurationGenerated,
            this, &MainWindow::applyConfiguration);

    // 初始化AI网络管理器
    aiNetworkManager = new QNetworkAccessManager(this);
    aiApiKey = "sk-824b0dea3d674ebf980c1ac28d9fa0b6";
}

// 移除了toggleAIPanel方法，AI面板现在永远显示

void MainWindow::sendAIMessage()
{
    QString message = aiInputEdit->text().trimmed();
    if (message.isEmpty()) return;

    // 显示用户消息
    aiChatArea->append(QString("\n👤 <b>您</b>: %1").arg(message));
    aiInputEdit->clear();

    // 简单的AI响应逻辑（模拟）
    processAIRequest(message);
}

void MainWindow::processAIRequest(const QString& request)
{
    QString response = "🤖 <b>快速助手</b>: ";
    QString lowerRequest = request.toLower();
    bool handled = false;

    // 图表类型修改 - 需要更具体的关键词组合
    if ((lowerRequest.contains("改为") || lowerRequest.contains("切换") || lowerRequest.contains("使用") || lowerRequest.contains("设置")) && lowerRequest.contains("柱状图")) {
        chartTypeCombo->setCurrentIndex(1);
        response += "已将图表类型改为柱状图。";
        handled = true;
    } else if ((lowerRequest.contains("改为") || lowerRequest.contains("切换") || lowerRequest.contains("使用") || lowerRequest.contains("设置")) && lowerRequest.contains("线图")) {
        chartTypeCombo->setCurrentIndex(0);
        response += "已将图表类型改为线图。";
        handled = true;
    } else if ((lowerRequest.contains("改为") || lowerRequest.contains("切换") || lowerRequest.contains("使用") || lowerRequest.contains("设置")) && lowerRequest.contains("饼图")) {
        chartTypeCombo->setCurrentIndex(2);
        response += "已将图表类型改为饼图。";
        handled = true;
    } else if ((lowerRequest.contains("改为") || lowerRequest.contains("切换") || lowerRequest.contains("使用") || lowerRequest.contains("设置")) && lowerRequest.contains("散点图")) {
        chartTypeCombo->setCurrentIndex(3);
        response += "已将图表类型改为散点图。";
        handled = true;
    } else if ((lowerRequest.contains("改为") || lowerRequest.contains("切换") || lowerRequest.contains("使用") || lowerRequest.contains("设置")) && lowerRequest.contains("直方图")) {
        chartTypeCombo->setCurrentIndex(4);
        response += "已将图表类型改为直方图。";
        handled = true;
    } else if ((lowerRequest.contains("改为") || lowerRequest.contains("切换") || lowerRequest.contains("使用") || lowerRequest.contains("设置")) && lowerRequest.contains("箱线图")) {
        chartTypeCombo->setCurrentIndex(5);
        response += "已将图表类型改为箱线图。";
        handled = true;
    } else if ((lowerRequest.contains("改为") || lowerRequest.contains("切换") || lowerRequest.contains("使用") || lowerRequest.contains("设置")) && lowerRequest.contains("小提琴图")) {
        chartTypeCombo->setCurrentIndex(6);
        response += "已将图表类型改为小提琴图。";
        handled = true;
    } else if ((lowerRequest.contains("改为") || lowerRequest.contains("切换") || lowerRequest.contains("使用") || lowerRequest.contains("设置")) && lowerRequest.contains("密度图")) {
        chartTypeCombo->setCurrentIndex(7);
        response += "已将图表类型改为密度图。";
        handled = true;
    } else if ((lowerRequest.contains("改为") || lowerRequest.contains("切换") || lowerRequest.contains("使用") || lowerRequest.contains("设置")) && lowerRequest.contains("面积图")) {
        chartTypeCombo->setCurrentIndex(8);
        response += "已将图表类型改为面积图。";
        handled = true;
    }
    // 标题修改 - 需要包含动作词
    else if ((lowerRequest.contains("修改标题") || lowerRequest.contains("设置标题") || lowerRequest.contains("改标题") || lowerRequest.contains("标题改为") || lowerRequest.contains("标题设为"))) {
        QRegExp rx("标题.{0,10}[为改成设是]?['\"]?([^'\"\n]{1,50})['\"]?");
        if (rx.indexIn(request) != -1) {
            QString title = rx.cap(1).trimmed();
            if (!title.isEmpty()) {
                titleEdit->setText(title);
                updateChartTitle();
                response += QString("已将标题设置为: %1").arg(title);
                handled = true;
            } else {
                response += "请提供有效的标题内容。";
                handled = true;
            }
        } else {
            response += "请告诉我您希望设置的标题内容，例如：\"修改标题为销售数据分析\"。";
            handled = true;
        }
    }
    // X轴标签修改 - 需要包含动作词
    else if ((lowerRequest.contains("修改x轴") || lowerRequest.contains("设置x轴") || lowerRequest.contains("x轴标签") || lowerRequest.contains("修改横轴") || lowerRequest.contains("设置横轴") || lowerRequest.contains("横轴标签"))) {
        QRegExp rx("[xX横]轴.{0,15}[为改成设是标签]?['\"]?([^'\"\n]{1,30})['\"]?");
        if (rx.indexIn(request) != -1) {
            QString label = rx.cap(1).trimmed();
            if (!label.isEmpty()) {
                xLabelEdit->setText(label);
                updateAxisLabels();
                response += QString("已将X轴标签设置为: %1").arg(label);
                handled = true;
            }
        } else {
            response += "请指定X轴标签内容，例如：\"设置X轴标签为时间\"。";
            handled = true;
        }
    }
    // Y轴标签修改 - 需要包含动作词
    else if ((lowerRequest.contains("修改y轴") || lowerRequest.contains("设置y轴") || lowerRequest.contains("y轴标签") || lowerRequest.contains("修改纵轴") || lowerRequest.contains("设置纵轴") || lowerRequest.contains("纵轴标签"))) {
        QRegExp rx("[yY纵]轴.{0,15}[为改成设是标签]?['\"]?([^'\"\n]{1,30})['\"]?");
        if (rx.indexIn(request) != -1) {
            QString label = rx.cap(1).trimmed();
            if (!label.isEmpty()) {
                yLabelEdit->setText(label);
                updateAxisLabels();
                response += QString("已将Y轴标签设置为: %1").arg(label);
                handled = true;
            }
        } else {
            response += "请指定Y轴标签内容，例如：\"设置Y轴标签为数量\"。";
            handled = true;
        }
    }
    // 网格显示控制 - 需要包含动作词
    else if ((lowerRequest.contains("隐藏网格") || lowerRequest.contains("关闭网格") || lowerRequest.contains("不显示网格"))) {
        plotWidget->setGridVisible(false);
        response += "已隐藏网格。";
        handled = true;
    } else if ((lowerRequest.contains("显示网格") || lowerRequest.contains("开启网格") || lowerRequest.contains("打开网格"))) {
        plotWidget->setGridVisible(true);
        response += "已显示网格。";
        handled = true;
    }
    // 线条宽度修改 - 需要更具体的关键词
    else if ((lowerRequest.contains("线条宽度") || lowerRequest.contains("设置线条") || lowerRequest.contains("修改线条") || lowerRequest.contains("线条粗细"))) {
        QRegExp rx("线条.{0,15}(\\d+)");
        if (rx.indexIn(request) != -1) {
            int width = rx.cap(1).toInt();
            if (width >= 1 && width <= 10) {
                plotWidget->setLineWidth(width);
                response += QString("已将线条宽度设置为: %1").arg(width);
                handled = true;
            } else {
                response += "线条宽度应在1-10之间。";
                handled = true;
            }
        } else if (lowerRequest.contains("粗线") || lowerRequest.contains("加粗线条")) {
            plotWidget->setLineWidth(5);
            response += "已将线条设置为粗线。";
            handled = true;
        } else if (lowerRequest.contains("细线") || lowerRequest.contains("变细线条")) {
            plotWidget->setLineWidth(1);
            response += "已将线条设置为细线。";
            handled = true;
        } else {
            response += "请指定线条粗细，例如：\"设置线条宽度为3\"或\"使用粗线\"。";
            handled = true;
        }
    }
    // 点大小修改 - 需要更具体的关键词
    else if ((lowerRequest.contains("点大小") || lowerRequest.contains("设置点") || lowerRequest.contains("修改点") || lowerRequest.contains("点的大小"))) {
        QRegExp rx("点.{0,15}(\\d+)");
        if (rx.indexIn(request) != -1) {
            int size = rx.cap(1).toInt();
            if (size >= 2 && size <= 15) {
                plotWidget->setPointSize(size);
                response += QString("已将点大小设置为: %1").arg(size);
                handled = true;
            } else {
                response += "点大小应在2-15之间。";
                handled = true;
            }
        } else if (lowerRequest.contains("大点") || lowerRequest.contains("增大点")) {
            plotWidget->setPointSize(8);
            response += "已将点设置为大尺寸。";
            handled = true;
        } else if (lowerRequest.contains("小点") || lowerRequest.contains("缩小点")) {
            plotWidget->setPointSize(3);
            response += "已将点设置为小尺寸。";
            handled = true;
        } else {
            response += "请指定点的大小，例如：\"设置点大小为5\"或\"使用大点\"。";
            handled = true;
        }
    }

    // 如果快速命令未处理，则使用DeepSeek
    if (!handled) {
        aiChatArea->append("\n" + response + "快速命令无法处理，正在使用DeepSeek AI...");
        fallbackToDeepSeek(request);
        return;
    }

    aiChatArea->append("\n" + response);
    
    // 重新显示快速命令链接
    showQuickCommandLinks();
    
    aiChatArea->verticalScrollBar()->setValue(aiChatArea->verticalScrollBar()->maximum());
}

void MainWindow::fallbackToDeepSeek(const QString& request)
{
    // 设置当前图表信息
    QString plotInfo = generateCurrentPlotInfo();
    deepseekDialog->setCurrentPlotInfo(plotInfo);

    // 创建一个临时的网络管理器来发送请求
    QNetworkAccessManager *tempManager = new QNetworkAccessManager(this);

    // 准备DeepSeek API请求
    QNetworkRequest networkRequest(QUrl("https://api.deepseek.com/chat/completions"));
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, "TxtPlotter/1.0");
    networkRequest.setRawHeader("Authorization", QString("Bearer %1").arg(aiApiKey).toUtf8());
    networkRequest.setRawHeader("Accept", "application/json");

    // SSL配置
    QSslConfiguration sslConfig = networkRequest.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    networkRequest.setSslConfiguration(sslConfig);

    // 准备消息数组
    QJsonArray messages;

    // 系统消息
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    QString systemPrompt = QString::fromUtf8(R"(
你是一个专业的数据可视化助手。用户会用中文描述他们想要的图表修改需求，你需要生成对应的JSON配置。

重要规则：
1. 你必须严格按照以下JSON格式输出，不要添加任何解释文字
2. 只输出JSON，不要用markdown代码块包装，不要加```json```标记
3. 所有字符串值必须用双引号
4. 布尔值只能是true或false（小写）
5. 数字不要加引号
6. 理解中文指令并正确映射到配置

标准JSON格式：
{
  "chart": {
    "type": "图表类型",
    "title": "图表标题",
    "x_label": "X轴标签",
    "y_label": "Y轴标签"
  },
  "data": {
    "file": "数据文件名",
    "x_column": 0,
    "y_column": 1
  },
  "appearance": {
    "colors": {
      "background": "#ffffff",
      "grid": "#e9ecef",
      "axis": "#343a40",
      "text": "#212529",
      "primary": "#007bff"
    },
    "font": {
      "title_size": 16,
      "axis_size": 12,
      "label_size": 10
    },
    "style": {
      "show_grid": true,
      "line_width": 2,
      "point_size": 5
    }
  }
}

当前图表状态：
%1
        )").arg(plotInfo);
    systemMessage["content"] = systemPrompt;
    messages.append(systemMessage);

    // 用户消息
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = request;
    messages.append(userMsg);

    // 请求体
    QJsonObject requestBody;
    requestBody["model"] = "deepseek-chat";
    requestBody["messages"] = messages;
    requestBody["temperature"] = 0.1;
    requestBody["max_tokens"] = 1000;
    requestBody["stream"] = false;

    QJsonDocument doc(requestBody);

    QNetworkReply *reply = tempManager->post(networkRequest, doc.toJson());

    // 处理响应
    connect(reply, &QNetworkReply::finished, [this, reply, tempManager, request]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument response = QJsonDocument::fromJson(data);
            QJsonObject responseObj = response.object();

            if (responseObj.contains("choices")) {
                QJsonArray choices = responseObj["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject choice = choices[0].toObject();
                    QJsonObject message = choice["message"].toObject();
                    QString content = message["content"].toString().trimmed();

                    aiChatArea->append(QString("\n🤖 <b>DeepSeek AI</b>: %1").arg(content));

                    // 尝试解析并应用配置
                    parseAndApplyDeepSeekConfig(content);
                }
            } else if (responseObj.contains("error")) {
                QJsonObject error = responseObj["error"].toObject();
                QString errorMsg = error["message"].toString();
                aiChatArea->append(QString("\n❌ <b>DeepSeek错误</b>: %1").arg(errorMsg));
            } else {
                aiChatArea->append("\n❌ <b>系统</b>: DeepSeek响应格式未知");
            }
        } else {
            aiChatArea->append(QString("\n❌ <b>网络错误</b>: %1").arg(reply->errorString()));
            aiChatArea->append("\n💡 <b>提示</b>: DeepSeek连接失败，请检查网络或使用手动JSON配置。");
        }

        aiChatArea->verticalScrollBar()->setValue(aiChatArea->verticalScrollBar()->maximum());
        reply->deleteLater();
        tempManager->deleteLater();
    });
}

void MainWindow::parseAndApplyDeepSeekConfig(const QString& jsonString)
{
    // 清理JSON字符串
    QString cleanJson = jsonString;
    cleanJson = cleanJson.replace(QRegExp("```json\\s*"), "");
    cleanJson = cleanJson.replace(QRegExp("```\\s*"), "");
    cleanJson = cleanJson.trimmed();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(cleanJson.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        aiChatArea->append(QString("\n❌ <b>JSON解析失败</b>: %1 (位置: %2)")
                               .arg(error.errorString()).arg(error.offset));
        return;
    }

    QJsonObject root = doc.object();
    PlotConfig config;

    try {
        // 解析图表设置
        if (root.contains("chart")) {
            QJsonObject chart = root["chart"].toObject();
            config.chartType = chart["type"].toString("line");
            config.title = chart["title"].toString();
            config.xLabel = chart["x_label"].toString();
            config.yLabel = chart["y_label"].toString();
        }

        // 解析数据设置
        if (root.contains("data")) {
            QJsonObject data = root["data"].toObject();
            config.xColumn = data["x_column"].toInt(0);
            config.yColumn = data["y_column"].toInt(1);
        }

        // 解析外观设置
        if (root.contains("appearance")) {
            QJsonObject appearance = root["appearance"].toObject();

            if (appearance.contains("colors")) {
                QJsonObject colors = appearance["colors"].toObject();
                config.backgroundColor = colors["background"].toString("#f8f9fa");
                config.gridColor = colors["grid"].toString("#e9ecef");
                config.axisColor = colors["axis"].toString("#343a40");
                config.textColor = colors["text"].toString("#212529");
                config.primaryColor = colors["primary"].toString("#007bff");
            }

            if (appearance.contains("font")) {
                QJsonObject font = appearance["font"].toObject();
                config.titleSize = font["title_size"].toInt(16);
                config.axisSize = font["axis_size"].toInt(12);
                config.labelSize = font["label_size"].toInt(10);
            }

            if (appearance.contains("style")) {
                QJsonObject style = appearance["style"].toObject();
                config.showGrid = style["show_grid"].toBool(true);
                config.lineWidth = style["line_width"].toInt(2);
                config.pointSize = style["point_size"].toInt(5);
            }
        }

        // 应用配置
        applyConfiguration(config);
        aiChatArea->append("\n✅ <b>系统</b>: DeepSeek配置已成功应用！");

    } catch (...) {
        aiChatArea->append("\n❌ <b>配置应用失败</b>: 请检查JSON格式是否正确。");
    }
}

void MainWindow::loadConfigurationFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法打开配置文件: " + filePath);
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "JSON解析错误", error.errorString());
        return;
    }

    QJsonObject root = doc.object();
    PlotConfig config;

    // Parse configuration
    if (root.contains("chart")) {
        QJsonObject chart = root["chart"].toObject();
        config.chartType = chart["type"].toString("line");
        config.title = chart["title"].toString();
        config.xLabel = chart["x_label"].toString();
        config.yLabel = chart["y_label"].toString();
    }

    if (root.contains("data")) {
        QJsonObject data = root["data"].toObject();
        config.xColumn = data["x_column"].toInt(0);
        config.yColumn = data["y_column"].toInt(1);
    }

    if (root.contains("appearance")) {
        QJsonObject appearance = root["appearance"].toObject();

        if (appearance.contains("colors")) {
            QJsonObject colors = appearance["colors"].toObject();
            config.backgroundColor = colors["background"].toString("#f8f9fa");
            config.gridColor = colors["grid"].toString("#e9ecef");
            config.axisColor = colors["axis"].toString("#343a40");
            config.textColor = colors["text"].toString("#212529");
            config.primaryColor = colors["primary"].toString("#007bff");
        }

        if (appearance.contains("font")) {
            QJsonObject font = appearance["font"].toObject();
            config.titleSize = font["title_size"].toInt(16);
            config.axisSize = font["axis_size"].toInt(12);
            config.labelSize = font["label_size"].toInt(10);
        }

        if (appearance.contains("style")) {
            QJsonObject style = appearance["style"].toObject();
            config.showGrid = style["show_grid"].toBool(true);
            config.lineWidth = style["line_width"].toInt(2);
            config.pointSize = style["point_size"].toInt(5);
        }
    }

    // Apply configuration
    applyConfiguration(config);
}

QString MainWindow::generateCurrentPlotInfo()
{
    QString info = "当前图表配置信息：\n";
    info += QString("- 图表类型：%1\n").arg(getCurrentChartTypeName());
    info += QString("- 标题：%1\n").arg(titleEdit->text().isEmpty() ? "未设置" : titleEdit->text());
    info += QString("- X轴标签：%1\n").arg(xLabelEdit->text().isEmpty() ? "未设置" : xLabelEdit->text());
    info += QString("- Y轴标签：%1\n").arg(yLabelEdit->text().isEmpty() ? "未设置" : yLabelEdit->text());

    if (!rawData.empty()) {
        info += QString("- 数据行数：%1\n").arg(rawData.size());
        info += QString("- 数据列数：%1\n").arg(rawData[0].size());
        info += QString("- X轴列：%1\n").arg(xColumnCombo->currentText());
        info += QString("- Y轴列：%1").arg(yColumnCombo->currentText());
    } else {
        info += "- 当前无数据";
    }

    return info;
}

QString MainWindow::getCurrentChartTypeName()
{
    int currentIndex = chartTypeCombo->currentIndex();
    switch (currentIndex) {
    case 0: return "线图";
    case 1: return "柱状图";
    case 2: return "饼图";
    case 3: return "散点图";
    case 4: return "直方图";
    case 5: return "箱线图";
    case 6: return "小提琴图";
    case 7: return "密度图";
    case 8: return "面积图";
    default: return "未知";
    }
}

void MainWindow::applyConfiguration(const PlotConfig& config)
{
    // Apply chart type
    if (config.chartType == "line") chartTypeCombo->setCurrentIndex(0);
    else if (config.chartType == "bar") chartTypeCombo->setCurrentIndex(1);
    else if (config.chartType == "pie") chartTypeCombo->setCurrentIndex(2);
    else if (config.chartType == "scatter") chartTypeCombo->setCurrentIndex(3);
    else if (config.chartType == "histogram") chartTypeCombo->setCurrentIndex(4);
    else if (config.chartType == "box") chartTypeCombo->setCurrentIndex(5);
    else if (config.chartType == "violin") chartTypeCombo->setCurrentIndex(6);
    else if (config.chartType == "density") chartTypeCombo->setCurrentIndex(7);
    else if (config.chartType == "area") chartTypeCombo->setCurrentIndex(8);

    // Apply labels
    if (!config.title.isEmpty()) {
        titleEdit->setText(config.title);
    }
    if (!config.xLabel.isEmpty()) {
        xLabelEdit->setText(config.xLabel);
    }
    if (!config.yLabel.isEmpty()) {
        yLabelEdit->setText(config.yLabel);
    }

    // Apply column selection if valid
    if (config.xColumn >= 0 && config.xColumn < xColumnCombo->count()) {
        xColumnCombo->setCurrentIndex(config.xColumn);
    }
    if (config.yColumn >= 0 && config.yColumn < yColumnCombo->count()) {
        yColumnCombo->setCurrentIndex(config.yColumn);
    }

    // Apply colors and styles to PlotWidget
    applyColorsToPlotWidget(config);

    // Trigger chart update
    onChartTypeChanged(getCheckedChartTypeId());
    applyColumnSelection();

    statusLabel->setText("✅ AI配置已成功应用！");
}

void MainWindow::exportConfiguration()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出配置文件", "", "JSON文件 (*.json)");
    if (fileName.isEmpty()) return;

    QJsonObject config;

    // 图表信息
    QJsonObject chart;
    chart["type"] = getCurrentChartTypeString();
    chart["title"] = titleEdit->text();
    chart["x_label"] = xLabelEdit->text();
    chart["y_label"] = yLabelEdit->text();
    config["chart"] = chart;

    // 数据信息
    QJsonObject data;
    data["x_column"] = xColumnCombo->currentIndex();
    data["y_column"] = yColumnCombo->currentIndex();
    config["data"] = data;

    // 外观设置
    QJsonObject appearance;
    QJsonObject colors;
    colors["background"] = plotWidget->property("backgroundColor").toString();
    colors["grid"] = plotWidget->property("gridColor").toString();
    colors["axis"] = plotWidget->property("axisColor").toString();
    colors["text"] = plotWidget->property("textColor").toString();
    appearance["colors"] = colors;

    QJsonObject style;
    style["show_grid"] = plotWidget->property("showGrid").toBool();
    style["line_width"] = plotWidget->property("lineWidth").toInt();
    style["point_size"] = plotWidget->property("pointSize").toInt();
    appearance["style"] = style;

    config["appearance"] = appearance;
    config["export_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    config["version"] = "2.0";

    QJsonDocument doc(config);
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        aiChatArea->append(QString("\n💾 <b>系统</b>: 配置已成功导出到 %1").arg(QFileInfo(fileName).fileName()));
    } else {
        QMessageBox::warning(this, "错误", "无法写入配置文件");
    }
}

void MainWindow::importConfiguration()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入配置文件", "", "JSON文件 (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法读取配置文件");
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "JSON解析错误", error.errorString());
        return;
    }

    QJsonObject config = doc.object();

    // 应用配置
    if (config.contains("chart")) {
        QJsonObject chart = config["chart"].toObject();
        if (chart.contains("type")) {
            QString type = chart["type"].toString();
            setChartTypeByString(type);
        }
        if (chart.contains("title")) {
            titleEdit->setText(chart["title"].toString());
            updateChartTitle();
        }
        if (chart.contains("x_label")) {
            xLabelEdit->setText(chart["x_label"].toString());
            updateAxisLabels();
        }
        if (chart.contains("y_label")) {
            yLabelEdit->setText(chart["y_label"].toString());
            updateAxisLabels();
        }
    }

    if (config.contains("data")) {
        QJsonObject data = config["data"].toObject();
        if (data.contains("x_column") && data["x_column"].toInt() < xColumnCombo->count()) {
            xColumnCombo->setCurrentIndex(data["x_column"].toInt());
        }
        if (data.contains("y_column") && data["y_column"].toInt() < yColumnCombo->count()) {
            yColumnCombo->setCurrentIndex(data["y_column"].toInt());
        }
        applyColumnSelection();
    }

    aiChatArea->append(QString("\n📂 <b>系统</b>: 配置已成功从 %1 导入").arg(QFileInfo(fileName).fileName()));
    statusLabel->setText("✅ JSON配置已成功导入！");
}

QString MainWindow::getCurrentChartTypeString()
{
    int currentIndex = chartTypeCombo->currentIndex();
    switch (currentIndex) {
    case 0: return "line";
    case 1: return "bar";
    case 2: return "pie";
    case 3: return "scatter";
    case 4: return "histogram";
    case 5: return "box";
    case 6: return "violin";
    case 7: return "density";
    case 8: return "area";
    default: return "line";
    }
}

void MainWindow::setChartTypeByString(const QString& type)
{
    if (type == "line") chartTypeCombo->setCurrentIndex(0);
    else if (type == "bar") chartTypeCombo->setCurrentIndex(1);
    else if (type == "pie") chartTypeCombo->setCurrentIndex(2);
    else if (type == "scatter") chartTypeCombo->setCurrentIndex(3);
    else if (type == "histogram") chartTypeCombo->setCurrentIndex(4);
    else if (type == "box") chartTypeCombo->setCurrentIndex(5);
    else if (type == "violin") chartTypeCombo->setCurrentIndex(6);
    else if (type == "density") chartTypeCombo->setCurrentIndex(7);
    else if (type == "area") chartTypeCombo->setCurrentIndex(8);
}

void MainWindow::applyColorsToPlotWidget(const PlotConfig& config)
{
    // Apply colors to the plot widget
    // Note: This would require extending PlotWidget to accept color configurations
    plotWidget->setProperty("backgroundColor", config.backgroundColor);
    plotWidget->setProperty("gridColor", config.gridColor);
    plotWidget->setProperty("axisColor", config.axisColor);
    plotWidget->setProperty("textColor", config.textColor);
    plotWidget->setProperty("primaryColor", config.primaryColor);
    plotWidget->update();
}

int MainWindow::getCheckedChartTypeId()
{
    return chartTypeCombo->currentIndex();
}

void MainWindow::updateDetailedStatistics(const std::vector<double>& data)
{
    if (data.empty()) {
        statsText->clear();
        return;
    }

    // Calculate detailed statistics
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    double mean = sum / data.size();

    double variance = 0.0;
    for (double value : data) {
        variance += (value - mean) * (value - mean);
    }
    variance /= data.size();
    double stddev = std::sqrt(variance);

    auto minmax = std::minmax_element(data.begin(), data.end());

    // Calculate median
    std::vector<double> sortedData = data;
    std::sort(sortedData.begin(), sortedData.end());
    double median;
    if (sortedData.size() % 2 == 0) {
        median = (sortedData[sortedData.size()/2 - 1] + sortedData[sortedData.size()/2]) / 2.0;
    } else {
        median = sortedData[sortedData.size()/2];
    }

    // Calculate quartiles
    double q1 = sortedData[sortedData.size() / 4];
    double q3 = sortedData[3 * sortedData.size() / 4];

    QString statsInfo = "📊 统计总结:\n\n";
    statsInfo += QString("数量: %1\n").arg(data.size());
    statsInfo += QString("总和: %1\n").arg(sum, 0, 'f', 3);
    statsInfo += QString("平均值: %1\n").arg(mean, 0, 'f', 3);
    statsInfo += QString("中位数: %1\n").arg(median, 0, 'f', 3);
    statsInfo += QString("标准差: %1\n").arg(stddev, 0, 'f', 3);
    statsInfo += QString("最小值: %1\n").arg(*minmax.first, 0, 'f', 3);
    statsInfo += QString("最大值: %1\n").arg(*minmax.second, 0, 'f', 3);
    statsInfo += QString("第一四分位数: %1\n").arg(q1, 0, 'f', 3);
    statsInfo += QString("第三四分位数: %1\n").arg(q3, 0, 'f', 3);
    statsInfo += QString("范围: %1").arg(*minmax.second - *minmax.first, 0, 'f', 3);

    statsText->setPlainText(statsInfo);
}

void MainWindow::showQuickCommandLinks()
{
    QString quickCommands = "<br><div style='background-color:#f8f9fa; padding:8px; border-radius:4px; margin:5px 0;'>"
                           "🤖 <b>快速命令</b>: 点击下方链接快速执行<br>"
                           "<b>📈 图表:</b> "
                           "<a href='cmd:改为线图'>线图</a> | "
                           "<a href='cmd:改为柱状图'>柱状图</a> | "
                           "<a href='cmd:改为饼图'>饼图</a> | "
                           "<a href='cmd:改为散点图'>散点图</a> | "
                           "<a href='cmd:改为直方图'>直方图</a><br>"
                           "<b>✏️ 标题:</b> "
                           "<a href='cmd:修改标题为数据分析'>数据分析</a> | "
                           "<a href='cmd:修改标题为销售统计'>销售统计</a> | "
                           "<a href='cmd:修改标题为趋势图'>趋势图</a><br>"
                           "<b>📊 轴标签:</b> "
                           "<a href='cmd:设置X轴标签为时间'>X轴-时间</a> | "
                           "<a href='cmd:设置Y轴标签为数量'>Y轴-数量</a> | "
                           "<a href='cmd:设置Y轴标签为价格'>Y轴-价格</a><br>"
                           "<b>🎨 样式:</b> "
                           "<a href='cmd:显示网格'>显示网格</a> | "
                           "<a href='cmd:隐藏网格'>隐藏网格</a> | "
                           "<a href='cmd:设置线条宽度为3'>粗线条</a> | "
                           "<a href='cmd:使用大点'>大点</a>"
                           "</div>";
    aiChatArea->append(quickCommands);
}

void MainWindow::setupMultiColumnCheckboxes()
{
    // 清除现有的checkboxes
    for (QCheckBox* checkbox : columnCheckboxes) {
        checkbox->setParent(nullptr);
        checkbox->deleteLater();
    }
    columnCheckboxes.clear();

    // 创建新的layout
    if (columnCheckboxWidget->layout()) {
        delete columnCheckboxWidget->layout();
    }
    QVBoxLayout* checkboxLayout = new QVBoxLayout(columnCheckboxWidget);
    checkboxLayout->setSpacing(4);
    checkboxLayout->setContentsMargins(8, 8, 8, 8);

    // 为每个数据列（排除虚拟行索引列）创建checkbox
    for (int i = 1; i < columnHeaders.size(); ++i) {
        QCheckBox* checkbox = new QCheckBox(columnHeaders[i]);
        checkbox->setStyleSheet(QString("QCheckBox { font-size: %1px; }").arg(scaledSize(8)));
        
        // 默认选中第一个数据列
        if (i == 1) {
            checkbox->setChecked(true);
        }

        connect(checkbox, &QCheckBox::toggled, this, &MainWindow::onMultiColumnCheckboxChanged);
        
        columnCheckboxes.push_back(checkbox);
        checkboxLayout->addWidget(checkbox);
    }
    
    checkboxLayout->addStretch();
}

void MainWindow::onMultiColumnToggled(bool checked)
{
    if (checked) {
        // 显示多列选择区域
        columnCheckboxArea->setVisible(true);
        // 隐藏原来的单列选择
        yColumnCombo->setVisible(false);
    } else {
        // 隐藏多列选择区域
        columnCheckboxArea->setVisible(false);
        // 显示原来的单列选择
        yColumnCombo->setVisible(true);
        
        // 清除多系列状态，切换回单系列模式
        plotWidget->clearData();
    }
    
    // 应用变更
    applyColumnSelection();
}

void MainWindow::onMultiColumnCheckboxChanged()
{
    if (multiColumnCheckbox->isChecked()) {
        applyColumnSelection();
    }
}

void MainWindow::applyMultiColumnSelection()
{
    // 验证数据有效性
    if (rawData.empty()) {
        statusLabel->setText("❌ 错误：无数据可用");
        return;
    }

    // 获取X轴数据
    std::vector<double> xData;
    int xCol = xColumnCombo->currentIndex();
    
    // 验证X轴列索引
    if (xCol < 0 || xCol >= columnHeaders.size()) {
        statusLabel->setText("❌ 错误：X轴列选择无效");
        return;
    }
    
    if (xCol == 0) {
        // Virtual row index column selected
        for (size_t i = 0; i < rawData.size(); ++i) {
            xData.push_back(static_cast<double>(i + 1));
        }
    } else {
        // Real data column selected
        int realXCol = xCol - 1;
        if (realXCol >= 0 && realXCol < (int)rawData[0].size()) {
            for (const auto& row : rawData) {
                if (realXCol < (int)row.size()) {
                    xData.push_back(row[realXCol]);
                } else {
                    xData.push_back(0.0);
                }
            }
        } else {
            statusLabel->setText("❌ 错误：X轴列索引超出范围");
            return;
        }
    }
    
    // 收集选中的Y轴列
    std::vector<int> selectedColumns;
    for (size_t i = 0; i < columnCheckboxes.size(); ++i) {
        if (i < columnCheckboxes.size() && columnCheckboxes[i]->isChecked()) {
            int columnIndex = i + 1; // +1 because checkboxes start from column 1 (excluding virtual column)
            // 验证列索引是否有效
            if (columnIndex >= 0 && columnIndex < columnHeaders.size()) {
                selectedColumns.push_back(columnIndex);
            }
        }
    }
    
    if (selectedColumns.empty()) {
        statusLabel->setText("❌ 错误：请至少选择一个Y轴列");
        return;
    }
    
    if (selectedColumns.size() == 1) {
        // Single series - use traditional setData
        int firstSelectedCol = selectedColumns[0];
        std::vector<double> yData;
        
        int realYCol = firstSelectedCol - 1; // Convert to real column index
        for (const auto& row : rawData) {
            if (realYCol < (int)row.size()) {
                yData.push_back(row[realYCol]);
            } else {
                yData.push_back(0.0);
            }
        }
        
        plotWidget->setData(xData, yData);
        updateDetailedStatistics(yData);
        
        // Update axis labels
        xLabelEdit->setText(columnHeaders[xCol]);
        yLabelEdit->setText(columnHeaders[firstSelectedCol]);
        
        statusLabel->setText(QString("✅ 正在绘制 %1 (X轴) vs %2 (Y轴)")
                                 .arg(columnHeaders[xCol])
                                 .arg(columnHeaders[firstSelectedCol]));
        
        if (titleEdit->text().isEmpty() || titleEdit->text().contains("数据可视化")) {
            titleEdit->setText(QString("%1 对比 %2")
                                   .arg(columnHeaders[xCol])
                                   .arg(columnHeaders[firstSelectedCol]));
        }
    } else {
        // Multiple series - use new multi-series functionality
        std::vector<std::vector<double>> ySeriesData;
        std::vector<QString> seriesNames;
        
        for (int selectedCol : selectedColumns) {
            // 验证列索引有效性
            if (selectedCol >= columnHeaders.size()) {
                statusLabel->setText(QString("❌ 错误：列索引 %1 超出范围，最大列数：%2").arg(selectedCol).arg(columnHeaders.size()));
                return;
            }
            
            std::vector<double> yData;
            int realYCol = selectedCol - 1; // Convert to real column index
            
            // 验证实际数据列索引
            if (realYCol < 0 || (!rawData.empty() && realYCol >= (int)rawData[0].size())) {
                statusLabel->setText(QString("❌ 错误：数据列索引 %1 无效").arg(realYCol));
                return;
            }
            
            for (const auto& row : rawData) {
                if (realYCol < (int)row.size()) {
                    yData.push_back(row[realYCol]);
                } else {
                    yData.push_back(0.0);
                }
            }
            
            ySeriesData.push_back(yData);
            seriesNames.push_back(columnHeaders[selectedCol]);
        }
        
        plotWidget->setMultiSeriesData(xData, ySeriesData, seriesNames);
        
        // Use first series for statistics
        if (!ySeriesData.empty()) {
            updateDetailedStatistics(ySeriesData[0]);
        }
        
        // Update axis labels
        xLabelEdit->setText(columnHeaders[xCol]);
        yLabelEdit->setText(QString("多列数据 (%1个)").arg(selectedColumns.size()));
        
        statusLabel->setText(QString("✅ 正在绘制 %1 (X轴) vs %2个Y轴列")
                                 .arg(columnHeaders[xCol])
                                 .arg(selectedColumns.size()));
        
        if (titleEdit->text().isEmpty() || titleEdit->text().contains("数据可视化")) {
            titleEdit->setText(QString("多列数据对比图"));
        }
    }
}

bool MainWindow::validateColumnIndices()
{
    // Validate X column combo
    if (xColumnCombo->currentIndex() < 0 || xColumnCombo->currentIndex() >= columnHeaders.size()) {
        return false;
    }
    
    // Validate Y column combo
    if (yColumnCombo->currentIndex() < 0 || yColumnCombo->currentIndex() >= columnHeaders.size()) {
        return false;
    }
    
    // Validate multi-column checkboxes if they exist
    if (multiColumnCheckbox->isChecked()) {
        for (size_t i = 0; i < columnCheckboxes.size(); ++i) {
            if (columnCheckboxes[i]->isChecked()) {
                int columnIndex = i + 1; // +1 because checkboxes start from column 1
                if (columnIndex >= columnHeaders.size()) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

void MainWindow::performDataFitting()
{
    // 检查是否有数据可拟合
    if (rawData.empty() || xColumnCombo->currentIndex() < 0 || yColumnCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "拟合错误", "请先加载数据并选择X、Y轴列");
        return;
    }
    
    // 检查是否为多系列模式
    if (multiColumnCheckbox->isChecked() && multiColumnCheckbox->isVisible()) {
        QMessageBox::information(this, "拟合提示", "多系列模式下暂不支持拟合，请切换到单系列模式");
        return;
    }
    
    // 获取当前显示的数据
    std::vector<double> xData, yData;
    int xCol = xColumnCombo->currentIndex();
    int yCol = yColumnCombo->currentIndex();
    
    // 构建X数据
    if (xCol == 0) {
        for (size_t i = 0; i < rawData.size(); ++i) {
            xData.push_back(static_cast<double>(i + 1));
        }
    } else {
        int realXCol = xCol - 1;
        for (const auto& row : rawData) {
            if (realXCol < (int)row.size()) {
                xData.push_back(row[realXCol]);
            }
        }
    }
    
    // 构建Y数据
    if (yCol == 0) {
        for (size_t i = 0; i < rawData.size(); ++i) {
            yData.push_back(static_cast<double>(i + 1));
        }
    } else {
        int realYCol = yCol - 1;
        for (const auto& row : rawData) {
            if (realYCol < (int)row.size()) {
                yData.push_back(row[realYCol]);
            }
        }
    }
    
    if (xData.size() != yData.size() || xData.size() < 3) {
        QMessageBox::warning(this, "拟合错误", "数据点不足，至少需要3个数据点进行拟合");
        return;
    }
    
    // 获取拟合类型
    int fittingType = fittingCombo->currentData().toInt();
    std::vector<double> coefficients;
    std::vector<double> residuals;
    QString fittingTypeName;
    QString statusMessage;
    
    if (fittingType <= 2) {
        // 多项式拟合
        int degree = fittingType;
        coefficients = polynomialFit(xData, yData, degree);
        fittingTypeName = QString("%1次多项式拟合").arg(degree);
        statusMessage = QString("✅ %1次多项式拟合完成").arg(degree);
        
        if (coefficients.empty()) {
            QMessageBox::critical(this, "拟合错误", "多项式拟合失败，请检查数据");
            return;
        }
        
        // 计算多项式残差
        for (size_t i = 0; i < xData.size(); ++i) {
            double predicted = 0.0;
            for (size_t j = 0; j < coefficients.size(); ++j) {
                predicted += coefficients[j] * std::pow(xData[i], j);
            }
            residuals.push_back(yData[i] - predicted);
        }
    } else if (fittingType == 3) {
        // 正弦拟合
        coefficients = sinusoidalFit(xData, yData);
        fittingTypeName = "正弦拟合";
        statusMessage = "✅ 正弦拟合完成";
        
        if (coefficients.empty() || coefficients.size() < 4) {
            QMessageBox::critical(this, "拟合错误", "正弦拟合失败，请检查数据");
            return;
        }
        
        // 计算正弦拟合残差: y = A*sin(B*x + C) + D
        for (size_t i = 0; i < xData.size(); ++i) {
            double predicted = coefficients[0] * std::sin(coefficients[1] * xData[i] + coefficients[2]) + coefficients[3];
            residuals.push_back(yData[i] - predicted);
        }
    } else if (fittingType == 4) {
        // 高斯拟合
        coefficients = gaussianFit(xData, yData);
        fittingTypeName = "高斯拟合";
        statusMessage = "✅ 高斯拟合完成";
        
        if (coefficients.empty() || coefficients.size() < 3) {
            QMessageBox::critical(this, "拟合错误", "高斯拟合失败，请检查数据");
            return;
        }
        
        // 计算高斯拟合残差: y = A*exp(-((x-B)/C)^2)
        for (size_t i = 0; i < xData.size(); ++i) {
            double arg = (xData[i] - coefficients[1]) / coefficients[2];
            double predicted = coefficients[0] * std::exp(-arg * arg);
            residuals.push_back(yData[i] - predicted);
        }
    }
    
    // 将拟合结果传递给PlotWidget
    plotWidget->setPolynomialFitting(fittingType, coefficients, residuals, fittingTypeName);
    
    // 更新状态信息
    statusLabel->setText(statusMessage);
    
    // 更新统计信息，包含拟合参数
    QString fittingInfo = QString("🎯 %1参数:\n").arg(fittingTypeName);
    
    if (fittingType <= 2) {
        // 多项式拟合参数
        for (size_t i = 0; i < coefficients.size(); ++i) {
            if (i == 0) {
                fittingInfo += QString("  常数项: %1\n").arg(coefficients[i], 0, 'f', 4);
            } else if (i == 1) {
                fittingInfo += QString("  一次项: %1\n").arg(coefficients[i], 0, 'f', 4);
            } else {
                fittingInfo += QString("  %1次项: %2\n").arg(i).arg(coefficients[i], 0, 'f', 4);
            }
        }
    } else if (fittingType == 3) {
        // 正弦拟合参数: y = A*sin(B*x + C) + D
        fittingInfo += QString("  振幅 A: %1\n").arg(coefficients[0], 0, 'f', 4);
        fittingInfo += QString("  频率 B: %1\n").arg(coefficients[1], 0, 'f', 4);
        fittingInfo += QString("  相位 C: %1\n").arg(coefficients[2], 0, 'f', 4);
        fittingInfo += QString("  偏移 D: %1\n").arg(coefficients[3], 0, 'f', 4);
        fittingInfo += QString("  拟合函数: y = %1*sin(%2*x + %3) + %4\n")
                           .arg(coefficients[0], 0, 'f', 3)
                           .arg(coefficients[1], 0, 'f', 3)
                           .arg(coefficients[2], 0, 'f', 3)
                           .arg(coefficients[3], 0, 'f', 3);
    } else if (fittingType == 4) {
        // 高斯拟合参数: y = A*exp(-((x-B)/C)^2)
        fittingInfo += QString("  振幅 A: %1\n").arg(coefficients[0], 0, 'f', 4);
        fittingInfo += QString("  中心 B: %1\n").arg(coefficients[1], 0, 'f', 4);
        fittingInfo += QString("  标准差 C: %1\n").arg(coefficients[2], 0, 'f', 4);
        fittingInfo += QString("  拟合函数: y = %1*exp(-((x-%2)/%3)²)\n")
                           .arg(coefficients[0], 0, 'f', 3)
                           .arg(coefficients[1], 0, 'f', 3)
                           .arg(coefficients[2], 0, 'f', 3);
    }
    
    // 计算R²
    double ssRes = 0.0, ssTot = 0.0;
    double yMean = std::accumulate(yData.begin(), yData.end(), 0.0) / yData.size();
    
    for (size_t i = 0; i < residuals.size(); ++i) {
        ssRes += residuals[i] * residuals[i];
        ssTot += (yData[i] - yMean) * (yData[i] - yMean);
    }
    
    double rSquared = (ssTot > 0) ? 1.0 - (ssRes / ssTot) : 0.0;
    fittingInfo += QString("  决定系数 R²: %1").arg(rSquared, 0, 'f', 4);
    
    // 将拟合信息附加到统计信息
    QString currentStats = statsText->toPlainText();
    if (!currentStats.isEmpty()) {
        currentStats += "\n\n" + fittingInfo;
    } else {
        currentStats = fittingInfo;
    }
    statsText->setPlainText(currentStats);
}

// 简单的多项式拟合实现（最小二乘法）
std::vector<double> MainWindow::polynomialFit(const std::vector<double>& x, const std::vector<double>& y, int degree)
{
    if (x.size() != y.size() || x.size() <= degree) {
        return std::vector<double>();
    }
    
    int n = x.size();
    int m = degree + 1;
    
    // 构建设计矩阵 A 和目标向量 b
    std::vector<std::vector<double>> A(n, std::vector<double>(m));
    std::vector<double> b = y;
    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            A[i][j] = std::pow(x[i], j);
        }
    }
    
    // 构建正规方程 A^T * A * coeffs = A^T * b
    std::vector<std::vector<double>> AtA(m, std::vector<double>(m, 0.0));
    std::vector<double> Atb(m, 0.0);
    
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            for (int k = 0; k < n; ++k) {
                AtA[i][j] += A[k][i] * A[k][j];
            }
        }
        for (int k = 0; k < n; ++k) {
            Atb[i] += A[k][i] * b[k];
        }
    }
    
    // 使用高斯消元法求解线性方程组
    return gaussianElimination(AtA, Atb);
}

// 高斯消元法
std::vector<double> MainWindow::gaussianElimination(std::vector<std::vector<double>>& A, std::vector<double>& b)
{
    int n = A.size();
    
    // 前向消元
    for (int i = 0; i < n; ++i) {
        // 找到主元
        int maxRow = i;
        for (int k = i + 1; k < n; ++k) {
            if (std::abs(A[k][i]) > std::abs(A[maxRow][i])) {
                maxRow = k;
            }
        }
        
        // 交换行
        std::swap(A[i], A[maxRow]);
        std::swap(b[i], b[maxRow]);
        
        // 检查奇异矩阵
        if (std::abs(A[i][i]) < 1e-10) {
            return std::vector<double>(); // 奇异矩阵
        }
        
        // 消元
        for (int k = i + 1; k < n; ++k) {
            double factor = A[k][i] / A[i][i];
            for (int j = i; j < n; ++j) {
                A[k][j] -= factor * A[i][j];
            }
            b[k] -= factor * b[i];
        }
    }
    
    // 回代
    std::vector<double> x(n);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = b[i];
        for (int j = i + 1; j < n; ++j) {
            x[i] -= A[i][j] * x[j];
        }
        x[i] /= A[i][i];
    }
    
    return x;
}

// 正弦拟合实现 y = A*sin(B*x + C) + D
std::vector<double> MainWindow::sinusoidalFit(const std::vector<double>& x, const std::vector<double>& y)
{
    if (x.size() != y.size() || x.size() < 4) {
        return std::vector<double>();
    }
    
    // 初始估计参数
    double yMean = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    auto yMinMax = std::minmax_element(y.begin(), y.end());
    double amplitude = (*yMinMax.second - *yMinMax.first) / 2.0;
    double offset = yMean;
    
    // 估计频率：使用FFT的简化版本，这里用简单的周期估计
    double xRange = *std::max_element(x.begin(), x.end()) - *std::min_element(x.begin(), x.end());
    double frequency = 2.0 * M_PI / (xRange / 2.0); // 估计频率
    double phase = 0.0;
    
    // 使用非线性最小二乘法拟合（简化版本）
    // 这里使用迭代优化
    std::vector<double> params = {amplitude, frequency, phase, offset};
    
    // 简单的梯度下降优化
    double learningRate = 0.001;
    int maxIterations = 1000;
    double tolerance = 1e-6;
    
    for (int iter = 0; iter < maxIterations; ++iter) {
        std::vector<double> gradients(4, 0.0);
        double totalError = 0.0;
        
        for (size_t i = 0; i < x.size(); ++i) {
            double predicted = params[0] * std::sin(params[1] * x[i] + params[2]) + params[3];
            double error = y[i] - predicted;
            totalError += error * error;
            
            // 计算梯度
            double cosValue = std::cos(params[1] * x[i] + params[2]);
            double sinValue = std::sin(params[1] * x[i] + params[2]);
            
            gradients[0] += -2.0 * error * sinValue; // ∂E/∂A
            gradients[1] += -2.0 * error * params[0] * x[i] * cosValue; // ∂E/∂B
            gradients[2] += -2.0 * error * params[0] * cosValue; // ∂E/∂C
            gradients[3] += -2.0 * error; // ∂E/∂D
        }
        
        // 更新参数
        bool converged = true;
        for (int j = 0; j < 4; ++j) {
            double update = learningRate * gradients[j] / x.size();
            if (std::abs(update) > tolerance) converged = false;
            params[j] -= update;
        }
        
        if (converged) break;
        
        // 自适应学习率
        if (iter % 100 == 0) {
            learningRate *= 0.95;
        }
    }
    
    return params;
}

// 高斯拟合实现 y = A*exp(-((x-B)/C)^2)
std::vector<double> MainWindow::gaussianFit(const std::vector<double>& x, const std::vector<double>& y)
{
    if (x.size() != y.size() || x.size() < 3) {
        return std::vector<double>();
    }
    
    // 初始估计参数
    auto yMaxIter = std::max_element(y.begin(), y.end());
    size_t maxIndex = std::distance(y.begin(), yMaxIter);
    
    double A = *yMaxIter; // 振幅：最大值
    double B = x[maxIndex]; // 中心：最大值对应的x
    
    // 估计标准差：找到半最大值的位置
    double halfMax = A / 2.0;
    double C = 1.0;
    
    // 寻找半最大值宽度来估计标准差
    for (size_t i = 0; i < y.size(); ++i) {
        if (std::abs(y[i] - halfMax) < A * 0.1) {
            C = std::max(C, std::abs(x[i] - B));
        }
    }
    
    std::vector<double> params = {A, B, C};
    
    // 使用非线性最小二乘法（Levenberg-Marquardt的简化版）
    double learningRate = 0.001;
    int maxIterations = 1000;
    double tolerance = 1e-6;
    
    for (int iter = 0; iter < maxIterations; ++iter) {
        std::vector<double> gradients(3, 0.0);
        double totalError = 0.0;
        
        for (size_t i = 0; i < x.size(); ++i) {
            double arg = (x[i] - params[1]) / params[2];
            double expValue = std::exp(-arg * arg);
            double predicted = params[0] * expValue;
            double error = y[i] - predicted;
            totalError += error * error;
            
            // 计算梯度
            gradients[0] += -2.0 * error * expValue; // ∂E/∂A
            gradients[1] += -2.0 * error * params[0] * expValue * (2.0 * arg / params[2]); // ∂E/∂B
            gradients[2] += -2.0 * error * params[0] * expValue * (2.0 * arg * arg / params[2]); // ∂E/∂C
        }
        
        // 更新参数
        bool converged = true;
        for (int j = 0; j < 3; ++j) {
            double update = learningRate * gradients[j] / x.size();
            if (std::abs(update) > tolerance) converged = false;
            params[j] -= update;
        }
        
        // 防止参数变得不合理
        if (params[0] <= 0) params[0] = 0.1;
        if (params[2] <= 0) params[2] = 0.1;
        
        if (converged) break;
        
        // 自适应学习率
        if (iter % 100 == 0) {
            learningRate *= 0.95;
        }
    }
    
    return params;
}
