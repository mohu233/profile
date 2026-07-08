#ifndef ENVMANAGER_H
#define ENVMANAGER_H
#include <QObject>
#include <QStringList>
#include <QMap>
#include <QComboBox> //指针
// 结构体：保存浏览器信息
struct BrowserInfo {
    QString name; // 浏览器名字，如 "Chrome", "Edge"
    QString version; //版本
    QString path; // 浏览器的 exe 绝对路径
};


struct EnvironmentInfo
{
    QString name;      // 环境名称
    QString status;    // 运行中/未运行
    QString group;     // 分组
    QString configPath;  // 新增：config.json 的完整路径
    QStringList modules; // 挂载模块
};



class EnvManager : public QObject
{
    Q_OBJECT
public:
    explicit EnvManager(QObject *parent = nullptr);
    ~EnvManager();
    QList<BrowserInfo> detectLocalBrowsers(); // 1. 扫描本地浏览器
    void initBrowserComboBox(QComboBox *comboBox); //一键初始化并绑定下拉框
private:
    // 🌟 新增私有辅助函数：读取本地 .exe 文件的真实版本号
    QString getExeVersion(const QString &exePath);
signals:
};

#endif // ENVMANAGER_H
