#include "envmanager.h"
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QSettings> // 针对 Windows 注册表高级查询（可选）

EnvManager::EnvManager(QObject *parent)
    : QObject{parent}
{}

EnvManager::~EnvManager() {}

// 🌟 核心辅助函数：精准读取 exe 文件的版本号
QString EnvManager::getExeVersion(const QString &exePath)
{
    if (!QFile::exists(exePath)) return "未知版本";

    // 方案一（首选）：直接运行浏览器 --version，速度极快（比 PowerShell 快 10 倍）
    QProcess fastProc;
    fastProc.start(QDir::toNativeSeparators(exePath), QStringList() << "--version");
    if (fastProc.waitForFinished(3000)) {
        QString output = QString::fromUtf8(fastProc.readAllStandardOutput()).trimmed();
        // 输出格式如 "Google Chrome 120.0.6099.110"
        if (!output.isEmpty()) {
            // 从末尾提取版本号：找到最后一个空格，取后面的部分
            int lastSpace = output.lastIndexOf(' ');
            if (lastSpace > 0) {
                QString verStr = output.mid(lastSpace + 1);
                QStringList parts = verStr.split(".");
                if (!parts.isEmpty()) {
                    return parts[0]; // 返回主版本号 "120"
                }
                return verStr;
            }
        }
    }

    // 方案二（备选）：用 PowerShell 读取文件版本属性（部分小众浏览器不支持 --version）
    QProcess proc;
    QString nativePath = QDir::toNativeSeparators(exePath);
    QString cmd = QString("(Get-Item '%1').VersionInfo.FileVersion").arg(nativePath);

    proc.start("powershell", QStringList() << "-NoProfile" << "-Command" << cmd);
    if (proc.waitForFinished(5000)) { // PowerShell 冷启动慢，等 5 秒
        QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        if (!output.isEmpty()) {
            QStringList parts = output.split(".");
            if (!parts.isEmpty()) {
                return parts[0];
            }
            return output;
        }
    }
    return "未知";
}

// 🌟 自动扫描本地浏览器，并附带版本号
QList<BrowserInfo> EnvManager::detectLocalBrowsers()
{
    qDebug() << "核心类：开始自动扫描本地浏览器环境...";
    QList<BrowserInfo> browserList;

    QString progFiles = qEnvironmentVariable("ProgramFiles");
    QString progFilesX86 = qEnvironmentVariable("ProgramFiles(x86)");
    QString localAppData = qEnvironmentVariable("LocalAppData");

    QMap<QString, QStringList> checkPaths;
    checkPaths["Chrome"] = QStringList()
                           << QDir::toNativeSeparators(progFiles + "/Google/Chrome/Application/chrome.exe")
                           << QDir::toNativeSeparators(progFilesX86 + "/Google/Chrome/Application/chrome.exe")
                           << QDir::toNativeSeparators(localAppData + "/Google/Chrome/Application/chrome.exe");

    checkPaths["Edge"] = QStringList()
                         << QDir::toNativeSeparators(progFiles + "/Microsoft/Edge/Application/msedge.exe")
                         << QDir::toNativeSeparators(progFilesX86 + "/Microsoft/Edge/Application/msedge.exe");

    QMap<QString, QStringList>::const_iterator i = checkPaths.constBegin();
    while (i != checkPaths.constEnd()) {
        QString browserName = i.key();
        QStringList paths = i.value();

        for (const QString &path : paths) {
            if (QFile::exists(path)) {
                BrowserInfo info;
                // 🌟 调用版本获取函数
                QString mainVersion = getExeVersion(path);

                // 完美拼接成 AdsPower 同款名字：如 "Chrome 120"
                info.name = QString("%1 %2").arg(browserName).arg(mainVersion);
                info.version = mainVersion;
                info.path = QDir::toNativeSeparators(path);

                browserList.append(info);
                qDebug() << "成功识别到详细浏览器：" << info.name << " -> 路径：" << path;
                break;
            }
        }
        ++i;
    }

    if (browserList.isEmpty()) {
        BrowserInfo info;
        info.name = "SunBrowser (内置默认内核)";
        info.version = "120";
        info.path = "mock_path/sunbrowser.exe";
        browserList.append(info);
    }

    return browserList;
}

// 🌟【精简 MainWindow 的核心功臣】：在此处直接操作 UI 控件
void EnvManager::initBrowserComboBox(QComboBox *comboBox)
{
    if (!comboBox) return;

    // 1. 清空下拉框
    comboBox->clear();

    // 2. 扫描数据
    QList<BrowserInfo> localBrowsers = this->detectLocalBrowsers();

    // 3. 循环填充
    for (const BrowserInfo &browser : localBrowsers) {
        // 展示的是详细名字（如 "Chrome 120"），隐藏绑定的是 exe 路径
        comboBox->addItem(browser.name, browser.path);
    }

    // 4. 默认选中第一项
    if (comboBox->count() > 0) {
        comboBox->setCurrentIndex(0);
    }
}
#include "moc_envmanager.cpp"
