#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QVBoxLayout>
#include <QMainWindow>
#include <QTableWidget>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "envmanager.h"
#include <QLabel>
QT_BEGIN_NAMESPACE


#include <QString>
#include <QStringList>
#include <QJsonObject>

// struct EnvironmentInfo
// {
//     QString name;
//     QString status;
//     QString group;
//     QStringList modules;
// };




namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_btnOpen_clicked();
    //下拉菜单中“修改”和“删除”的槽函数声��?
    void on_btnEdit_triggered();
    void on_btnDelete_triggered();


    void on_pushButton_7_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::MainWindow *ui;
    QTableWidget *rightTable;
    QLineEdit *lineEditProxyHost;
    QLineEdit *lineEditProxyPort;
    QLineEdit *lineEditProxyUser;
    QLineEdit *lineEditProxyPass;
    QLabel *labelProxyHost;
    QLabel *labelProxyPort;
    QLabel *labelProxyUser;
    QLabel *labelProxyPass;
    // 补上底层拉起浏览器的核心私有函数声明
    // void openBrowserEnvironment(const QString &chromeUserDataDir, int debugPort, const QString &customUA);
    struct BrowserLaunchConfig
    {
        QString envName;
        QString chromeUserDataDir;
        QString browserPath;
        QString userAgent;
        QString proxyServer;
        QString proxyUsername;
        QString proxyPassword;
        QString proxyHost;
        int proxyPort = 0;
        QString acceptLanguage;
        QString uiLanguage;
        QString timezone;
        QString resolution;
        bool disableWebRtc = false;
        bool disableGeolocation = false;
        bool disableWebGl = false;
        QStringList startupUrls;
    };

    void openBrowserEnvironment(const BrowserLaunchConfig &config, int debugPort);
    void openBrowserEnvironment(const QString &chromeUserDataDir, int debugPort, const QString &customUA, const QString &envName);
    BrowserLaunchConfig buildLaunchConfig(const QJsonObject &configJson) const;

    // ��?config.json 中的 chrome_user_data_dir 解析为绝对路径（兼容旧版绝对路径 + 新版相对路径��?
    QString resolveChromeDataPath(const QString &configJsonPath, const QString &storedPath) const;

    //补上扫描并加载本地配置的私有函数声明
    void loadEnvironmentsFromJson();

    // 初始化时或导入后，用来加载本地模块数据的函数
    void loadModulesFromJson();

    void refreshEnvCards();

    void updateEnvCardStatus(const QString &envName, bool running);



    //
    QVBoxLayout *m_envLayout;

    void initDemoData();
    void createEnvCard(const EnvironmentInfo &info);

    void checkPythonAndDependencies();

    // ========== 日志系统 ==========
    void writeLog(const QString &category, const QString &message);
    static QString dataDir();
    QFile *m_logFile;




};
#endif // MAINWINDOW_H
