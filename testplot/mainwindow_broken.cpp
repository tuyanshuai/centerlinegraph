#include "mainwindow.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow::MainWindow(const QString& initialFile = QString(), QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("TXT数据绘图工具 - 中文增强版");
        setMinimumSize(static_cast<int>(1000 * qApp->devicePixelRatio()), 
                       static_cast<int>(700 * qApp->devicePixelRatio()));

        setupUI();
        connectSignals();
        setupDeepSeekDialog();
        
        // Load initial file if provided
        if (!initialFile.isEmpty()) {
            loadDataFromFile(initialFile);
        }
    }
    
    void setInitialChartType(ChartType type)
    {
        int index = static_cast<int>(type);
        if (index >= 0 && index < chartTypeCombo->count()) {
            chartTypeCombo->setCurrentIndex(index);
        }
        plotWidget->setChartType(type);
    }

private slots:
    void loadFile()
    {
        QString fileName = QFileDialog::getOpenFileName(this, 
            "打开数据文件",
            "", 
            "文本文件 (*.txt);;所有文件 (*)"); // Chinese filter
        
        if (!fileName.isEmpty()) {
            loadDataFromFile(fileName);
        }
    }
    
    void applyColumnSelection()
    {
        if (rawData.empty() || xColumnCombo->currentIndex() < 0 || yColumnCombo->currentIndex() < 0) {
            statusLabel->setText("❌ 错误：未加载数据或列选择无效");
            return;
        }
        
        int xCol = xColumnCombo->currentIndex();
        int yCol = yColumnCombo->currentIndex();
        
        // Validate column indices
        if (xCol >= columnHeaders.size() || yCol >= columnHeaders.size()) {
            statusLabel->setText("❌ 错误：列选择无效");
            return;
        }
        
        std::vector<double> xData, yData;
        
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

    void clearPlot()
    {
        plotWidget->clearData();
        statsText->clear();
        statusLabel->setText("✅ 图表已清空");
        infoText->clear();
    }
    
    void onChartTypeChanged(int buttonId)
    {
        ChartType type = static_cast<ChartType>(buttonId);
        plotWidget->setChartType(type);
        updateStatistics();
    }
    
    void updateChartTitle()
    {
        plotWidget->setTitle(titleEdit->text());
    }
    
    void updateAxisLabels()
    {
        plotWidget->setAxisLabels(xLabelEdit->text(), yLabelEdit->text());
    }
    
    void updateStatistics()
    {
        // This will be implemented when we enhance the statistics functionality
        if (plotWidget && !plotWidget->property("isEmpty").toBool()) {
            QString statsInfo = "📈 统计总结:\n\n";
            // Add more detailed statistics here
            statsText->setPlainText(statsInfo);
        }
    }
    
    void openDeepSeekDialog()
    {
        // 设置当前图表信息
        QString plotInfo = generateCurrentPlotInfo();
        deepseekDialog->setCurrentPlotInfo(plotInfo);
        
        // 显示对话框
        deepseekDialog->show();
        deepseekDialog->raise();
        deepseekDialog->activateWindow();
    }

private:
    // DPI缩放辅助函数
    int scaledSize(int baseSize) const {
        return static_cast<int>(baseSize * qMax(1.0, qApp->devicePixelRatio()));
    }
    
    void setupUI()
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
        
        // File operations group
        QGroupBox *fileGroup = new QGroupBox("文件操作");
        fileGroup->setStyleSheet("QGroupBox { font-weight: bold; padding-top: 10px; font-size: 11px; }");
        QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);
        fileLayout->setSpacing(6);
        fileLayout->setContentsMargins(8, 10, 8, 8);
        
        // 创建按钮的两列布局
        QGridLayout *buttonGrid = new QGridLayout();
        buttonGrid->setSpacing(4);
        
        loadButton = new QPushButton("📂 加载");
        loadButton->setMinimumHeight(scaledSize(32));
        loadButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        loadButton->setStyleSheet(QString("QPushButton { padding: 6px 8px; font-size: %1px; background-color: #007bff; color: white; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #0056b3; }").arg(scaledSize(9)));
        
        clearButton = new QPushButton("🗑️ 清空");
        clearButton->setMinimumHeight(scaledSize(32));
        clearButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        clearButton->setStyleSheet(QString("QPushButton { padding: 6px 8px; font-size: %1px; background-color: #6c757d; color: white; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #545b62; }").arg(scaledSize(9)));
        
        resetViewButton = new QPushButton("🎯 重置视图");
        resetViewButton->setMinimumHeight(scaledSize(32));
        resetViewButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        resetViewButton->setStyleSheet(QString("QPushButton { padding: 6px 8px; font-size: %1px; background-color: #17a2b8; color: white; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #138496; }").arg(scaledSize(9)));
        resetViewButton->setToolTip("重置缩放和平移到默认视图");
        
        // 添加AI对话按钮
        aiDialogButton = new QPushButton("🤖 AI助手");
        aiDialogButton->setMinimumHeight(scaledSize(32));
        aiDialogButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        aiDialogButton->setStyleSheet(QString("QPushButton { padding: 6px 8px; font-size: %1px; background-color: #6f42c1; color: white; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #5a32a3; }").arg(scaledSize(9)));
        aiDialogButton->setToolTip("打开DeepSeek AI对话");
        
        // 按钮排列：两行两列
        buttonGrid->addWidget(loadButton, 0, 0);
        buttonGrid->addWidget(clearButton, 0, 1);
        buttonGrid->addWidget(resetViewButton, 1, 0);
        buttonGrid->addWidget(aiDialogButton, 1, 1);
        
        fileLayout->addLayout(buttonGrid);
        
        // AI对话现在永远显示，移除切换按钮
        
        QLabel *viewHelpLabel = new QLabel("📜 视图控制：\n• 鼠标：拖拽平移\n• 滚轮：缩放\n• +/-：缩放快捷键\n• 0：重置视图");
        viewHelpLabel->setStyleSheet(QString("QLabel { font-size: %1px; color: #6c757d; padding: 6px; background-color: #e9ecef; border-radius: 3px; }").arg(scaledSize(8)));
        fileLayout->addWidget(viewHelpLabel);
        
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
        
        // Statistics group
        QGroupBox *statsGroup = new QGroupBox("统计信息");
        statsGroup->setStyleSheet("QGroupBox { font-weight: bold; padding-top: 10px; font-size: 11px; }");
        QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);
        statsLayout->setSpacing(6);
        statsLayout->setContentsMargins(8, 10, 8, 8);
        
        statsText = new QTextEdit();
        statsText->setMinimumHeight(scaledSize(120));
        statsText->setMaximumHeight(scaledSize(180));
        statsText->setReadOnly(true);
        statsText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        statsText->setStyleSheet(QString("QTextEdit { font-family: 'Microsoft YaHei', 'Courier New'; font-size: %1px; border: 1px solid #ced4da; border-radius: 4px; background-color: white; }").arg(scaledSize(8)));
        statsLayout->addWidget(statsText);
        
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
        columnLayout->addWidget(applyColumnButton, 2, 0, 1, 2);
        
        // Add reset button
        QPushButton *resetColumnButton = new QPushButton("🔄 重置");
        resetColumnButton->setMinimumHeight(scaledSize(28));
        resetColumnButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        resetColumnButton->setStyleSheet(QString("QPushButton { padding: 6px 8px; font-size: %1px; background-color: #ffc107; color: black; border: none; border-radius: 3px; } QPushButton:hover { background-color: #e0a800; }").arg(scaledSize(8)));
        resetColumnButton->setToolTip("重置为默认列选择");
        columnLayout->addWidget(resetColumnButton, 3, 0, 1, 2);
        
        connect(resetColumnButton, &QPushButton::clicked, [this]() {
            if (columnHeaders.size() >= 2) {
                xColumnCombo->setCurrentIndex(0); // Row Index (Virtual)
                yColumnCombo->setCurrentIndex(1); // First data column
            }
            applyColumnSelection();
        });
        
        columnInfoLabel = new QLabel("");
        columnInfoLabel->setStyleSheet(QString("QLabel { font-size: %1px; color: #6c757d; padding: 6px; }").arg(scaledSize(8)));
        columnLayout->addWidget(columnInfoLabel, 4, 0, 1, 2);
        
        // AI对话面板 - 永远显示
        aiChatPanel = new QWidget();
        aiChatPanel->setVisible(true); // 永远显示
        QVBoxLayout *aiLayout = new QVBoxLayout(aiChatPanel);
        aiLayout->setContentsMargins(0, 0, 0, 0);
        
        // 创建AI对话界面组件
        createAIChatInterface(aiLayout);
        
        rightLayout->addWidget(fileGroup);
        rightLayout->addWidget(columnGroup);
        rightLayout->addWidget(chartGroup);
        rightLayout->addWidget(customGroup);
        rightLayout->addWidget(statsGroup);
        rightLayout->addWidget(aiChatPanel);
        rightLayout->addStretch();
        
        // 将内容组件设置到滚动区域中
        rightScrollArea->setWidget(rightPanel);
        
        // Plot area layout
        QVBoxLayout *plotAreaLayout = new QVBoxLayout(plotArea);
        plotAreaLayout->setContentsMargins(10, 10, 10, 10);
        
        // 绘图控件
        plotWidget = new PlotWidget();
        plotWidget->setStyleSheet("QWidget { background-color: white; border: 1px solid #dee2e6; border-radius: 8px; }");

        // 信息显示
        infoText = new QTextEdit();
        infoText->setMaximumHeight(scaledSize(80));
        infoText->setReadOnly(true);
        infoText->setStyleSheet(QString("QTextEdit { font-family: 'Microsoft YaHei', 'Consolas'; font-size: %1px; border: 1px solid #ced4da; border-radius: 4px; background-color: #f8f9fa; }").arg(scaledSize(8)));

        // 状态标签
        statusLabel = new QLabel("准备加载数据文件");
        statusLabel->setStyleSheet(QString("QLabel { padding: 8px; background-color: #e9ecef; border-radius: 4px; font-weight: bold; font-size: %1px; }").arg(scaledSize(9)));
        
        plotAreaLayout->addWidget(plotWidget, 1);
        QLabel *dataInfoLabel = new QLabel("📊 数据信息:");
        dataInfoLabel->setStyleSheet(QString("font-size: %1px; font-weight: bold;").arg(scaledSize(9)));
        plotAreaLayout->addWidget(dataInfoLabel);
        plotAreaLayout->addWidget(infoText);
        plotAreaLayout->addWidget(statusLabel);
        
        mainLayout->addWidget(plotArea, 1);
        mainLayout->addWidget(rightScrollArea);
    }

    void connectSignals()
    {
        connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadFile);
        connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearPlot);
        
        connect(chartTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &MainWindow::onChartTypeChanged);
        
        connect(titleEdit, &QLineEdit::textChanged, this, &MainWindow::updateChartTitle);
        connect(xLabelEdit, &QLineEdit::textChanged, this, &MainWindow::updateAxisLabels);
        connect(yLabelEdit, &QLineEdit::textChanged, this, &MainWindow::updateAxisLabels);
        
        connect(applyColumnButton, &QPushButton::clicked, this, &MainWindow::applyColumnSelection);
        
        // Auto-apply when combo boxes change
        connect(xColumnCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyColumnSelection);
        connect(yColumnCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyColumnSelection);
        
        // Connect reset view button
        connect(resetViewButton, &QPushButton::clicked, plotWidget, &PlotWidget::resetView);
        
        // Connect AI dialog button
        connect(aiDialogButton, &QPushButton::clicked, this, &MainWindow::openDeepSeekDialog);
    }

    std::vector<double> parseNumbersFromLine(const std::string& line) {
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
    
    void loadDataFromFile(const QString& fileName)
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
    
    void updateColumnSelectionUI()
    {
        if (rawData.empty()) {
            columnGroup->setVisible(false);
            return;
        }
        
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
            
            columnInfoLabel->setText(QString("文件包含 %1 个数据列 + 虚拟行索引。默认: %2 (X) vs %3 (Y)。")
                                   .arg(columnHeaders.size() - 1)
                                   .arg(columnHeaders[0])
                                   .arg(columnHeaders[1]));
        } else if (columnHeaders.size() == 2) { // Row Index + 1 data column
            xColumnCombo->setCurrentIndex(0); // Row Index (Virtual)
            yColumnCombo->setCurrentIndex(1); // Single data column
            
            columnInfoLabel->setText(QString("检测到单个数据列: %1。默认X轴使用行索引。")
                                   .arg(columnHeaders[1]));
        } else {
            columnGroup->setVisible(false);
        }
    }

    QPushButton *loadButton;
    QPushButton *clearButton;
    QPushButton *resetViewButton;
    QPushButton *aiDialogButton;
    PlotWidget *plotWidget;
    QTextEdit *infoText;
    QTextEdit *statsText;
    QLabel *statusLabel;
    
    QComboBox *chartTypeCombo;
    
    QLineEdit *titleEdit;
    QLineEdit *xLabelEdit;
    QLineEdit *yLabelEdit;
    
    // Column selection controls
    QComboBox *xColumnCombo;
    QComboBox *yColumnCombo;
    QPushButton *applyColumnButton;
    QLabel *columnInfoLabel;
    QGroupBox *columnGroup;
    
    // Raw data storage
    std::vector<std::vector<double>> rawData;
    QStringList columnHeaders;
    bool hasMultipleColumns;
    
    // AI Assistant
    DeepSeekDialog *deepseekDialog;
    QWidget *aiChatPanel;
    QTextBrowser *aiChatArea;
    QLineEdit *aiInputEdit;
    QPushButton *aiSendButton;
    QNetworkAccessManager *aiNetworkManager;
    QString aiApiKey;
    
    void createAIChatInterface(QVBoxLayout *layout)
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
        
        // 添加欢迎信息
        aiChatArea->append("🤖 <b>智能助手</b>: 先尝试快速命令处理，无法处理时自动使用DeepSeek AI。<br>支持图表类型、标题、轴标签等修改。复杂需求会自动转到DeepSeek处理。");
        
        // 连接信号
        connect(aiSendButton, &QPushButton::clicked, this, &MainWindow::sendAIMessage);
        connect(aiInputEdit, &QLineEdit::returnPressed, this, &MainWindow::sendAIMessage);
    }
    
    void setupDeepSeekDialog()
    {
        deepseekDialog = new DeepSeekDialog(this);
        connect(deepseekDialog, &DeepSeekDialog::configurationGenerated, 
                this, &MainWindow::applyConfiguration);
        
        // 初始化AI网络管理器
        aiNetworkManager = new QNetworkAccessManager(this);
        aiApiKey = "sk-824b0dea3d674ebf980c1ac28d9fa0b6";
    }
    
    // 移除了toggleAIPanel方法，AI面板现在永远显示
    
    void sendAIMessage()
    {
        QString message = aiInputEdit->text().trimmed();
        if (message.isEmpty()) return;
        
        // 显示用户消息
        aiChatArea->append(QString("\n👤 <b>您</b>: %1").arg(message));
        aiInputEdit->clear();
        
        // 简单的AI响应逻辑（模拟）
        processAIRequest(message);
    }
    
    void processAIRequest(const QString& request)
    {
        QString response = "🤖 <b>快速助手</b>: ";
        QString lowerRequest = request.toLower();
        bool handled = false;
        
        // 图表类型修改
        if (lowerRequest.contains("柱状图") || lowerRequest.contains("bar")) {
            chartTypeCombo->setCurrentIndex(1);
            response += "已将图表类型改为柱状图。";
            handled = true;
        } else if (lowerRequest.contains("线图") || lowerRequest.contains("line")) {
            chartTypeCombo->setCurrentIndex(0);
            response += "已将图表类型改为线图。";
            handled = true;
        } else if (lowerRequest.contains("饼图") || lowerRequest.contains("pie")) {
            chartTypeCombo->setCurrentIndex(2);
            response += "已将图表类型改为饼图。";
            handled = true;
        } else if (lowerRequest.contains("散点图") || lowerRequest.contains("scatter")) {
            chartTypeCombo->setCurrentIndex(3);
            response += "已将图表类型改为散点图。";
            handled = true;
        } else if (lowerRequest.contains("直方图") || lowerRequest.contains("histogram")) {
            chartTypeCombo->setCurrentIndex(4);
            response += "已将图表类型改为直方图。";
            handled = true;
        } else if (lowerRequest.contains("箱线图") || lowerRequest.contains("box")) {
            chartTypeCombo->setCurrentIndex(5);
            response += "已将图表类型改为箱线图。";
            handled = true;
        } else if (lowerRequest.contains("小提琴图") || lowerRequest.contains("violin")) {
            chartTypeCombo->setCurrentIndex(6);
            response += "已将图表类型改为小提琴图。";
            handled = true;
        } else if (lowerRequest.contains("密度图") || lowerRequest.contains("density")) {
            chartTypeCombo->setCurrentIndex(7);
            response += "已将图表类型改为密度图。";
            handled = true;
        } else if (lowerRequest.contains("面积图") || lowerRequest.contains("area")) {
            chartTypeCombo->setCurrentIndex(8);
            response += "已将图表类型改为面积图。";
            handled = true;
        }
        // 标题修改
        else if (lowerRequest.contains("标题")) {
            QRegExp rx("标题.{0,10}['\"]?([^'\"\n]{1,50})['\"]?");
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
                response += "请告诉我您希望设置的标题内容，例如：\"标题为销售数据分析\"。";
                handled = true;
            }
        }
        // X轴标签修改
        else if (lowerRequest.contains("x轴") || lowerRequest.contains("横轴")) {
            QRegExp rx("[xX横]轴.{0,10}['\"]?([^'\"\n]{1,30})['\"]?");
            if (rx.indexIn(request) != -1) {
                QString label = rx.cap(1).trimmed();
                if (!label.isEmpty()) {
                    xLabelEdit->setText(label);
                    updateAxisLabels();
                    response += QString("已将X轴标签设置为: %1").arg(label);
                    handled = true;
                }
            } else {
                response += "请指定X轴标签内容。";
                handled = true;
            }
        }
        // Y轴标签修改
        else if (lowerRequest.contains("y轴") || lowerRequest.contains("纵轴")) {
            QRegExp rx("[yY纵]轴.{0,10}['\"]?([^'\"\n]{1,30})['\"]?");
            if (rx.indexIn(request) != -1) {
                QString label = rx.cap(1).trimmed();
                if (!label.isEmpty()) {
                    yLabelEdit->setText(label);
                    updateAxisLabels();
                    response += QString("已将Y轴标签设置为: %1").arg(label);
                    handled = true;
                }
            } else {
                response += "请指定Y轴标签内容。";
                handled = true;
            }
        }
        // 网格显示控制
        else if (lowerRequest.contains("网格")) {
            if (lowerRequest.contains("隐藏") || lowerRequest.contains("关闭") || lowerRequest.contains("不显示")) {
                plotWidget->setGridVisible(false);
                response += "已隐藏网格。";
                handled = true;
            } else if (lowerRequest.contains("显示") || lowerRequest.contains("开启")) {
                plotWidget->setGridVisible(true);
                response += "已显示网格。";
                handled = true;
            }
        }
        // 线条宽度修改
        else if (lowerRequest.contains("线条") && (lowerRequest.contains("宽") || lowerRequest.contains("粗") || lowerRequest.contains("细"))) {
            QRegExp rx("线条.{0,10}(\\d+)");
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
            } else if (lowerRequest.contains("粗")) {
                plotWidget->setLineWidth(5);
                response += "已将线条设置为粗线。";
                handled = true;
            } else if (lowerRequest.contains("细")) {
                plotWidget->setLineWidth(1);
                response += "已将线条设置为细线。";
                handled = true;
            }
        }
        // 点大小修改
        else if (lowerRequest.contains("点") && (lowerRequest.contains("大") || lowerRequest.contains("小"))) {
            QRegExp rx("点.{0,10}(\\d+)");
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
            } else if (lowerRequest.contains("大")) {
                plotWidget->setPointSize(8);
                response += "已将点设置为大尺寸。";
                handled = true;
            } else if (lowerRequest.contains("小")) {
                plotWidget->setPointSize(3);
                response += "已将点设置为小尺寸。";
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
        aiChatArea->verticalScrollBar()->setValue(aiChatArea->verticalScrollBar()->maximum());
    }
    
    void fallbackToDeepSeek(const QString& request)
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
    
    void parseAndApplyDeepSeekConfig(const QString& jsonString)
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
    
    void loadConfigurationFile(const QString& filePath)
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
    
    QString generateCurrentPlotInfo()
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
    
    QString getCurrentChartTypeName()
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
    
    void applyConfiguration(const PlotConfig& config)
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
    
    void exportConfiguration()
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
    
    void importConfiguration()
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
    
    QString getCurrentChartTypeString()
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
    
    void setChartTypeByString(const QString& type)
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
    
    void applyColorsToPlotWidget(const PlotConfig& config)
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
    
    int getCheckedChartTypeId()
    {
        return chartTypeCombo->currentIndex();
    }

    void updateDetailedStatistics(const std::vector<double>& data)
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
};
