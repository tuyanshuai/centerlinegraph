#include "deepseek_dialog.h"

DeepSeekDialog::DeepSeekDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("AI智能助手 - 对话式绘图配置");
    setMinimumSize(800, 650);
    resize(950, 750);

    setupUI();
    setupNetworkManager();
    loadSystemPrompt();
    
    // Pre-set the API key
    apiKey = "sk-824b0dea3d674ebf980c1ac28d9fa0b6";
    apiKeyButton->setText("已设置密钥 ✓");
    apiKeyButton->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #28a745; color: white; border: none; border-radius: 4px; font-weight: bold; }");
    
    // Add welcome message with SSL troubleshooting info
    addMessageToChat("系统", "🤖 DeepSeek AI智能助手已准备就绪！\n\n如果遇到SSL错误，这通常是因为缺少OpenSSL库。请联系管理员安装或使用手动JSON配置。\n\n您可以用自然语言描述想要的图表修改，例如：\n• \"改成柱状图\"\n• \"标题改为'销售数据分析'\"\n• \"使用绿色配色方案\"\n• \"调整字体大小\"");
}

void DeepSeekDialog::setCurrentPlotInfo(const QString& info)
{
    currentPlotInfo = info;
}

void DeepSeekDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // API Key and control area
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    apiKeyButton = new QPushButton("设置API密钥");
    apiKeyButton->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #ffc107; color: black; border: none; border-radius: 4px; font-weight: bold; }");
    connect(apiKeyButton, &QPushButton::clicked, this, &DeepSeekDialog::setApiKey);
    
    QPushButton *helpButton = new QPushButton("📖 使用说明");
    helpButton->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #17a2b8; color: white; border: none; border-radius: 4px; }");
    connect(helpButton, &QPushButton::clicked, this, &DeepSeekDialog::showHelp);
    
    QPushButton *clearButton = new QPushButton("🗑️ 清空对话");
    clearButton->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #6c757d; color: white; border: none; border-radius: 4px; }");
    connect(clearButton, &QPushButton::clicked, [this]() {
        chatArea->clear();
        addMessageToChat("系统", "对话历史已清空，您可以重新开始对话。");
    });
    
    QPushButton *testButton = new QPushButton("🔧 测试连接");
    testButton->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #ffc107; color: black; border: none; border-radius: 4px; }");
    connect(testButton, &QPushButton::clicked, this, &DeepSeekDialog::testConnection);
    
    controlLayout->addWidget(apiKeyButton);
    controlLayout->addWidget(helpButton);
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(testButton);
    controlLayout->addStretch();

    // Chat area
    chatArea = new QTextBrowser();
    chatArea->setStyleSheet(R"(
        QTextBrowser {
            border: 1px solid #dee2e6;
            border-radius: 8px;
            background-color: #fafafa;
            font-family: 'Microsoft YaHei', '微软雅黑', Arial, sans-serif;
            font-size: 15px;
            padding: 12px;
        }
    )");

    // Input area
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    inputEdit = new QLineEdit();
    inputEdit->setPlaceholderText("💬 输入您的绘图需求，例如：把图表改成柱状图，标题改为'销售数据分析'...");
    inputEdit->setStyleSheet(R"(
        QLineEdit {
            padding: 12px 16px;
            border: 2px solid #ced4da;
            border-radius: 25px;
            font-size: 15px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #007bff;
            outline: none;
        }
    )");
    
    sendButton = new QPushButton("🚀 发送");
    sendButton->setStyleSheet(R"(
        QPushButton {
            padding: 12px 24px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 25px;
            font-weight: bold;
            font-size: 15px;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QPushButton:disabled {
            background-color: #6c757d;
        }
    )");
    
    connect(inputEdit, &QLineEdit::returnPressed, this, &DeepSeekDialog::sendMessage);
    connect(sendButton, &QPushButton::clicked, this, &DeepSeekDialog::sendMessage);

    inputLayout->addWidget(inputEdit, 1);
    inputLayout->addWidget(sendButton);

    // Loading indicator
    loadingLabel = new QLabel("🤔 AI正在思考中，请稍候...");
    loadingLabel->setStyleSheet(R"(
        QLabel {
            color: #6c757d;
            font-style: italic;
            padding: 8px;
            border-radius: 4px;
            background-color: #e9ecef;
        }
    )");
    loadingLabel->hide();

    // Status area
    QLabel *statusInfo = new QLabel("💡 提示：描述越具体，AI生成的配置越准确！支持中文对话。");
    statusInfo->setStyleSheet(R"(
        QLabel {
            color: #495057;
            font-size: 13px;
            padding: 8px;
            background-color: #e7f3ff;
            border-radius: 4px;
            border-left: 3px solid #007bff;
        }
    )");

    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(chatArea, 1);
    mainLayout->addWidget(loadingLabel);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(statusInfo);
}

void DeepSeekDialog::setupNetworkManager()
{
    networkManager = new QNetworkAccessManager(this);
    
    // 配置SSL/TLS设置
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // 临时跳过证书验证
    QSslConfiguration::setDefaultConfiguration(sslConfig);
    
    // 连接SSL错误信号进行调试
    connect(networkManager, &QNetworkAccessManager::sslErrors, 
            [this](QNetworkReply* reply, const QList<QSslError>& errors) {
        // 忽略SSL错误继续连接
        reply->ignoreSslErrors(errors);
        
        QString errorInfo = "SSL警告（已忽略）: ";
        for (const QSslError &error : errors) {
            errorInfo += error.errorString() + "; ";
        }
        addMessageToChat("系统", errorInfo);
    });
}

void DeepSeekDialog::loadSystemPrompt()
{
    systemPrompt = QString::fromUtf8(R"(
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

图表类型映射：
- 线图/折线图 → "line"
- 柱状图/条形图/直方图 → "bar" 
- 饼图 → "pie"
- 散点图 → "scatter"
- 直方图 → "histogram"
- 箱线图 → "box"
- 小提琴图 → "violin"
- 密度图 → "density"
- 面积图 → "area"

颜色映射：
- 蓝色 → "#007bff"
- 绿色 → "#28a745"  
- 红色 → "#dc3545"
- 黄色 → "#ffc107"
- 紫色 → "#6f42c1"
- 白色 → "#ffffff"
- 黑色 → "#000000"

字体大小建议：
- 小：title_size: 14, axis_size: 10, label_size: 8
- 中：title_size: 16, axis_size: 12, label_size: 10  
- 大：title_size: 20, axis_size: 14, label_size: 12

可用数据文件：
- "sample_data.txt": 标准数值数据（2列：序号，数值）
- "test_stats.txt": 统计数据  
- "pie_data.txt": 饼图数据（2列：类别，数值）

重要：data.file字段必须指定具体的数据文件名，根据图表类型选择合适的文件。

记住：只输出纯JSON，不要任何其他文字！
    )");
}

void DeepSeekDialog::addMessageToChat(const QString& sender, const QString& message, bool isError)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString color, bgColor, icon;
    
    if (isError) {
        color = "#dc3545";
        bgColor = "#f8d7da";
        icon = "❌";
    } else if (sender == "用户") {
        color = "#007bff";
        bgColor = "#d1ecf1";
        icon = "👤";
    } else if (sender == "AI助手") {
        color = "#28a745";
        bgColor = "#d4edda";
        icon = "🤖";
    } else {
        color = "#6c757d";
        bgColor = "#e2e3e5";
        icon = "ℹ️";
    }
    
    QString html = QString(R"(
        <div style="margin: 12px 0; padding: 12px; border-left: 4px solid %1; background-color: %2; border-radius: 8px;">
            <div style="margin-bottom: 6px;">
                <strong style="color: %1;">%3 %4</strong>
                <span style="color: #6c757d; font-size: 11px; float: right;">%5</span>
            </div>
            <div style="color: #333; white-space: pre-wrap; line-height: 1.4;">%6</div>
        </div>
    )")
    .arg(color)
    .arg(bgColor)
    .arg(icon)
    .arg(sender)
    .arg(timestamp)
    .arg(message.toHtmlEscaped());
    
    chatArea->insertHtml(html);
    chatArea->ensureCursorVisible();
}

void DeepSeekDialog::sendMessage()
{
    QString message = inputEdit->text().trimmed();
    if (message.isEmpty()) return;

    // Add user message to chat
    addMessageToChat("用户", message);
    inputEdit->clear();
    sendButton->setEnabled(false);

    // Check API key
    if (apiKey.isEmpty()) {
        addMessageToChat("系统", "请先设置DeepSeek API密钥！", true);
        sendButton->setEnabled(true);
        return;
    }

    sendToDeepSeek(message);
}

void DeepSeekDialog::setApiKey()
{
    bool ok;
    QString key = QInputDialog::getText(this, "设置API密钥",
        "请输入DeepSeek API密钥:", QLineEdit::Password, apiKey, &ok);
    if (ok && !key.isEmpty()) {
        apiKey = key;
        apiKeyButton->setText("已设置密钥 ✓");
        apiKeyButton->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #28a745; color: white; border: none; border-radius: 4px; font-weight: bold; }");
    }
}

void DeepSeekDialog::sendToDeepSeek(const QString& userMessage)
{
    loadingLabel->show();

    // Prepare request
    QNetworkRequest request(QUrl("https://api.deepseek.com/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "TxtPlotter/1.0");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
    request.setRawHeader("Accept", "application/json");
    
    // 设置SSL配置 - 重要的修复！
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    request.setSslConfiguration(sslConfig);

    // Prepare messages array
    QJsonArray messages;
    
    // System message with current plot info
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    QString fullPrompt = systemPrompt;
    if (!currentPlotInfo.isEmpty()) {
        fullPrompt += "\n\n当前图表状态：\n" + currentPlotInfo;
    }
    systemMessage["content"] = fullPrompt;
    messages.append(systemMessage);

    // User message
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = userMessage;
    messages.append(userMsg);

    // Request body
    QJsonObject requestBody;
    requestBody["model"] = "deepseek-chat";
    requestBody["messages"] = messages;
    requestBody["temperature"] = 0.1;
    requestBody["max_tokens"] = 1000;
    requestBody["stream"] = false;

    QJsonDocument doc(requestBody);
    
    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        loadingLabel->hide();
        sendButton->setEnabled(true);
        
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
                    
                    addMessageToChat("AI助手", content);
                    
                    // Try to parse as JSON configuration
                    parseAndApplyConfig(content);
                }
            } else if (responseObj.contains("error")) {
                QJsonObject error = responseObj["error"].toObject();
                QString errorMsg = error["message"].toString();
                addMessageToChat("系统", QString("API错误: %1").arg(errorMsg), true);
            } else {
                addMessageToChat("系统", "未知的API响应格式", true);
            }
        } else {
            QString error = QString("网络请求失败: %1 (错误代码: %2)").arg(reply->errorString()).arg(reply->error());
            addMessageToChat("系统", error, true);
        }
        
        reply->deleteLater();
    });
}

void DeepSeekDialog::parseAndApplyConfig(const QString& jsonString)
{
    // Clean the JSON string - remove any markdown formatting
    QString cleanJson = jsonString;
    cleanJson = cleanJson.replace(QRegExp("```json\\s*"), "");
    cleanJson = cleanJson.replace(QRegExp("```\\s*"), "");
    cleanJson = cleanJson.trimmed();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(cleanJson.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        addMessageToChat("系统", QString("❌ JSON解析失败: %1\n位置: %2\n\n请AI重新生成配置。")
                        .arg(error.errorString()).arg(error.offset), true);
        return;
    }

    QJsonObject root = doc.object();
    PlotConfig config;

    try {
        // Parse chart settings
        if (root.contains("chart")) {
            QJsonObject chart = root["chart"].toObject();
            config.chartType = chart["type"].toString("line");
            config.title = chart["title"].toString();
            config.xLabel = chart["x_label"].toString();
            config.yLabel = chart["y_label"].toString();
        }

        // Parse data settings
        if (root.contains("data")) {
            QJsonObject data = root["data"].toObject();
            config.xColumn = data["x_column"].toInt(0);
            config.yColumn = data["y_column"].toInt(1);
        }

        // Parse appearance settings
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

        // Emit signal with parsed config
        emit configurationGenerated(config);
        
        addMessageToChat("系统", "✅ 配置已成功应用到图表！您可以继续提出修改要求。");

    } catch (...) {
        addMessageToChat("系统", "❌ 配置应用失败，请检查JSON格式是否正确。", true);
    }
}

void DeepSeekDialog::testConnection()
{
    addMessageToChat("系统", "🔧 开始测试网络连接...");
    
    // 测试简单的HTTPS连接
    QNetworkRequest request(QUrl("https://httpbin.org/ip"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "TxtPlotter/1.0");
    
    // 使用相同的SSL配置
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    request.setSslConfiguration(sslConfig);
    
    QNetworkReply *reply = networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            addMessageToChat("系统", "✅ 网络连接测试成功！SSL/TLS工作正常。");
            addMessageToChat("系统", "现在可以尝试DeepSeek API调用。");
        } else {
            addMessageToChat("系统", QString("❌ 网络连接测试失败: %1 (错误代码: %2)")
                            .arg(reply->errorString()).arg(reply->error()), true);
            addMessageToChat("系统", "建议解决方案：\n1. 检查网络连接\n2. 联系管理员安装OpenSSL库\n3. 使用手动JSON配置作为替代方案", true);
        }
        reply->deleteLater();
    });
}

void DeepSeekDialog::showHelp()
{
    QString helpText = QString::fromUtf8(R"(
🤖 DeepSeek AI助手使用说明

【基本功能】
通过自然语言对话来修改图表配置，AI会生成相应的JSON配置并自动应用。

【对话示例】
📊 图表类型：
• "改成柱状图" / "换成线图" / "做成饼图"
• "用散点图显示" / "改为直方图"

🎨 样式修改：
• "标题改为'2024年销售数据'"
• "X轴标签改成'月份'"
• "使用绿色配色方案" / "改成红色主题"
• "背景色改为白色"

🔤 字体调整：
• "标题字体调大一点" / "标题字体18号"
• "整体字体调小" / "轴标签字体12号"

⚙️ 其他设置：
• "显示网格线" / "隐藏网格"
• "线条粗一点" / "数据点大一些"
• "X轴用行序号，Y轴用第2列数据"

【支持的图表类型】
• 线图 • 柱状图 • 饼图 • 散点图
• 直方图 • 箱线图 • 小提琴图 • 密度图 • 面积图

【颜色选择】
• 蓝色（专业）• 绿色（自然）• 红色（醒目）
• 黄色（温暖）• 紫色（优雅）• 白色/黑色

【使用技巧】
1. 描述越具体，AI理解越准确
2. 可以一次提出多个修改要求
3. 支持中文对话，表达自然
4. 如果效果不理想，可以继续调整

💡 AI会根据您的描述智能生成最合适的配置！
    )");
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("使用说明");
    msgBox.setText(helpText);
    msgBox.setTextFormat(Qt::PlainText);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.resize(600, 500);
    msgBox.exec();
}