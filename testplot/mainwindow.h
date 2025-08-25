#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QScrollArea>
#include <QTextBrowser>
#include <QNetworkAccessManager>
#include <QCheckBox>
#include "plotwidget_new.h"
#include "deepseek_dialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString& initialFile = QString(), QWidget *parent = nullptr);
    void setInitialChartType(ChartType type);

private slots:
    void loadFile();
    void applyColumnSelection();
    void applyMultiColumnSelection();
    void clearPlot();
    void onChartTypeChanged(int buttonId);
    void updateChartTitle();
    void updateAxisLabels();
    void updateStatistics();
    void openDeepSeekDialog();
    void sendAIMessage();
    void exportConfiguration();
    void importConfiguration();
    void onMultiColumnToggled(bool checked);
    void onMultiColumnCheckboxChanged();
    void performDataFitting();

private:
    int scaledSize(int baseSize) const;
    void setupUI();
    void setupMenuBar();
    void connectSignals();
    void setupDeepSeekDialog();
    void loadDataFromFile(const QString& fileName);
    void updateColumnSelectionUI();
    void setupMultiColumnCheckboxes();
    void updateDetailedStatistics(const std::vector<double>& data);
    void createAIChatInterface(QVBoxLayout *layout);
    void processAIRequest(const QString& request);
    void fallbackToDeepSeek(const QString& request);
    void showQuickCommandLinks();
    void parseAndApplyDeepSeekConfig(const QString& jsonString);
    void loadConfigurationFile(const QString& filePath);
    QString generateCurrentPlotInfo();
    QString getCurrentChartTypeName();
    QString getCurrentChartTypeString();
    void setChartTypeByString(const QString& type);
    void applyConfiguration(const PlotConfig& config);
    void applyColorsToPlotWidget(const PlotConfig& config);
    int getCheckedChartTypeId();
    std::vector<double> parseNumbersFromLine(const std::string& line);
    bool validateColumnIndices();
    std::vector<double> polynomialFit(const std::vector<double>& x, const std::vector<double>& y, int degree);
    std::vector<double> gaussianElimination(std::vector<std::vector<double>>& A, std::vector<double>& b);
    std::vector<double> sinusoidalFit(const std::vector<double>& x, const std::vector<double>& y);
    std::vector<double> gaussianFit(const std::vector<double>& x, const std::vector<double>& y);

    // UI components
    PlotWidget *plotWidget;
    QTextEdit *infoText;
    QTextEdit *statsText;
    QLabel *statusLabel;
    QComboBox *chartTypeCombo;
    QLineEdit *titleEdit;
    QLineEdit *xLabelEdit;
    QLineEdit *yLabelEdit;
    QComboBox *xColumnCombo;
    QComboBox *yColumnCombo;
    QPushButton *applyColumnButton;
    QLabel *columnInfoLabel;
    QGroupBox *columnGroup;
    QCheckBox *multiColumnCheckbox;
    QScrollArea *columnCheckboxArea;
    QWidget *columnCheckboxWidget;
    std::vector<QCheckBox*> columnCheckboxes;
    
    // Data fitting components
    QComboBox *fittingCombo;
    QPushButton *fittingButton;
    
    // Data
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
};

#endif // MAINWINDOW_H