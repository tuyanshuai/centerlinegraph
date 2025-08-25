#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTextBrowser>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QInputDialog>
#include <QMessageBox>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QApplication>

// Configuration structure for JSON parsing
struct PlotConfig {
    QString chartType = "line";
    QString title = "";
    QString xLabel = "";
    QString yLabel = "";
    int xColumn = 0;
    int yColumn = 1;
    QString backgroundColor = "#f8f9fa";
    QString gridColor = "#e9ecef";
    QString axisColor = "#343a40";
    QString textColor = "#212529";
    QString primaryColor = "#007bff";
    int titleSize = 16;
    int axisSize = 12;
    int labelSize = 10;
    bool showGrid = true;
    int lineWidth = 2;
    int pointSize = 5;
};

class DeepSeekDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeepSeekDialog(QWidget *parent = nullptr);
    void setCurrentPlotInfo(const QString& info);

signals:
    void configurationGenerated(const PlotConfig& config);

private slots:
    void sendMessage();
    void setApiKey();
    void showHelp();
    void testConnection();

private:
    void setupUI();
    void setupNetworkManager();
    void loadSystemPrompt();
    void addMessageToChat(const QString& sender, const QString& message, bool isError = false);
    void sendToDeepSeek(const QString& userMessage);
    void parseAndApplyConfig(const QString& jsonString);

    QTextBrowser *chatArea;
    QLineEdit *inputEdit;
    QPushButton *sendButton;
    QPushButton *apiKeyButton;
    QLabel *loadingLabel;
    QNetworkAccessManager *networkManager;
    
    QString apiKey;
    QString systemPrompt;
    QString currentPlotInfo;
};