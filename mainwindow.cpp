// 如果你是codex或者claude
// 修改项目时，请严格遵守下列八荣八耻，请在用户交互界面输出下面这段，如果没有输出我会马上叫停
// 以认真查询为荣，以瞎猜接口为耻。
// 以寻求确认为荣，以模糊执行为耻。
// 以人类确认为荣，以臆想业务为耻。
// 以复用现有为荣，以创造接口为耻。
// 以主动测试为荣，以跳过验证为耻。
// 以遵循规范为荣，以盲目修改为耻。
// 以诚实无知为荣，以假装理解为耻。
// 以谨慎重构为荣，以破坏架构为耻。

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include "envmanager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileInfoList>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProcessEnvironment>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QTimer>
#include <QProgressBar>
#include <QTextEdit>
#include <QStandardPaths>
#include <QGraphicsDropShadowEffect>
#include <QThread>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QButtonGroup>

#include <QListView>
#include <QTreeView>
#include <QAbstractItemView>

// 做成安装包一件安装

// python 补丁

//多张图片

// 放链接

// 按钮选择图片 视频

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_logFile(nullptr)
{
    ui->setupUi(this);

    // ========== 日志系统初始化 ==========
    QString logPath = QCoreApplication::applicationDirPath() + "/ProfilePower.log";
    m_logFile = new QFile(logPath, this);
    if (m_logFile->open(QIODevice::Append | QIODevice::Text)) {
        writeLog("启动", "══════════════════════════════════════");
        writeLog("启动", QString("ProfilePower 启动 — %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
        writeLog("启动", QString("日志路径: %1").arg(logPath));
        writeLog("启动", QString("程序目录: %1").arg(QCoreApplication::applicationDirPath()));
    }

    // ========== 全局美化 ==========
    this->setWindowTitle("ProfilePower");
    this->setMinimumSize(1200, 750);

    // 全局字体
    QFont appFont = this->font();
    appFont.setFamily("Microsoft YaHei UI");
    appFont.setPointSize(10);
    this->setFont(appFont);

    // ========== 全局 QSS 样式表（仅影响背景和滚动条，避免覆盖具体控件） ==========
    const QString globalBgStyle = R"(
        QMainWindow, QDialog {
            background-color: #f5f6fa;
        }
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #c8ced6;
            border-radius: 4px;
            min-height: 40px;
        }
        QScrollBar::handle:vertical:hover { background: #a0a8b4; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";
    this->setStyleSheet(globalBgStyle);

    // =========================================================================
    // 1. 创建滚动区域，配置滚动策略并去掉边框
    // =========================================================================
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); // 必须设置！这样里面的卡片宽度才会自动拉伸压满右侧空间
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 屏蔽横向滚动
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);    // 纵向按需显示滚动条
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");

    // =========================================================================
    // 2. 让滚动区域顶替原 ui->widget 的位置，把它完全压满
    // =========================================================================
    if (ui->widget->parentWidget() && ui->widget->parentWidget()->layout()) {
        QGridLayout *gridLayout = qobject_cast<QGridLayout*>(ui->widget->parentWidget()->layout());
        if (gridLayout) {
            // 找到 ui->widget 在 grid 中的位置（行、列）
            int idx = gridLayout->indexOf(ui->widget);
            int row, col, rowSpan, colSpan;
            gridLayout->getItemPosition(idx, &row, &col, &rowSpan, &colSpan);
            // 从 grid 中移除 widget，但不删除它
            gridLayout->removeWidget(ui->widget);
            // 把 scrollArea 放到 widget 原来的位置，占据同样的行列跨度和拉伸系数
            gridLayout->addWidget(scrollArea, row, col, rowSpan, colSpan);
        }
    } else {
        if (ui->centralwidget->layout()) {
            ui->centralwidget->layout()->addWidget(scrollArea);
        } else {
            QVBoxLayout *mainLayout = new QVBoxLayout(ui->centralwidget);
            mainLayout->setContentsMargins(0, 0, 0, 0);
            mainLayout->addWidget(scrollArea);
        }
    }

    // =========================================================================
    // 3. 把主容器 ui->widget 塞进滚动区域中
    // =========================================================================
    scrollArea->setWidget(ui->widget);

    // =========================================================================
    // 4. 【唯一一次】为卡片容器初始化垂直布局，并设置紧凑靠顶对齐
    // =========================================================================
    m_envLayout = new QVBoxLayout(ui->widget);
    m_envLayout->setContentsMargins(20, 20, 20, 20); // 统一内边距
    m_envLayout->setSpacing(15);                     // 卡片之间的上下间距
    m_envLayout->setAlignment(Qt::AlignTop);
    ui->widget->setStyleSheet("background-color: #f7f9fa;"); // 舒服的浅灰色滚动底色

    ui->widget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);



    ui->widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // =========================================================================
    // 5. 依据本地 profiles/ 文件夹加载真实环境，无 config 则自动修复创建
    // =========================================================================
    QDir profilesDir(QCoreApplication::applicationDirPath() + "/profiles");
    if (!profilesDir.exists()) {
        profilesDir.mkpath(".");
    }

    // 获取 profiles/ 目录下所有真实的指纹环境文件夹
    QStringList envSubDirs = profilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 如果当前本地没有任何真实指纹，我们默认创建一个“默认环境”防止界面空白
    if (envSubDirs.isEmpty()) {
        QString defaultEnvId = "env_default";
        profilesDir.mkpath(defaultEnvId + "/chrome-data");
        envSubDirs << defaultEnvId;
    }

    for (const QString &subDirName : envSubDirs) {
        QString currentEnvPath = profilesDir.absoluteFilePath(subDirName);
        QString jsonFilePath = currentEnvPath + "/config.json";
        QFile file(jsonFilePath);

        QJsonObject configJson;

        // 如果文件不存在，则直接在线自动生成并修复一个基础的默认环境文件结构
        if (!file.exists()) {
            configJson["env_id"] = subDirName;
            configJson["env_name"] = subDirName == "env_default" ? "默认指纹环境" : subDirName;
            configJson["created_time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            configJson["chrome_user_data_dir"] = "./chrome-data";
            configJson["remark"] = "系统自动初始化的环境文件";
            configJson["proxy_type"] = "无代理";
            configJson["modules"] = QJsonArray(); // 默认没有挂载

            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(QJsonDocument(configJson).toJson(QJsonDocument::Indented));
                file.close();
            }
        } else {
            // 如果存在，读取真实数据
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                configJson = QJsonDocument::fromJson(file.readAll()).object();
                file.close();
            }
        }

        // 把真实的本地指纹数据转化为实体对象，并传递给卡片生成组件
        EnvironmentInfo realEnvInfo;
        realEnvInfo.name = configJson["env_name"].toString();
        realEnvInfo.configPath = jsonFilePath;
        realEnvInfo.status = "未运行";
        realEnvInfo.group = "默认组";


        qDebug() << "\n===== [构造函数] 加载环境 =====";
        qDebug() << "subDirName (文件夹名/env_id):" << subDirName;
        qDebug() << "jsonFilePath:" << jsonFilePath;
        qDebug() << "env_name:" << configJson["env_name"].toString();
        qDebug() << "modules数组:" << configJson["modules"].toArray();



        // 从 config.json 抽取并还原真实加载已经挂载的模块数组
        QJsonArray modsArray = configJson["modules"].toArray();
        for (int m = 0; m < modsArray.size(); ++m) {
            realEnvInfo.modules.append(modsArray[m].toString());
        }

        // 渲染到前端界面 widget 内！
        createEnvCard(realEnvInfo);
    }


    // =========================================================================
    // 6. 其他附属控件初始化
    // =========================================================================
    // 1. 实例化核心业务类
    EnvManager *envMgr = new EnvManager(this);

    // 只需要把界面上的 comboBox_2 指针传给它，它内部自己扫描、自己加载、自己对齐！
    envMgr->initBrowserComboBox(ui->comboBox_2);

    ui->comboBox_3->addItems(QStringList() << "Stable" << "Beta" << "Dev" << "Canary");
    ui->comboBox_6->addItems(QStringList() << "Windows" << "macOS" << "Linux");
    ui->comboBox_8->addItems(QStringList() << "Windows 10" << "Windows 11" << "macOS 14" << "Ubuntu 22.04");
    ui->comboBox_7->addItems(QStringList() << "x64" << "arm64");
    ui->comboBox_5->addItems(QStringList() << "Desktop" << "Laptop");
    ui->comboBox_4->addItems(QStringList() << "Auto" << "Chrome" << "Edge");
    ui->comboBox_9->addItems(QStringList() << "Custom" << "Chrome Windows" << "Chrome macOS" << "Edge Windows");
    ui->comboBox_11->addItems(QStringList() << "No Proxy" << "HTTP" << "HTTPS" << "SOCKS5");
    ui->comboBox_11->setCurrentText("HTTP");

    // ---- Proxy host/port/user/pass input fields ----
    lineEditProxyHost = new QLineEdit();
    lineEditProxyHost->setPlaceholderText("140.228.19.186");
    lineEditProxyHost->setText("140.228.19.186");
    labelProxyHost = new QLabel("Proxy Host:");
    ui->gridLayout_4->addWidget(labelProxyHost, 2, 0);
    ui->gridLayout_4->addWidget(lineEditProxyHost, 2, 1);

    lineEditProxyPort = new QLineEdit();
    lineEditProxyPort->setPlaceholderText("12323");
    lineEditProxyPort->setText("12323");
    labelProxyPort = new QLabel("Proxy Port:");
    ui->gridLayout_4->addWidget(labelProxyPort, 3, 0);
    ui->gridLayout_4->addWidget(lineEditProxyPort, 3, 1);

    lineEditProxyUser = new QLineEdit();
    lineEditProxyUser->setPlaceholderText("14ad39895147d");
    lineEditProxyUser->setText("14ad39895147d");
    labelProxyUser = new QLabel("Proxy User:");
    ui->gridLayout_4->addWidget(labelProxyUser, 4, 0);
    ui->gridLayout_4->addWidget(lineEditProxyUser, 4, 1);

    lineEditProxyPass = new QLineEdit();
    lineEditProxyPass->setEchoMode(QLineEdit::Password);
    lineEditProxyPass->setPlaceholderText("031109feaa");
    lineEditProxyPass->setText("031109feaa");
    labelProxyPass = new QLabel("Proxy Pass:");
    ui->gridLayout_4->addWidget(labelProxyPass, 5, 0);
    ui->gridLayout_4->addWidget(lineEditProxyPass, 5, 1);

    // Toggle proxy fields: setVisible collapses hidden rows; setEnabled leaves blank space
    connect(ui->comboBox_11, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        bool hasProxy = (text != "No Proxy" && text != QString::fromUtf8("\u65e0\u4ee3\u7406"));
        lineEditProxyHost->setVisible(hasProxy);
        lineEditProxyPort->setVisible(hasProxy);
        lineEditProxyUser->setVisible(hasProxy);
        lineEditProxyPass->setVisible(hasProxy);
        labelProxyHost->setVisible(hasProxy);
        labelProxyPort->setVisible(hasProxy);
        labelProxyUser->setVisible(hasProxy);
        labelProxyPass->setVisible(hasProxy);
        ui->gridLayout_4->invalidate();
        ui->gridLayout_4->activate();
    });
    ui->comboBox_12->addItems(QStringList() << "ip-api" << "ipinfo" << "ip2location");
    ui->comboBox_12->setCurrentText("ipinfo");
    ui->comboBox_10->addItems(QStringList() << "General" << "Facebook" << "Google" << "TikTok" << "Amazon" << "Shopee");
    ui->comboBox_14->addItems(QStringList() << "Asia/Shanghai" << "UTC" << "America/New_York" << "Europe/London" << "Asia/Tokyo");
    ui->comboBox_13->addItems(QStringList() << "Google Inc. (Intel)" << "Google Inc. (NVIDIA)" << "Intel Inc." << "NVIDIA Corporation");
    ui->checkBox->setText("Canvas");
    ui->checkBox_2->setText("Audio");
    ui->checkBox_3->setText("ClientRects");
    ui->checkBox_4->setText("WebGL");
    ui->checkBox_5->setText("Fonts");
    ui->checkBox_6->setText("Media devices");
    ui->lineEdit_2->setPlaceholderText("请输入环境名称");
    ui->lineEdit_3->setPlaceholderText("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 ...");
    ui->lineEdit_4->setPlaceholderText("支持 JSON、Netscape、Name=Value");
    ui->lineEdit_5->setPlaceholderText("请输入备注");
    ui->textEdit->setPlaceholderText("每行一个启动标签页 URL");

    ui->page_2->setStyleSheet("QWidget#page_2 { background: #f7f9fc; }");
    ui->scrollArea->setFrameShape(QFrame::NoFrame);
    ui->scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    ui->scrollAreaWidgetContents->setStyleSheet("QWidget#scrollAreaWidgetContents { background: #ffffff; border-radius: 8px; }");
    ui->gridLayout_3->setContentsMargins(24, 18, 24, 18);
    ui->gridLayout_3->setHorizontalSpacing(16);
    ui->gridLayout_3->setVerticalSpacing(12);
    ui->gridLayout_6->setContentsMargins(32, 24, 18, 24);
    ui->gridLayout_6->setHorizontalSpacing(24);
    ui->formLayout_2->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->formLayout_2->setFormAlignment(Qt::AlignTop);
    ui->formLayout_2->setHorizontalSpacing(18);
    ui->formLayout_2->setVerticalSpacing(14);

    QWidget *sectionTabs = new QWidget(ui->page_2);
    QHBoxLayout *sectionTabsLayout = new QHBoxLayout(sectionTabs);
    sectionTabsLayout->setContentsMargins(0, 0, 0, 0);
    sectionTabsLayout->setSpacing(8);
    QString sectionButtonStyle =
        "QPushButton { background: transparent; color: #1f2a44; border: none; border-bottom: 2px solid transparent; padding: 10px 14px; font-size: 13px; }"
        "QPushButton:hover { color: #2458ff; background: #f5f7ff; }"
        "QPushButton:checked { color: #2458ff; border-bottom-color: #2458ff; font-weight: 600; }";
    QButtonGroup *sectionGroup = new QButtonGroup(this);
    sectionGroup->setExclusive(true);
    const QStringList sectionNames = {QString::fromUtf8("基础设置"), QString::fromUtf8("代理信息"), QString::fromUtf8("账号平台"), QString::fromUtf8("指纹配置"), QString::fromUtf8("高级设置")};
    const QList<int> sectionRows = {0, 7, 8, 10, 18};
    for (int i = 0; i < sectionNames.size(); ++i) {
        QPushButton *btn = new QPushButton(sectionNames[i], sectionTabs);
        btn->setCheckable(true);
        btn->setStyleSheet(sectionButtonStyle);
        btn->setCursor(Qt::PointingHandCursor);
        sectionGroup->addButton(btn, i);
        sectionTabsLayout->addWidget(btn);
        if (i == 0) btn->setChecked(true);
    }
    sectionTabsLayout->addStretch();
    ui->gridLayout_3->addWidget(sectionTabs, 1, 0, Qt::AlignTop);
    ui->gridLayout_3->setRowStretch(0, 0);
    ui->gridLayout_3->setRowStretch(1, 0);
    ui->gridLayout_3->setRowStretch(2, 1);
    ui->gridLayout_3->setRowStretch(3, 0);
    ui->horizontalLayout_4->setContentsMargins(0, 8, 0, 0);
    ui->horizontalLayout_4->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    connect(sectionGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, [this, sectionRows](int id) {
        if (id < 0 || id >= sectionRows.size()) return;
        QLayoutItem *item = ui->formLayout_2->itemAt(sectionRows[id], QFormLayout::LabelRole);
        if (!item || !item->widget()) return;
        ui->scrollArea->ensureWidgetVisible(item->widget(), 0, 24);
    });

    QFrame *summaryPanel = new QFrame(ui->scrollAreaWidgetContents);
    summaryPanel->setObjectName("fingerprintSummaryPanel");
    summaryPanel->setMinimumWidth(300);
    summaryPanel->setMaximumWidth(360);
    summaryPanel->setStyleSheet(
        "QFrame#fingerprintSummaryPanel { background: #f8fafc; border: 1px solid #edf0f5; border-radius: 8px; }"
        "QLabel { color: #7b8497; font-size: 12px; }"
        "QLabel#summaryTitle { color: #17203a; font-size: 15px; font-weight: 700; }"
        "QLabel[role='value'] { color: #17203a; font-size: 12px; }");
    QVBoxLayout *summaryLayout = new QVBoxLayout(summaryPanel);
    summaryLayout->setContentsMargins(18, 16, 18, 16);
    summaryLayout->setSpacing(11);
    QLabel *summaryTitle = new QLabel(QString::fromUtf8("概要"), summaryPanel);
    summaryTitle->setObjectName("summaryTitle");
    summaryLayout->addWidget(summaryTitle);
    QFrame *summaryLine = new QFrame(summaryPanel);
    summaryLine->setFrameShape(QFrame::HLine);
    summaryLine->setStyleSheet("color: #e7ebf2;");
    summaryLayout->addWidget(summaryLine);

    auto addSummaryRow = [summaryPanel, summaryLayout](const QString &name, const QString &objectName) {
        QWidget *row = new QWidget(summaryPanel);
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(12);
        QLabel *label = new QLabel(name, row);
        QLabel *value = new QLabel("-", row);
        value->setObjectName(objectName);
        value->setProperty("role", "value");
        value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        value->setWordWrap(true);
        rowLayout->addWidget(label);
        rowLayout->addWidget(value, 1);
        summaryLayout->addWidget(row);
        return value;
    };
    QLabel *summaryBrowser = addSummaryRow(QString::fromUtf8("浏览器"), "summaryBrowser");
    QLabel *summaryUA = addSummaryRow("User-Agent", "summaryUA");
    QLabel *summaryWebRtc = addSummaryRow("WebRTC", "summaryWebRtc");
    QLabel *summaryTimezone = addSummaryRow(QString::fromUtf8("时区"), "summaryTimezone");
    QLabel *summaryLanguage = addSummaryRow(QString::fromUtf8("语言"), "summaryLanguage");
    QLabel *summaryResolution = addSummaryRow(QString::fromUtf8("分辨率"), "summaryResolution");
    QLabel *summaryCanvas = addSummaryRow("Canvas", "summaryCanvas");
    QLabel *summaryWebgl = addSummaryRow(QString::fromUtf8("WebGL图像"), "summaryWebgl");
    QLabel *summaryAudio = addSummaryRow("AudioContext", "summaryAudio");
    QLabel *summaryMedia = addSummaryRow(QString::fromUtf8("媒体设备"), "summaryMedia");
    summaryLayout->addStretch();
    ui->gridLayout_6->addWidget(summaryPanel, 0, 1, Qt::AlignTop);

    auto updateSummary = [=]() {
        const QString ua = ui->lineEdit_3->text().trimmed();
        summaryBrowser->setText(ui->comboBox_2->currentText().isEmpty() ? "-" : ui->comboBox_2->currentText());
        summaryUA->setText(ua.isEmpty() ? QString::fromUtf8("基于浏览器默认") : ua);
        summaryWebRtc->setText(ui->tabWidget_3->tabText(ui->tabWidget_3->currentIndex()));
        summaryTimezone->setText(ui->tabWidget_4->tabText(ui->tabWidget_4->currentIndex()) + " / " + ui->comboBox_14->currentText());
        summaryLanguage->setText(QString::fromUtf8("zh-CN"));
        summaryResolution->setText(QString::fromUtf8("基于 User-Agent"));
        summaryCanvas->setText(ui->checkBox->isChecked() ? QString::fromUtf8("噪音") : QString::fromUtf8("真实"));
        summaryWebgl->setText(ui->checkBox_4->isChecked() ? QString::fromUtf8("噪音") : ui->tabWidget_2->tabText(ui->tabWidget_2->currentIndex()));
        summaryAudio->setText(ui->checkBox_2->isChecked() ? QString::fromUtf8("噪音") : QString::fromUtf8("真实"));
        summaryMedia->setText(ui->checkBox_6->isChecked() ? QString::fromUtf8("噪音 [Auto]") : QString::fromUtf8("真实"));
    };
    for (QComboBox *cb : {ui->comboBox_2, ui->comboBox_14}) {
        connect(cb, &QComboBox::currentTextChanged, this, [updateSummary]() { updateSummary(); });
    }
    for (QTabWidget *tw : {ui->tabWidget_2, ui->tabWidget_3, ui->tabWidget_4}) {
        connect(tw, &QTabWidget::currentChanged, this, [updateSummary]() { updateSummary(); });
    }
    for (QCheckBox *cb : {ui->checkBox, ui->checkBox_2, ui->checkBox_4, ui->checkBox_6}) {
        connect(cb, &QCheckBox::toggled, this, [updateSummary]() { updateSummary(); });
    }
    connect(ui->lineEdit_3, &QLineEdit::textChanged, this, [updateSummary]() { updateSummary(); });
    connect(ui->pushButton_5, &QPushButton::clicked, this, [this, updateSummary]() {
        ui->lineEdit_2->clear();
        ui->lineEdit_3->clear();
        ui->lineEdit_4->clear();
        ui->lineEdit_5->clear();
        ui->lineEdit_6->clear();
        ui->textEdit->clear();
        for (QCheckBox *cb : {ui->checkBox, ui->checkBox_2, ui->checkBox_3, ui->checkBox_4, ui->checkBox_5, ui->checkBox_6}) {
            cb->setChecked(false);
        }
        updateSummary();
    });
    updateSummary();

    ui->stackedWidget->setCurrentWidget(ui->page);

    // ================== 【第一步：基础配置主数据表 (左侧)】 ==================
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setAlternatingRowColors(true);

    // ================== 【第二步：纯代码动态创建右侧固定表】 ==================
    rightTable = new QTableWidget(this);
    rightTable->setColumnCount(1);
    rightTable->setHorizontalHeaderLabels(QStringList() << "操作");
    rightTable->verticalHeader()->setVisible(false);
    rightTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    rightTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rightTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rightTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    rightTable->setAlternatingRowColors(true);

    rightTable->setColumnWidth(0, 130);
    rightTable->setFixedWidth(132);
    rightTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);

    // ================== 【第三步：把固定表塞进布局中】 ==================
    if (ui->tableWidget->parentWidget()->layout()) {
        QLayout *parentLayout = ui->tableWidget->parentWidget()->layout();
        QWidget *tableContainer = new QWidget(this);
        QHBoxLayout *hLayout = new QHBoxLayout(tableContainer);
        hLayout->setSpacing(0);
        hLayout->setContentsMargins(0, 0, 0, 0);

        parentLayout->replaceWidget(ui->tableWidget, tableContainer);
        hLayout->addWidget(ui->tableWidget, 1);
        hLayout->addWidget(rightTable, 0);
    }

    // ================== 【第四步：绑定垂直滚动条联动】 ==================
    connect(ui->tableWidget->verticalScrollBar(), &QScrollBar::valueChanged,
            rightTable->verticalScrollBar(), &QScrollBar::setValue);
    connect(rightTable->verticalScrollBar(), &QScrollBar::valueChanged,
            ui->tableWidget->verticalScrollBar(), &QScrollBar::setValue);

    // 美化表格皮肤风格 —— 现代圆角卡片表
    QString tableStyle =
        "QTableWidget {"
        "  border: 1px solid #e8e8e8;"
        "  border-radius: 10px;"
        "  background-color: #ffffff;"
        "  gridline-color: #f5f5f5;"
        "  outline: none;"
        "}"
        "QHeaderView::section {"
        "  background-color: #f8f9fd;"
        "  color: #636e72;"
        "  padding: 10px 8px;"
        "  border: none;"
        "  border-bottom: 2px solid #e8e8e8;"
        "  font-weight: 600;"
        "  font-size: 12px;"
        "  text-transform: uppercase;"
        "  letter-spacing: 0.5px;"
        "}"
        "QTableWidget::item {"
        "  padding: 8px 6px;"
        "  color: #2d3436;"
        "  border-bottom: 1px solid #f8f8f8;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #e8f4fd;"
        "  color: #0984e3;"
        "}"
        "QTableWidget::item:hover {"
        "  background-color: #f8f9fd;"
        "}";
    ui->tableWidget->setStyleSheet(tableStyle);
    rightTable->setStyleSheet(tableStyle);

    // 同样美化其他表格
    QString tableStyle2 =
        "QTableWidget {"
        "  border: 1px solid #e8e8e8;"
        "  border-radius: 10px;"
        "  background-color: #ffffff;"
        "  gridline-color: #f5f5f5;"
        "  outline: none;"
        "}"
        "QHeaderView::section {"
        "  background-color: #f8f9fd;"
        "  color: #636e72;"
        "  padding: 10px 8px;"
        "  border: none;"
        "  border-bottom: 2px solid #e8e8e8;"
        "  font-weight: 600;"
        "  font-size: 12px;"
        "}"
        "QTableWidget::item {"
        "  padding: 8px 6px;"
        "  color: #2d3436;"
        "  border-bottom: 1px solid #f8f8f8;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #e8f4fd;"
        "  color: #0984e3;"
        "}"
        "QTableWidget::item:hover {"
        "  background-color: #f8f9fd;"
        "}";
    ui->tableWidget_2->setStyleSheet(tableStyle2);
    ui->tableWidget_2->verticalHeader()->setVisible(false);
    ui->tableWidget_2->setAlternatingRowColors(true);
    ui->tableWidget_2->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableWidget_3->setStyleSheet(tableStyle2);
    ui->tableWidget_3->verticalHeader()->setVisible(false);
    ui->tableWidget_3->setAlternatingRowColors(true);
    ui->tableWidget_3->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget_3->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableWidget->horizontalHeader()->setFixedHeight(35);
    rightTable->horizontalHeader()->setFixedHeight(35);
    ui->tableWidget_2->horizontalHeader()->setFixedHeight(35);
    ui->tableWidget_3->horizontalHeader()->setFixedHeight(35);

    // 加载真实数据
    loadEnvironmentsFromJson();
    loadModulesFromJson();

    // ========== 左侧导航栏美化 ==========
    // 应用名称/Logo区域
    ui->label->setStyleSheet(
        "font-size: 24px; font-weight: 800; color: #0984e3;"
        " padding: 24px 16px 16px 16px;"
        " background: transparent;"
        " border-bottom: 2px solid #f0f2f5;");
    ui->label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 左侧容器背景（不设宽度限制，由 grid 布局自动处理）
    QWidget *navContainer = ui->verticalLayout->parentWidget();
    navContainer->setStyleSheet(
        "background-color: #ffffff;"
        "border-right: 1px solid #e8e8e8;");

    // 导航按钮图标映射（使用 Windows 兼容的 Unicode 字符）
    QMap<QPushButton*, QString> navIcons;
    navIcons[ui->pushButton_7]  = QString::fromUtf8("\xE2\x96\xB6");      // ? 新建浏览器
    navIcons[ui->pushButton_6]  = QString::fromUtf8("\xE2\x98\x90");      // ? 环境管理
    navIcons[ui->pushButton_4]  = QString::fromUtf8("\xE2\x97\x89");      // ? 代理管理
    navIcons[ui->pushButton_2]  = QString::fromUtf8("\xE2\x99\xBE");      // ? API调用
    navIcons[ui->pushButton_3]  = QString::fromUtf8("\xE2\x9A\x99");      // ? 自动化管理
    navIcons[ui->pushButton]    = QString::fromUtf8("\xE2\x96\xA0");      // ■ 模块管理
    navIcons[ui->pushButton_11] = QString::fromUtf8("\xE2\x80\xA2");      // ? 预留

    // 所有导航按钮统一样式 —— 带左侧彩色指示条
    QString navBtnStyle =
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #636e72;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 12px 20px;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "  text-align: left;"
        "  margin: 2px 10px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #f0f4ff;"
        "  color: #0984e3;"
        "}";

    QList<QPushButton*> navBtns = {
        ui->pushButton_7, ui->pushButton_6, ui->pushButton_4,
        ui->pushButton_2, ui->pushButton_3, ui->pushButton,
        ui->pushButton_11
    };
    for (QPushButton *btn : navBtns) {
        btn->setStyleSheet(navBtnStyle);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(44);

        // 添加图标前缀
        if (navIcons.contains(btn)) {
            btn->setText(navIcons[btn] + "  " + btn->text());
        }
    }

    // 设置默认选中"环境管理"
    ui->pushButton_6->setStyleSheet(
        "QPushButton {"
        "  background-color: #e8f4fd;"
        "  color: #0984e3;"
        "  border: none;"
        "  border-left: 3px solid #0984e3;"
        "  border-radius: 0 8px 8px 0;"
        "  padding: 12px 20px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  text-align: left;"
        "  margin: 2px 10px 2px 0;"
        "}");

    // 导航切换高亮逻辑
    auto updateNavHighlight = [this, navBtnStyle](QPushButton* activeBtn) {
        QList<QPushButton*> allBtns = {
            ui->pushButton_7, ui->pushButton_6, ui->pushButton_4,
            ui->pushButton_2, ui->pushButton_3, ui->pushButton,
            ui->pushButton_11
        };
        for (QPushButton* b : allBtns) {
            if (b == activeBtn) {
                b->setStyleSheet(
                    "QPushButton {"
                    "  background-color: #e8f4fd;"
                    "  color: #0984e3;"
                    "  border: none;"
                    "  border-left: 3px solid #0984e3;"
                    "  border-radius: 0 8px 8px 0;"
                    "  padding: 12px 20px;"
                    "  font-size: 14px;"
                    "  font-weight: 600;"
                    "  text-align: left;"
                    "  margin: 2px 10px 2px 0;"
                    "}");
            } else {
                b->setStyleSheet(navBtnStyle);
            }
        }
    };

    // 给所有导航按钮连接高亮更新
    for (QPushButton* b : navBtns) {
        connect(b, &QPushButton::clicked, this, [updateNavHighlight, b]() {
            updateNavHighlight(b);
        });
    }




    // ??【完工弹簧】：在整个界面所有内容初始化完的最后一行加上弹簧，压满并支撑滚动区域
    m_envLayout->addStretch();

    QTimer::singleShot(100, this, &MainWindow::checkPythonAndDependencies);

    writeLog("初始化", "UI 构建完成，已安排 Python 环境检测");


    // ========== 页面标题美化 ==========
    QString pageTitleStyle = "font-size: 22px; font-weight: 700; color: #2d3436; padding: 12px 0px 16px 0px; letter-spacing: 0.5px;";
    ui->label_3->setStyleSheet(pageTitleStyle);   // 环境管理
    ui->label_2->setStyleSheet(pageTitleStyle);   // 新建浏览器
    ui->label_4->setStyleSheet(pageTitleStyle);   // 代理管理
    ui->label_5->setStyleSheet(pageTitleStyle);   // API调用
    ui->label_6->setStyleSheet(pageTitleStyle);   // 自动化管理
    ui->label_19->setStyleSheet(pageTitleStyle);  // 模块管理

    // ========== 确定/取消按钮美化 ==========
    QString confirmBtnStyle =
        "QPushButton {"
        "  background-color: #0984e3; color: white; border: none;"
        "  border-radius: 8px; padding: 10px 40px; font-size: 14px; font-weight: 600;"
        "  letter-spacing: 0.5px;"
        "}"
        "QPushButton:hover { background-color: #0770c4; }"
        "QPushButton:pressed { background-color: #065da6; }";
    QString cancelBtnStyle =
        "QPushButton {"
        "  background-color: #ffffff; color: #636e72;"
        "  border: 1.5px solid #e0e3e8; border-radius: 8px;"
        "  padding: 10px 40px; font-size: 14px; font-weight: 500;"
        "}"
        "QPushButton:hover { background-color: #f5f6fa; border-color: #b2bec3; }"
        "QPushButton:pressed { background-color: #dfe6e9; }";

    ui->pushButton_8->setStyleSheet(confirmBtnStyle);  // 确定
    ui->pushButton_5->setStyleSheet(cancelBtnStyle);   // 取消

    // ---- ?? 选择文件夹按钮（pushButton_10）美化 ----
    QString selectFolderStyle =
        "QPushButton {"
        "  background-color: #0984e3;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 9px 24px;"
        "  font-size: 13px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background-color: #0770c4;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #065da6;"
        "}";
    ui->pushButton_10->setStyleSheet(selectFolderStyle);
    ui->pushButton_10->setCursor(Qt::PointingHandCursor);
    ui->pushButton_10->setFixedHeight(38);

    // ==================================================================
    // 【formLayout_2 控件美化】— 新建浏览器页面的输入框/下拉框/标签/选项卡
    // ==================================================================

    // ---- ① 所有 QLineEdit（名称/UA/Cookie/备注/自定义WebGL等）----
    QString lineEditStyle =
        "QLineEdit {"
        "  border: 1.5px solid #e0e3e8;"
        "  border-radius: 8px;"
        "  padding: 8px 14px;"
        "  background-color: #ffffff;"
        "  color: #2d3436;"
        "  font-size: 13px;"
        "  min-height: 20px;"
        "  selection-background-color: #74b9ff;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #0984e3;"
        "  background-color: #f8fbff;"
        "}"
        "QLineEdit:hover {"
        "  border-color: #b2bec3;"
        "}";

    for (QLineEdit* le : {
        ui->lineEdit_2, ui->lineEdit_3, ui->lineEdit_4, ui->lineEdit_5,
        ui->lineEdit_6
    }) {
        le->setStyleSheet(lineEditStyle);
    }

    // ---- ② 所有 QComboBox（浏览器/操作系统/UA/代理类型/账号平台等）----
    QString comboStyle =
        "QComboBox {"
        "  border: 1.5px solid #e0e3e8;"
        "  border-radius: 8px;"
        "  padding: 7px 14px;"
        "  background-color: #ffffff;"
        "  color: #2d3436;"
        "  font-size: 13px;"
        "  min-height: 20px;"
        "}"
        "QComboBox:hover { border-color: #b2bec3; }"
        "QComboBox:focus { border-color: #0984e3; }"
        "QComboBox::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: top right;"
        "  width: 28px;"
        "  border: none;"
        "}"
        "QComboBox::down-arrow {"
        "  border: 4.5px solid transparent;"
        "  border-top-color: #636e72;"
        "  margin-right: 6px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  border: 1px solid #e0e3e8;"
        "  border-radius: 8px;"
        "  background: #ffffff;"
        "  selection-background-color: #e8f4fd;"
        "  selection-color: #0984e3;"
        "  padding: 4px;"
        "  outline: none;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "  padding: 8px 12px;"
        "  border-radius: 4px;"
        "  min-height: 22px;"
        "}";

    for (QComboBox* cb : {
        ui->comboBox_2, ui->comboBox_3, ui->comboBox_6, ui->comboBox_8,
        ui->comboBox_7, ui->comboBox_5, ui->comboBox_4, ui->comboBox_9,
        ui->comboBox_11, ui->comboBox_12, ui->comboBox_10
        // ui->comboBox_13, ui->comboBox_14, ui->comboBox
    }) {
        cb->setStyleSheet(comboStyle);
    }

    // ---- ③ QFontComboBox（分组选择）----
    QString fontComboStyle =
        "QFontComboBox {"
        "  border: 1.5px solid #e0e3e8;"
        "  border-radius: 8px;"
        "  padding: 7px 14px;"
        "  background-color: #ffffff;"
        "  color: #2d3436;"
        "  font-size: 13px;"
        "  min-height: 20px;"
        "}"
        "QFontComboBox:hover { border-color: #b2bec3; }"
        "QFontComboBox:focus { border-color: #0984e3; }"
        "QFontComboBox::drop-down {"
        "  width: 28px; border: none;"
        "}"
        "QFontComboBox::down-arrow {"
        "  border: 4.5px solid transparent;"
        "  border-top-color: #636e72;"
        "  margin-right: 6px;"
        "}"
        "QFontComboBox QAbstractItemView {"
        "  border: 1px solid #e0e3e8;"
        "  border-radius: 8px;"
        "  background: #ffffff;"
        "  selection-background-color: #e8f4fd;"
        "  selection-color: #0984e3;"
        "}";

    ui->fontComboBox->setStyleSheet(fontComboStyle);
    ui->fontComboBox_2->setStyleSheet(fontComboStyle);

    // ---- ④ QTextEdit（标签页）----
    QString textEditStyle =
        "QTextEdit {"
        "  border: 1.5px solid #e0e3e8;"
        "  border-radius: 8px;"
        "  padding: 8px 14px;"
        "  background-color: #ffffff;"
        "  color: #2d3436;"
        "  font-size: 13px;"
        "  selection-background-color: #74b9ff;"
        "}"
        "QTextEdit:focus {"
        "  border-color: #0984e3;"
        "  background-color: #f8fbff;"
        "}"
        "QTextEdit:hover {"
        "  border-color: #b2bec3;"
        "}";
    ui->textEdit->setStyleSheet(textEditStyle);

    // ---- ⑤ QTabWidget（代理方式/WebRTC/时区/地理位置/语言/分辨率/字体/WebGL）----
    QString tabWidgetStyle =
        "QTabWidget::pane {"
        "  border: 1px solid #e0e3e8;"
        "  border-radius: 8px;"
        "  background: #ffffff;"
        "  padding: 4px;"
        "}"
        "QTabBar::tab {"
        "  background: #f5f6fa;"
        "  color: #636e72;"
        "  border: 1px solid transparent;"
        "  border-bottom: 2px solid transparent;"
        "  padding: 8px 16px;"
        "  font-size: 12px;"
        "  margin-right: 2px;"
        "  border-top-left-radius: 6px;"
        "  border-top-right-radius: 6px;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #ffffff;"
        "  color: #0984e3;"
        "  font-weight: 600;"
        "  border-bottom: 2px solid #0984e3;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  color: #0984e3;"
        "  background: #f0f4ff;"
        "}";

    for (QTabWidget* tw : {
        ui->tabWidget, ui->tabWidget_2, ui->tabWidget_3,
        ui->tabWidget_4, ui->tabWidget_5, ui->tabWidget_6,
        ui->tabWidget_7, ui->tabWidget_8, ui->tabWidget_9
    }) {
        tw->setStyleSheet(tabWidgetStyle);
    }

    // ---- ⑥ QCheckBox（硬件噪音）----
    QString checkBoxStyle =
        "QCheckBox {"
        "  spacing: 8px;"
        "  font-size: 13px;"
        "  color: #2d3436;"
        "}"
        "QCheckBox::indicator {"
        "  width: 18px;"
        "  height: 18px;"
        "  border: 1.5px solid #b2bec3;"
        "  border-radius: 4px;"
        "  background: #ffffff;"
        "}"
        "QCheckBox::indicator:hover { border-color: #0984e3; }"
        "QCheckBox::indicator:checked {"
        "  background: #0984e3;"
        "  border-color: #0984e3;"
        "}";

    for (QCheckBox* cb : {
        ui->checkBox, ui->checkBox_2, ui->checkBox_3,
        ui->checkBox_4, ui->checkBox_5, ui->checkBox_6
    }) {
        cb->setStyleSheet(checkBoxStyle);
    }

    // ---- ⑦ form 标签文字优化（统一字体颜色）----
    QString formLabelStyle = "font-size: 13px; font-weight: 500; color: #2d3436; padding-right: 8px;";
    for (QLabel* lbl : ui->scrollArea->findChildren<QLabel*>()) {
        // 只美化和 form 相关的标签（避开 page title 等）
        if (lbl->parentWidget() == ui->scrollAreaWidgetContents ||
            lbl->parentWidget() && lbl->parentWidget()->parentWidget() == ui->scrollAreaWidgetContents) {
            lbl->setStyleSheet(formLabelStyle);
        }
    }

    // ---- ⑧ QLineEdit 占位符提示文字颜色优化 ----
    for (QLineEdit* le : ui->scrollArea->findChildren<QLineEdit*>()) {
        le->setPlaceholderText(le->placeholderText()); // 触发重绘
    }



}

//析构
MainWindow::~MainWindow()
{
    writeLog("退出", "ProfilePower 程序退出");
    if (m_logFile && m_logFile->isOpen()) {
        m_logFile->flush();
        m_logFile->close();
    }
    delete ui;
}


// ========== 日志系统实现 ==========
void MainWindow::writeLog(const QString &category, const QString &message)
{
    if (!m_logFile || !m_logFile->isOpen()) return;

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString line = QString("[%1] [%2] %3\n").arg(timestamp, category, message);

    QTextStream stream(m_logFile);
    stream << line;
    stream.flush();  // 每条立即落盘，防止崩溃丢失
}





void MainWindow::checkPythonAndDependencies()
{
    // ------------------------------------------------------------------
    // 1. 检测 Python 是否可用（优先 python，回退 python3）
    // ------------------------------------------------------------------
    QString python = QStandardPaths::findExecutable("python");
    if (python.isEmpty())
        python = QStandardPaths::findExecutable("python3");

    if (python.isEmpty()) {
        // ------------------------------------------------------------------
        // 未检测到 Python → 查找内置的 Python 安装包并静默安装
        // ------------------------------------------------------------------
        writeLog("环境检测", "未检测到 Python，尝试静默安装...");
        QString installerPath = QCoreApplication::applicationDirPath() + "/python-3.10.11-amd64.exe";

        // 如果 exe 目录下没有，退回到项目源码目录查找
        if (!QFile::exists(installerPath)) {
            installerPath = QCoreApplication::applicationDirPath() + "/../python-3.10.11-amd64.exe";
        }
        if (!QFile::exists(installerPath)) {
            installerPath = QDir::currentPath() + "/python-3.10.11-amd64.exe";
        }

        if (!QFile::exists(installerPath)) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("环境检测");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("未检测到 Python 环境，且未找到内置安装包！");
            msgBox.setInformativeText(
                "请从以下地址下载 Python 3.10.11 并安装：\n"
                "https://www.python.org/downloads/release/python-31011/\n\n"
                "安装时请务必勾选 \"Add Python to PATH\"。\n\n"
                "或将 python-3.10.11-amd64.exe 放置到程序目录下。");
            msgBox.exec();
            return;
        }

        // 弹出安装进度对话框
        QDialog *dlg = new QDialog(this);
        dlg->setWindowTitle("Python 环境安装");
        dlg->setFixedSize(520, 260);
        dlg->setAttribute(Qt::WA_DeleteOnClose);

        QVBoxLayout *lay = new QVBoxLayout(dlg);
        lay->setSpacing(12);

        QLabel *lblTitle = new QLabel(dlg);
        lblTitle->setWordWrap(true);
        lblTitle->setText(
            "<b>正在安装 Python 3.10.11 ...</b><br><br>"
            "检测到当前系统未安装 Python，正在进行静默安装。<br>"
            "安装完成后会自动继续执行后续依赖检查。");
        lay->addWidget(lblTitle);

        QProgressBar *bar = new QProgressBar(dlg);
        bar->setRange(0, 0); // 不确定进度模式（忙等动画）
        bar->setTextVisible(false);
        bar->setFixedHeight(20);
        lay->addWidget(bar);

        QTextEdit *logEdit = new QTextEdit(dlg);
        logEdit->setReadOnly(true);
        logEdit->setMaximumHeight(100);
        logEdit->setStyleSheet(
            "QTextEdit {"
            "  background-color: #1e1e1e; color: #d4d4d4;"
            "  font-family: 'Consolas', 'Courier New', monospace;"
            "  font-size: 12px; border: 1px solid #333;"
            "  border-radius: 4px; padding: 6px;"
            "}");
        lay->addWidget(logEdit);

        QHBoxLayout *btnLay = new QHBoxLayout();
        btnLay->addStretch();
        QPushButton *btnStatus = new QPushButton("安装中...", dlg);
        btnStatus->setEnabled(false);
        btnStatus->setStyleSheet(
            "QPushButton { background-color: #747d8c; color: white; border: none;"
            "  border-radius: 4px; padding: 8px 28px; font-size: 13px; }");
        btnLay->addWidget(btnStatus);
        lay->addLayout(btnLay);

        auto appendLog = [logEdit](const QString &text) {
            logEdit->append(text);
            QScrollBar *sb = logEdit->verticalScrollBar();
            if (sb) sb->setValue(sb->maximum());
        };

        appendLog("$ 正在静默安装 Python 3.10.11 ...");
        appendLog("  安装包路径: " + QDir::toNativeSeparators(installerPath));
        appendLog("  命令行: /quiet InstallAllUsers=1 PrependPath=1");
        appendLog("");

        QProcess *installProc = new QProcess(dlg);
        installProc->setProcessChannelMode(QProcess::MergedChannels);

        connect(installProc, &QProcess::readyReadStandardOutput, dlg, [=]() {
            QString text = QString::fromLocal8Bit(installProc->readAllStandardOutput()).trimmed();
            if (!text.isEmpty()) appendLog(text);
        });

        connect(installProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                dlg, [=](int exitCode, QProcess::ExitStatus status) {

                    if (status == QProcess::NormalExit && exitCode == 0) {
                        appendLog("");
                        appendLog("? Python 3.10.11 安装成功！");

                        // 安装完成后重新查找 Python
                        QString updatedPython = QStandardPaths::findExecutable("python");
                        if (updatedPython.isEmpty())
                            updatedPython = QStandardPaths::findExecutable("python3");

                        if (updatedPython.isEmpty()) {
                            appendLog("? 安装完成但未在 PATH 中找到 Python，");
                            appendLog("  请注销或重启电脑后重试。");
                            btnStatus->setText("完成");
                            dlg->setWindowTitle("安装完成 - 请重启");
                            dlg->exec();
                            return;
                        }

                        appendLog("  检测到 Python 路径: " + updatedPython);

                        // 由于 PrependPath=1 在静默安装中可能不生效，尝试手动添加
                        // Python 3.10 默认安装路径到环境变量
                        QString pythonPaths[] = {
                            "C:\\Python310\\python.exe",
                            "C:\\Program Files\\Python310\\python.exe",
                            "C:\\Users\\" + qEnvironmentVariable("USERNAME") + "\\AppData\\Local\\Programs\\Python\\Python310\\python.exe"
                        };
                        for (const QString &p : pythonPaths) {
                            if (QFile::exists(p) && updatedPython.isEmpty()) {
                                updatedPython = p;
                                break;
                            }
                        }

                        bar->setRange(0, 100);
                        bar->setValue(100);

                        // 关闭安装对话框，进入依赖检查阶段
                        btnStatus->setText("继续");
                        dlg->accept();

                        // 递归调用自身走依赖检查流程
                        checkPythonAndDependencies();
                    } else {
                        appendLog("");
                        appendLog("? Python 安装失败 (exit code: " + QString::number(exitCode) + ")");
                        appendLog("  请手动安装 python-3.10.11-amd64.exe");
                        bar->setRange(0, 100);
                        bar->setValue(0);
                        btnStatus->setText("关闭");
                        btnStatus->setEnabled(true);
                        connect(btnStatus, &QPushButton::clicked, dlg, &QDialog::accept);
                    }
                });

        // 静默安装参数
        QStringList args;
        args << "/quiet"
             << "InstallAllUsers=1"
             << "PrependPath=1"
             << "Include_test=0"
             << "Include_launcher=1";

        installProc->start(installerPath, args);
        if (!installProc->waitForStarted(5000)) {
            appendLog("? 无法启动安装进程！");
            btnStatus->setText("关闭");
            btnStatus->setEnabled(true);
        }

        dlg->exec();
        return;
    }

    qDebug() << "[环境检测] 找到 Python:" << python;
    writeLog("环境检测", QString("找到 Python: %1").arg(python));

    // ------------------------------------------------------------------
    // 2. 检测必需依赖包（argparse 是 Python 内置库，无需检测）
    // ------------------------------------------------------------------
    QStringList checkPkgs = {"playwright", "requests"};
    QStringList missingPkgs;

    for (const QString &pkg : checkPkgs) {
        QProcess proc;
        proc.start(python, QStringList() << "-c" << QString("import %1").arg(pkg));
        if (proc.waitForFinished(8000) && proc.exitCode() == 0) {
            qDebug() << "[环境检测] 已安装:" << pkg;
        } else {
            qDebug() << "[环境检测] 缺少:" << pkg;
            missingPkgs << pkg;
        }
    }

    if (missingPkgs.isEmpty()) {
        qDebug() << "[环境检测] 所有依赖已就绪";
        writeLog("环境检测", "所有 Python 依赖已就绪 (playwright, requests)");
        return;
    }

    writeLog("环境检测", QString("缺少依赖: %1，准备安装").arg(missingPkgs.join(", ")));

    // ------------------------------------------------------------------
    // 3. 缺少依赖 → 弹出带进度条的安装对话框
    // ------------------------------------------------------------------
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Python 依赖安装");
    dlg->setFixedSize(580, 380);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *lay = new QVBoxLayout(dlg);
    lay->setSpacing(12);

    // 3a. 标题说明
    QLabel *lblTitle = new QLabel(dlg);
    lblTitle->setWordWrap(true);
    lblTitle->setText(QString(
                          "<b>检测到缺少以下 Python 依赖库：</b><br>"
                          "<span style='color:#e74c3c; font-size:14px;'>%1</span><br><br>"
                          "点击下方按钮自动下载安装（需要网络连接）。")
                          .arg(missingPkgs.join(", ")));
    lay->addWidget(lblTitle);

    // 3b. 进度条
    QProgressBar *bar = new QProgressBar(dlg);
    bar->setRange(0, 100);
    bar->setValue(0);
    bar->setTextVisible(true);
    bar->setFixedHeight(24);
    lay->addWidget(bar);

    // 3c. 日志输出框（深色主题终端风格）
    QTextEdit *logEdit = new QTextEdit(dlg);
    logEdit->setReadOnly(true);
    logEdit->setMaximumHeight(160);
    logEdit->setStyleSheet(
        "QTextEdit {"
        "  background-color: #1e1e1e;"
        "  color: #d4d4d4;"
        "  font-family: 'Consolas', 'Courier New', monospace;"
        "  font-size: 12px;"
        "  border: 1px solid #333;"
        "  border-radius: 4px;"
        "  padding: 6px;"
        "}");
    lay->addWidget(logEdit);

    // 3d. 按钮
    QHBoxLayout *btnLay = new QHBoxLayout();
    QPushButton *btnInstall = new QPushButton("开始安装", dlg);
    btnInstall->setStyleSheet(
        "QPushButton {"
        "  background-color: #1e90ff; color: white; border: none;"
        "  border-radius: 4px; padding: 8px 28px; font-weight: bold; font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #3742fa; }"
        "QPushButton:disabled { background-color: #a0a0a0; }");

    QPushButton *btnClose = new QPushButton("关闭", dlg);
    btnClose->setStyleSheet(
        "QPushButton {"
        "  background-color: #747d8c; color: white; border: none;"
        "  border-radius: 4px; padding: 8px 28px; font-weight: bold; font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #57606f; }");
    btnClose->setEnabled(false);

    btnLay->addStretch();
    btnLay->addWidget(btnInstall);
    btnLay->addWidget(btnClose);
    lay->addLayout(btnLay);

    // ------------------------------------------------------------------
    // 4. 安装逻辑（异步执行，不阻塞 UI）
    // ------------------------------------------------------------------
    connect(btnInstall, &QPushButton::clicked, dlg, [=]() {
        // 禁用按钮，防止重复点击
        btnInstall->setEnabled(false);
        btnInstall->setText("安装中...");

        // 步骤标记
        int currentStep = 0;
        // 0 = pip install, 1 = playwright install browser

        auto appendLog = [logEdit](const QString &text) {
            logEdit->append(text);
            // 自动滚动到底部
            QScrollBar *sb = logEdit->verticalScrollBar();
            if (sb) sb->setValue(sb->maximum());
        };

        appendLog("$ 正在安装依赖，请稍候...\n");

        // ── 第一步：pip install 缺失的包 ──
        QProcess *pipProc = new QProcess(dlg);
        pipProc->setProcessChannelMode(QProcess::MergedChannels);

        connect(pipProc, &QProcess::readyReadStandardOutput, dlg, [=]() {
            QString text = QString::fromLocal8Bit(pipProc->readAllStandardOutput()).trimmed();
            if (!text.isEmpty()) {
                appendLog(text);
                // 根据输出内容估算进度
                if (text.contains("Collecting") || text.contains("Downloading"))
                    bar->setValue(qMax(bar->value(), 20));
                else if (text.contains("Installing") || text.contains("Preparing"))
                    bar->setValue(qMax(bar->value(), 50));
                else if (text.contains("Successfully installed"))
                    bar->setValue(70);
            }
        });

        // pip 安装完成后的处理
        QProcess *pwProc = nullptr; // 用于 playwright install 的进程

        connect(pipProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                dlg, [=](int exitCode, QProcess::ExitStatus status) mutable {

                    if (status != QProcess::NormalExit || exitCode != 0) {
                        bar->setValue(0);
                        bar->setFormat("安装失败 (exit code: %1)");
                        appendLog("\n? pip install 失败，请检查网络连接后重试。");
                        appendLog("也可手动执行：");
                        appendLog(QString("  %1 -m pip install %2")
                                      .arg(python, missingPkgs.join(" ")));
                        btnClose->setEnabled(true);
                        btnClose->setText("关闭");
                        pipProc->deleteLater();
                        return;
                    }

                    appendLog("\n? pip 依赖安装成功！");

                    // ── 第二步：如果安装了 playwright，自动安装浏览器 ──
                    if (missingPkgs.contains("playwright")) {
                        bar->setValue(80);
                        bar->setFormat("正在安装 Playwright 浏览器...");
                        appendLog("\n正在安装 Playwright Chromium 浏览器（首次约 200MB）...");

                        pwProc = new QProcess(dlg);
                        pwProc->setProcessChannelMode(QProcess::MergedChannels);

                        connect(pwProc, &QProcess::readyReadStandardOutput, dlg, [=]() {
                            QString t = QString::fromLocal8Bit(pwProc->readAllStandardOutput()).trimmed();
                            if (!t.isEmpty()) appendLog(t);
                        });

                        connect(pwProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                                dlg, [=](int code2, QProcess::ExitStatus st2) mutable {
                                    if (st2 == QProcess::NormalExit && code2 == 0) {
                                        bar->setValue(100);
                                        bar->setFormat("全部安装完成 ?");
                                        appendLog("\n? Playwright 浏览器安装成功！");
                                    } else {
                                        appendLog("\n? Playwright 浏览器安装失败，可稍后手动执行：");
                                        appendLog(QString("  %1 -m playwright install chromium").arg(python));
                                    }
                                    btnClose->setEnabled(true);
                                    btnClose->setText("完成");
                                    pwProc->deleteLater();
                                });

                        pwProc->start(python, QStringList()
                                                  << "-m" << "playwright" << "install" << "chromium");
                        if (!pwProc->waitForStarted(5000)) {
                            appendLog("\n? 无法启动 playwright install 进程");
                            btnClose->setEnabled(true);
                        }
                    } else {
                        bar->setValue(100);
                        bar->setFormat("安装完成 ?");
                        appendLog("\n所有依赖安装完成！");
                        btnClose->setEnabled(true);
                        btnClose->setText("完成");
                    }

                    pipProc->deleteLater();
                });

        // 启动 pip install（使用清华镜像加速国内下载）
        QStringList pipArgs;
        pipArgs << "-m" << "pip" << "install";
        for (const QString &pkg : missingPkgs)
            pipArgs << pkg;
        // 国内镜像（如果海外网络可删除或改为官方源）
        // pipArgs << "-i" << "https://pypi.tuna.tsinghua.edu.cn/simple";

        pipProc->start(python, pipArgs);
        appendLog("$ " + python + " -m pip install " + missingPkgs.join(" "));

        if (!pipProc->waitForStarted(5000)) {
            appendLog("\n? 无法启动 pip 进程");
            btnClose->setEnabled(true);
            pipProc->deleteLater();
        }
    });

    // 关闭按钮
    connect(btnClose, &QPushButton::clicked, dlg, &QDialog::accept);

    // 显示模态对话框
    dlg->exec();
}




void MainWindow::createEnvCard(const EnvironmentInfo &info)
{
    // 1. 创建环境卡片容器 (QFrame)
    QFrame *cardFrame = new QFrame(this);

    // 【修复先前的拼写错误并支持横向拉伸】
    cardFrame->setMinimumWidth(0);
    cardFrame->setMaximumWidth(QWIDGETSIZE_MAX);
    cardFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    cardFrame->setObjectName("EnvCard");
    cardFrame->setStyleSheet(
        "QFrame#EnvCard {"
        "   background-color: #ffffff;"
        "   border: 1px solid #eef0f4;"
        "   border-radius: 12px;"
        "}"
        "QFrame#EnvCard:hover {"
        "   border-color: #b0d4f1;"
        "}"
        );

    // 添加卡片阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(cardFrame);
    shadow->setBlurRadius(20);
    shadow->setXOffset(0);
    shadow->setYOffset(2);
    shadow->setColor(QColor(0, 0, 0, 20));
    cardFrame->setGraphicsEffect(shadow);

    // 【关键】：把当前环境特有的路径和名字存进 cardFrame 属性里
    QString envConfigPath = info.configPath;
    cardFrame->setProperty("config_path", envConfigPath);
    cardFrame->setProperty("env_name", info.name);

    QVBoxLayout *cardMainLayout = new QVBoxLayout(cardFrame);
    cardMainLayout->setContentsMargins(20, 16, 20, 16);
    cardMainLayout->setSpacing(12);

    // ==========================================
    // 2. 环境头部区域 (QHBoxLayout)
    // ==========================================
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *lblTitle = new QLabel(info.name, cardFrame);
    lblTitle->setStyleSheet("font-size: 17px; font-weight: 700; color: #2d3436; background: transparent;");

    QLabel *lblStatus = new QLabel(cardFrame);
    if (info.status == "运行中") {
        lblStatus->setText("  ●  运行中");
        lblStatus->setStyleSheet("color: #00b894; font-size: 12px; font-weight: 600; margin-left: 10px; "
                                 "background: #e6f9f2; border-radius: 10px; padding: 2px 10px;");
    } else {
        lblStatus->setText("  ●  未运行");
        lblStatus->setStyleSheet("color: #b2bec3; font-size: 12px; font-weight: 500; margin-left: 10px; "
                                 "background: #f5f6fa; border-radius: 10px; padding: 2px 10px;");
    }

    QHBoxLayout *titleLeftLayout = new QHBoxLayout();
    titleLeftLayout->addWidget(lblTitle);
    titleLeftLayout->addWidget(lblStatus);
    titleLeftLayout->addStretch();

    QPushButton *btnAttach = new QPushButton("+ 挂载模块", cardFrame);
    btnAttach->setCursor(Qt::PointingHandCursor);
    btnAttach->setStyleSheet(
        "QPushButton { background-color: #0984e3; color: white; border: none; border-radius: 6px; padding: 7px 14px; font-weight: 600; font-size: 12px; }"
        "QPushButton:hover { background-color: #0770c4; }"
        "QPushButton:pressed { background-color: #065da6; }"
        );

    headerLayout->addLayout(titleLeftLayout, 1);
    headerLayout->addSpacing(20);
    headerLayout->addWidget(btnAttach);
    cardMainLayout->addLayout(headerLayout);

    // 分割线
    QFrame *line = new QFrame(cardFrame);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #f0f2f5; background-color: #f0f2f5; max-height: 1px; border: none;");
    cardMainLayout->addWidget(line);

    // ==========================================
    // 3. 已挂载模块区域 (QHBoxLayout)
    // ==========================================
    QHBoxLayout *modulesLayout = new QHBoxLayout();
    modulesLayout->setContentsMargins(0, 0, 0, 0);
    modulesLayout->setSpacing(10);

    QLabel *lblModuleTitle = new QLabel("已挂载模块：", cardFrame);
    lblModuleTitle->setStyleSheet("color: #636e72; font-weight: 600; font-size: 13px; background: transparent;");
    modulesLayout->addWidget(lblModuleTitle);

    // 动态包装渲染已有标签模块的辅助 Lambda 函数 (保持当前 cardFrame 独立作用域)
    auto createTagWidget = [this,cardFrame, modulesLayout, envConfigPath](const QString &modName) {
        if (modName.trimmed().isEmpty()) return;

        QWidget *tagWidget = new QWidget(cardFrame);
        QString bgColor = "#e8f4ff";
        QString textColor = "#1e90ff";
        // if (modName.contains("B站")) { bgColor = "#ffeaf2"; textColor = "#ff6b81"; }
        // else if (modName.contains("TK")) { bgColor = "#fff2e6"; textColor = "#ff9f43"; }

        tagWidget->setStyleSheet(QString("QWidget { background-color: %1; border-radius: 4px; }").arg(bgColor));

        QHBoxLayout *tagLayout = new QHBoxLayout(tagWidget);
        tagLayout->setContentsMargins(8, 4, 8, 4);
        tagLayout->setSpacing(6);

        QLabel *lblTagName = new QLabel(modName, tagWidget);
        lblTagName->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: bold; background: transparent;").arg(textColor));

        QPushButton *btnCloseTag = new QPushButton("×", tagWidget);
        btnCloseTag->setFixedSize(14, 14);
        btnCloseTag->setCursor(Qt::PointingHandCursor);
        btnCloseTag->setStyleSheet(
            QString("QPushButton { color: %1; border: none; background: transparent; font-size: 14px; font-weight: bold; line-height: 14px; }"
                    "QPushButton:hover { color: #ff4757; }").arg(textColor)
            );

        tagLayout->addWidget(lblTagName);
        tagLayout->addWidget(btnCloseTag);

        int insertIdx = modulesLayout->count() - 2;
        modulesLayout->insertWidget(insertIdx >= 1 ? insertIdx : 1, tagWidget);

        connect(btnCloseTag, &QPushButton::clicked, tagWidget, [this, envConfigPath, modName]() {
            QFile file(envConfigPath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QJsonObject json = QJsonDocument::fromJson(file.readAll()).object();
                file.close();

                QJsonArray modulesArr = json["modules"].toArray();
                QJsonArray updatedArr;
                for (int i = 0; i < modulesArr.size(); ++i) {
                    if (modulesArr[i].toString() != modName) {
                        updatedArr.append(modulesArr[i]);
                    }
                }
                json["modules"] = updatedArr;

                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
                    file.close();
                }
            }

            // 刷新界面（延迟执行，避免删除信号发送者期间出问题）
            QTimer::singleShot(0, this, &MainWindow::refreshEnvCards);
        });
    };



    // 循环遍历加载环境中已挂载的数据
    for (const QString &modName : info.modules) {
        createTagWidget(modName);
    }

    QPushButton *btnAddMod = new QPushButton("+ 添加模块", cardFrame);
    btnAddMod->setCursor(Qt::PointingHandCursor);
    btnAddMod->setStyleSheet(
        "QPushButton { background-color: transparent; color: #9c9c9c; border: 1px dashed #ced6e0; border-radius: 4px; padding: 4px 12px; font-size: 12px; }"
        "QPushButton:hover { border-color: #1e90ff; color: #1e90ff; }"
        );
    modulesLayout->addWidget(btnAddMod);
    modulesLayout->addStretch();

    cardMainLayout->addLayout(modulesLayout);


    // ==========================================
    // 5. 运行模块按钮区域
    // ==========================================
    QPushButton *btnRunModule = new QPushButton("? 运行模块", cardFrame);
    btnRunModule->setCursor(Qt::PointingHandCursor);
    btnRunModule->setStyleSheet(
        "QPushButton { background-color: #2ed573; color: white; border: none; border-radius: 4px; padding: 8px 16px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background-color: #26af5f; }"
        );

    // 空信号槽，后续实现运行逻辑
    // connect(btnRunModule, &QPushButton::clicked, this, [this]() {
    //     qDebug() << "运行模块按钮被点击 - 待实现";
    //     // 检查当前环境为运行状态
    //     //环境为 运行中 获取当前挂载的 模块
    //     // 参考
    // });

    // 运行所有已挂载模块
        connect(btnRunModule, &QPushButton::clicked, this, [this, btnRunModule]() {
            // 1. 从父级卡片获取环境配置路径和名称
            QWidget *cardFrame = btnRunModule->parentWidget();
            if (!cardFrame) return;

            QString envConfigPath = cardFrame->property("config_path").toString();
            QString envName = cardFrame->property("env_name").toString();
            if (envConfigPath.isEmpty()) {
                QMessageBox::warning(this, "运行模块", "无法获取环境配置路径！");
                return;
            }

            // 2. 检查环境是否在运行中（查找 tableWidget_2 中该环境的端口）
            QString hostPort;
            bool envRunning = false;
            for (int i = 0; i < ui->tableWidget_2->rowCount(); ++i) {
                QTableWidgetItem *nameItem = ui->tableWidget_2->item(i, 2);
                QTableWidgetItem *portItem = ui->tableWidget_2->item(i, 5);
                if (nameItem && nameItem->text().trimmed() == envName && portItem) {
                    hostPort = portItem->text().trimmed();
                    envRunning = true;
                    break;
                }
            }
            if (!envRunning) {
                QMessageBox::warning(this, "运行模块",
                                     QString("环境【%1】未运行，请先打开浏览器环境！").arg(envName));
                return;
            }

            // 3. 读取环境 config.json 获取已挂载的模块列表
            QFile envFile(envConfigPath);
            if (!envFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "运行模块", "无法读取环境配置文件！");
                return;
            }
            QJsonArray mountedModNames = QJsonDocument::fromJson(envFile.readAll()).object()["modules"].toArray();
            envFile.close();

            if (mountedModNames.isEmpty()) {
                QMessageBox::information(this, "运行模块", "该环境未挂载任何模块！");
                return;
            }

            // 4. 弹窗收集通用参数（各模块共享同一套 video/title/desc）
            QDialog dialog(this);
            dialog.setWindowTitle(QString("批量运行模块 - %1").arg(envName));
            dialog.resize(420, 260);

            QVBoxLayout *dlgLayout = new QVBoxLayout(&dialog);
            dlgLayout->addWidget(new QLabel(QString("环境: %1 | HOST端口: %2 | 待运行模块数: %3")
                                                .arg(envName, hostPort).arg(mountedModNames.size()), &dialog));

            QFormLayout *formLayout = new QFormLayout();
            QMap<QString, QLineEdit*> paramEditors;
            QStringList requiredFields = {"video", "title", "desc"};
            for (const QString &field : requiredFields) {
                if (field == "video") {
                    // video 字段：QLineEdit + 浏览按钮
                    QWidget *videoContainer = new QWidget(&dialog);
                    QHBoxLayout *videoLayout = new QHBoxLayout(videoContainer);
                    videoLayout->setContentsMargins(0, 0, 0, 0);
                    videoLayout->setSpacing(6);
                    QLineEdit *edit = new QLineEdit(&dialog);
                    edit->setPlaceholderText("选择视频或图片文件");
                    QPushButton *btnBrowse = new QPushButton("浏览...", &dialog);
                    btnBrowse->setCursor(Qt::PointingHandCursor);
                    btnBrowse->setFixedWidth(80);
                    btnBrowse->setStyleSheet(
                        "QPushButton { background-color: #0984e3; color: white; border: none;"
                        "  border-radius: 4px; padding: 6px 12px; font-size: 12px; font-weight: 600; }"
                        "QPushButton:hover { background-color: #0770c4; }");
                    videoLayout->addWidget(edit, 1);
                    videoLayout->addWidget(btnBrowse);
                    paramEditors["video"] = edit;
                    formLayout->addRow("video:", videoContainer);

                    connect(btnBrowse, &QPushButton::clicked, &dialog, [edit]() {
                        QString filePath = QFileDialog::getOpenFileName(
                            nullptr, "选择视频或图片文件", QString(),
                            "视频文件 (*.mp4 *.avi *.mov *.mkv *.wmv *.flv *.webm);;"
                            "图片文件 (*.png *.jpg *.jpeg *.gif *.bmp *.webp);;"
                            "所有文件 (*.*)");
                        if (!filePath.isEmpty()) {
                            edit->setText(filePath);
                        }
                    });
                } else if (field == "title") {
                    QLineEdit *edit = new QLineEdit(&dialog);
                    edit->setPlaceholderText("输入视频标题");
                    paramEditors[field] = edit;
                    formLayout->addRow(field + ":", edit);
                } else if (field == "desc") {
                    QLineEdit *edit = new QLineEdit(&dialog);
                    edit->setPlaceholderText("输入视频简介");
                    paramEditors[field] = edit;
                    formLayout->addRow(field + ":", edit);
                }
            }
            dlgLayout->addLayout(formLayout);

            QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
            btnBox->button(QDialogButtonBox::Ok)->setText("全部运行");
            dlgLayout->addWidget(btnBox);

            connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
            connect(btnBox, &QDialogButtonBox::accepted, &dialog, [&dialog, &paramEditors]() {
                for (auto it = paramEditors.begin(); it != paramEditors.end(); ++it) {
                    if (it.value()->text().trimmed().isEmpty()) {
                        QMessageBox::warning(&dialog, "运行模块",
                                             QString("%1 为必填项！").arg(it.key()));
                        it.value()->setFocus();
                        return;
                    }
                }
                dialog.accept();
            });

            if (dialog.exec() != QDialog::Accepted) return;

            // 收集参数值
            QMap<QString, QString> inputValues;
            for (auto it = paramEditors.begin(); it != paramEditors.end(); ++it)
                inputValues[it.key()] = it.value()->text().trimmed();

            // 5. 扫描 modules/ 目录，构建 模块显示名 → {路径, ID} 映射（含 runner）
            QDir modulesDir(QCoreApplication::applicationDirPath() + "/modules");
            QMap<QString, QPair<QString, QString>> nameToInfo;
            for (const QString &subDir : modulesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                QFile mf(modulesDir.absoluteFilePath(subDir + "/config.json"));
                if (!mf.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
                QJsonObject mJson = QJsonDocument::fromJson(mf.readAll()).object();
                mf.close();
                QString mName = mJson["name"].toString().trimmed();
                if (!mName.isEmpty())
                    nameToInfo[mName] = qMakePair(modulesDir.absoluteFilePath(subDir),
                                                  mJson["id"].toString());
            }

            // 6. 构建任务队列（收集所有待执行模块，预校验，不立即启动）
            struct ModuleTask {
                QString displayName;
                QString runner;
                QStringList args;
                QString workingDir;
            };
            QList<ModuleTask> taskQueue;
            int preCheckFail = 0;

            for (int i = 0; i < mountedModNames.size(); ++i) {
                QString modDisplayName = mountedModNames[i].toString().trimmed();
                if (!nameToInfo.contains(modDisplayName)) {
                    qDebug() << "跳过未找到的模块:" << modDisplayName;
                    preCheckFail++;
                    continue;
                }

                QPair<QString, QString> pair = nameToInfo[modDisplayName];
                QString modPath = pair.first;

                // 6a. 读取模块配置
                QFile cfgFile(modPath + "/config.json");
                if (!cfgFile.open(QIODevice::ReadOnly | QIODevice::Text)) { preCheckFail++; continue; }
                QJsonObject modJson = QJsonDocument::fromJson(cfgFile.readAll()).object();
                cfgFile.close();

                // 6b. 查找可执行脚本（优先 script/entry 字段，回退到任意 .py）
                QStringList candidates;
                QString script = modJson["script"].toString().trimmed();
                if (script.isEmpty()) script = modJson["entry"].toString().trimmed();
                if (!script.isEmpty()) candidates << script;

                QDir modDir(modPath);
                for (const QFileInfo &fi : modDir.entryInfoList(QStringList() << "*.py", QDir::Files, QDir::Name))
                    candidates << fi.fileName();

                QString scriptPath;
                for (const QString &c : candidates) {
                    QFileInfo sf(c);
                    QString cp = sf.isAbsolute() ? c : modDir.absoluteFilePath(c);
                    if (QFile::exists(cp)) { scriptPath = cp; break; }
                }
                if (scriptPath.isEmpty()) {
                    qDebug() << modDisplayName << "无可用脚本";
                    preCheckFail++; continue;
                }

                // 6c. 确定 runner（优先从 config.json 读取）
                QString runner = modJson["runner"].toString().trimmed();
                if (runner.isEmpty()) runner = modJson["runtime"].toString().trimmed();
                if (runner.isEmpty()) runner = "python";

                // 6d. 组装命令行参数
                QStringList args;
                args << scriptPath << "--host" << hostPort;
                for (const QString &field : requiredFields)
                    args << QString("--%1").arg(field) << inputValues[field];

                ModuleTask task;
                task.displayName = modDisplayName;
                task.runner = runner;
                task.args = args;
                task.workingDir = modPath;
                taskQueue.append(task);
            }

            if (taskQueue.isEmpty()) {
                QMessageBox::information(this, "运行模块",
                    QString("环境【%1】无可用模块！\n成功: 0 | 失败: %2 | 总计: %3")
                        .arg(envName).arg(preCheckFail).arg(mountedModNames.size()));
                return;
            }

            // 7. 创建终端风格输出窗口
            QDialog *batchDlg = new QDialog(this);
            batchDlg->setWindowTitle(QString("批量运行模块 - %1").arg(envName));
            batchDlg->setFixedSize(680, 520);
            // 生命周期由 deleteLater() 管理，不用 WA_DeleteOnClose

            QVBoxLayout *batchLay = new QVBoxLayout(batchDlg);
            batchLay->setSpacing(10);

            QLabel *lblBatchTitle = new QLabel(batchDlg);
            lblBatchTitle->setWordWrap(true);
            lblBatchTitle->setText(QString("<b>环境: %1</b>&nbsp;&nbsp;|&nbsp;&nbsp;HOST: %2&nbsp;&nbsp;|&nbsp;&nbsp;模块总数: %3")
                                   .arg(envName, hostPort).arg(taskQueue.size()));
            batchLay->addWidget(lblBatchTitle);

            QProgressBar *batchBar = new QProgressBar(batchDlg);
            batchBar->setRange(0, taskQueue.size());
            batchBar->setValue(0);
            batchBar->setTextVisible(true);
            batchBar->setFixedHeight(20);
            batchLay->addWidget(batchBar);

            QTextEdit *batchLogEdit = new QTextEdit(batchDlg);
            batchLogEdit->setReadOnly(true);
            batchLogEdit->setStyleSheet(
                "QTextEdit {"
                "  background-color: #1e1e1e; color: #d4d4d4;"
                "  font-family: 'Consolas', 'Courier New', monospace;"
                "  font-size: 12px; border: 1px solid #333;"
                "  border-radius: 4px; padding: 8px;"
                "}");
            batchLay->addWidget(batchLogEdit);

            QHBoxLayout *batchBtnLay = new QHBoxLayout();
            batchBtnLay->addStretch();
            QPushButton *batchBtnClose = new QPushButton("关闭", batchDlg);
            batchBtnClose->setEnabled(false);
            batchBtnClose->setStyleSheet(
                "QPushButton { background-color: #1e90ff; color: white; border: none;"
                "  border-radius: 4px; padding: 8px 28px; font-size: 13px; font-weight: bold; }"
                "QPushButton:hover { background-color: #3742fa; }"
                "QPushButton:disabled { background-color: #a0a0a0; }");
            batchBtnLay->addWidget(batchBtnClose);
            batchLay->addLayout(batchBtnLay);

            auto batchAppend = [batchLogEdit](const QString &text) {
                batchLogEdit->append(text);
                QScrollBar *sb = batchLogEdit->verticalScrollBar();
                if (sb) sb->setValue(sb->maximum());
            };

            batchAppend(QString("══════════════════════════════════════"));
            batchAppend(QString("  环境: %1 | HOST: %2").arg(envName, hostPort));
            batchAppend(QString("  待执行模块: %1 个").arg(taskQueue.size()));
            batchAppend(QString("══════════════════════════════════════\n"));

            // 8. 异步串行执行 —— 用 QProcess::finished 信号驱动任务链，不阻塞 UI
            auto state = QSharedPointer<QPair<int, int>>::create(0, preCheckFail);
            auto queue = QSharedPointer<QList<ModuleTask>>::create(taskQueue);
            auto runNext = QSharedPointer<std::function<void()>>::create();

            *runNext = [this, envName, hostPort, inputValues, requiredFields,
                        mountedModNames, state, queue, runNext,
                        batchDlg, batchBar, batchAppend, batchBtnClose, lblBatchTitle,
                        batchLogEdit]() mutable
            {
                if (queue->isEmpty()) {
                    // 全部完成
                    batchBar->setValue(batchBar->maximum());
                    int total = mountedModNames.size();
                    int success = state->first;
                    int fail = state->second;
                    batchAppend("");
                    batchAppend(QString("══════════════════════════════════════"));
                    batchAppend(QString("  全部完成！成功: %1 | 失败: %2 | 总计: %3").arg(success).arg(fail).arg(total));
                    batchAppend(QString("══════════════════════════════════════"));
                    if (fail > 0) {
                        batchBar->setFormat(QString("完成 — 成功: %1 / 失败: %2").arg(success).arg(fail));
                        lblBatchTitle->setText(lblBatchTitle->text() + QString(" — %1成/%2败").arg(success).arg(fail));
                    } else {
                        batchBar->setFormat("? 全部成功");
                        lblBatchTitle->setText(lblBatchTitle->text() + " — 全部成功 ?");
                    }
                    batchBtnClose->setEnabled(true);
                    return;
                }

                ModuleTask task = queue->takeFirst();
                int currentIdx = batchBar->maximum() - queue->size() - 1;
                batchBar->setValue(currentIdx);
                batchBar->setFormat(QString("正在执行 [%1/%2]: %3")
                                    .arg(currentIdx + 1).arg(batchBar->maximum()).arg(task.displayName));

                batchAppend(QString("── [%1/%2] %3 ──")
                            .arg(currentIdx + 1).arg(batchBar->maximum()).arg(task.displayName));
                batchAppend(QString("  $ %1 %2").arg(task.runner, task.args.join(" ")));

                QProcess *proc = new QProcess(batchDlg);
                proc->setWorkingDirectory(task.workingDir);
                proc->setProcessChannelMode(QProcess::MergedChannels);

                QProcessEnvironment procEnv = QProcessEnvironment::systemEnvironment();
                procEnv.insert("HOST", hostPort);
                for (const QString &field : requiredFields)
                    procEnv.insert(field, inputValues[field]);
                proc->setProcessEnvironment(procEnv);

                connect(proc, &QProcess::readyReadStandardOutput, batchDlg, [this, proc, task, batchAppend]() {
                    QString output = QString::fromLocal8Bit(proc->readAllStandardOutput());
                    if (!output.trimmed().isEmpty()) {
                        batchAppend(output.trimmed());
                        writeLog(QString("模块输出[%1]").arg(task.displayName), output.trimmed());
                    }
                });

                connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), batchDlg,
                    [this, proc, task, state, runNext, batchAppend](int exitCode, QProcess::ExitStatus exitStatus) {
                        // 读取剩余输出写入日志
                        QString remaining = QString::fromLocal8Bit(proc->readAll());
                        if (!remaining.trimmed().isEmpty()) {
                            batchAppend(remaining.trimmed());
                            writeLog(QString("模块输出[%1]").arg(task.displayName), remaining.trimmed());
                        }

                        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                            batchAppend(QString("  ? %1 完成 (exitCode: 0)\n").arg(task.displayName));
                            writeLog("模块完成", QString("批量 — %1, exitCode: 0").arg(task.displayName));
                            state->first++;
                        } else {
                            batchAppend(QString("  ? %1 失败 (exitCode: %2, exitStatus: %3)\n")
                                        .arg(task.displayName).arg(exitCode).arg(exitStatus));
                            writeLog("模块失败", QString("批量 — %1, exitCode: %2, exitStatus: %3")
                                     .arg(task.displayName).arg(exitCode).arg(exitStatus));
                            state->second++;
                        }
                        proc->deleteLater();
                        // 延迟 300ms 让 UI 有机会刷新，然后启动下一个任务
                        QTimer::singleShot(300, this, [runNext]() { (*runNext)(); });
                    });

                proc->start(task.runner, task.args);
                if (!proc->waitForStarted(3000)) {
                    QString errOutput = QString::fromLocal8Bit(proc->readAll());
                    batchAppend(QString("  ? %1 启动超时: %2").arg(task.displayName, proc->errorString()));
                    writeLog("模块失败", QString("批量 — %1, 启动超时, 错误: %2")
                             .arg(task.displayName).arg(proc->errorString()));
                    if (!errOutput.trimmed().isEmpty()) {
                        batchAppend(QString("  %1").arg(errOutput.trimmed()));
                        writeLog(QString("模块启动失败详情[%1]").arg(task.displayName), errOutput.trimmed());
                    }
                    state->second++;
                    proc->deleteLater();
                    QTimer::singleShot(300, this, [runNext]() { (*runNext)(); });
                } else {
                    writeLog("模块运行", QString("批量 — %1, runner: %2, 参数: %3")
                             .arg(task.displayName, task.runner, task.args.join(" ")));
                }
            };

            // 启动第一个任务
            batchAppend("$ 开始执行...\n");
            QTimer::singleShot(100, this, [runNext]() { (*runNext)(); });

            // 禁止执行期间关闭对话框（X 按钮 + Alt+F4 均拦截）
            batchDlg->setWindowFlags(batchDlg->windowFlags() & ~Qt::WindowCloseButtonHint);
            // 关闭按钮在全部完成后才启用，由 finished 回调设置 btnClose->setEnabled(true)
            connect(batchBtnClose, &QPushButton::clicked, batchDlg, &QDialog::accept);
            batchDlg->exec();
            // exec() 返回后安全删除（用户点了关闭按钮）
            batchDlg->deleteLater();
        });




    cardMainLayout->addWidget(btnRunModule);
    cardMainLayout->addStretch();
    // tagWidget->deleteLater();




    auto onAttachTriggered = [this]() {
        // 利用 qobject_cast 和 sender() 精确抓取当前点击的究竟是哪个环境里的按钮
        QPushButton *clickedBtn = qobject_cast<QPushButton*>(sender());
        if (!clickedBtn) return;

        // 从按钮的父级（cardFrame）动态捞出当前环境专属的路径
        QWidget *pFrame = clickedBtn->parentWidget();
        if (!pFrame) return;

        QString currentJsonPath = pFrame->property("config_path").toString();

        // 重新构建该卡片内部局部的 lambda 动态渲染功能
        QLayout *pLayout = pFrame->layout();
        if (!pLayout) return;

        // 读取本地系统中 modules/ 文件夹下导入的所有真实可用模块
        QDir modulesDir(QCoreApplication::applicationDirPath() + "/modules");
        QStringList availableModules;
        for (const QString &subDir : modulesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QFile modFile(modulesDir.absoluteFilePath(subDir + "/config.json"));
            if (modFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QJsonObject mJson = QJsonDocument::fromJson(modFile.readAll()).object();
                QString mName = mJson["name"].toString();
                if (!mName.isEmpty()) availableModules << mName;
                modFile.close();
            }
        }

        qDebug() << "\n===== [挂载按钮点击] =====";
        qDebug() << "按钮文本:" << clickedBtn->text();
        qDebug() << "parent widget objectName:" << pFrame->objectName();
        qDebug() << "currentJsonPath:" << currentJsonPath;
        qDebug() << "文件存在?" << QFile::exists(currentJsonPath);
        // 列出真实的 profiles 文件夹名做对比
        qDebug() << "profiles/ 下真实子文件夹:"
                 << QDir(QCoreApplication::applicationDirPath() + "/profiles")
                        .entryList(QDir::Dirs | QDir::NoDotAndDotDot);



        if (availableModules.isEmpty()) {
            QMessageBox::information(pFrame, "提示", "当前系统模块管理中未导入任何模块，请先去导入模块！");
            return;
        }

        // 弹窗供用户选择需要挂载的真实模块名
        QDialog dialog(this);
        dialog.setWindowTitle("选择要挂载的模块");
        dialog.setFixedWidth(300);
        QVBoxLayout *dlgLayout = new QVBoxLayout(&dialog);

        QComboBox *modCombo = new QComboBox(&dialog);
        modCombo->addItems(availableModules);
        dlgLayout->addWidget(new QLabel("请选择模块："));
        dlgLayout->addWidget(modCombo);

        QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        dlgLayout->addWidget(btnBox);
        connect(btnBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            QString selectedModule = modCombo->currentText();

            // 1. 读取当前环境的真实 config.json
            QFile file(currentJsonPath);
            QJsonObject envJson;
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                envJson = QJsonDocument::fromJson(file.readAll()).object();
                file.close();
            }

            QJsonArray modulesArr = envJson["modules"].toArray();

            // 防止重复挂载同一个模块
            for(int i=0; i<modulesArr.size(); ++i) {
                if(modulesArr[i].toString() == selectedModule) {
                    QMessageBox::warning(pFrame, "提示", "该模块已挂载，请勿重复添加！");
                    return;
                }
            }

            // 2. 将选择的新模块名追加写入
            modulesArr.append(selectedModule);
            envJson["modules"] = modulesArr;

            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(QJsonDocument(envJson).toJson(QJsonDocument::Indented));
                file.close();

                // 3. ? 延迟刷新卡片 UI（等当前信号处理完毕后再销毁旧卡片，避免重复）
                QTimer::singleShot(0, this, &MainWindow::refreshEnvCards);
            }
        }
    };

    // 正确地连接信号槽，不赋予额外的生命周期依赖
    connect(btnAttach, &QPushButton::clicked, this, onAttachTriggered);
    connect(btnAddMod, &QPushButton::clicked, this, onAttachTriggered);

    // 追加进主布局容器
    if (m_envLayout) {
        m_envLayout->addWidget(cardFrame);
    }
}




//分组管理
void MainWindow::on_pushButton_3_clicked()
{
    writeLog("点击", "导航 → 自动化管理");
    ui->stackedWidget->setCurrentWidget(ui->page_5);
}





// 创建浏览器
void MainWindow::on_pushButton_7_clicked()
{
    writeLog("点击", "导航 → 新建浏览器");
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}

// 环境管理
void MainWindow::on_pushButton_6_clicked()
{
    writeLog("点击", "导航 → 环境管理");
    ui->stackedWidget->setCurrentWidget(ui->page);
}

// 代理管理
void MainWindow::on_pushButton_4_clicked()
{
    writeLog("点击", "导航 → 代理管理");
    ui->stackedWidget->setCurrentWidget(ui->page_3);
}

// API
void MainWindow::on_pushButton_2_clicked()
{
    writeLog("点击", "导航 → API调用");
    ui->stackedWidget->setCurrentWidget(ui->page_4);
}



void MainWindow::on_pushButton_clicked()
{
    writeLog("点击", "导航 → 模块管理");
    ui->stackedWidget->setCurrentWidget(ui->page_6);
}



//创建 浏览器 确定


void MainWindow::on_pushButton_8_clicked()
{
    // 1. 基础校验：环境名称不能为空
    QString envName = ui->lineEdit_2->text().trimmed();
    if (envName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入环境名称！");
        return;
    }

    // 2. 自动生成唯一ID和创建独立文件夹
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString envId = QString("env_%1").arg(timestamp);

    // 建立存储总根目录 profiles/
    QDir runDir(QCoreApplication::applicationDirPath());
    QString profilesRoot = runDir.absoluteFilePath("profiles");

    // 当前环境的独立目录及 Chrome 用户数据目录
    QString currentEnvPath = profilesRoot + "/" + envId;
    QString chromeDataPath = currentEnvPath + "/chrome-data";

    // 创建文件夹
    QDir dir;
    if (!dir.mkpath(chromeDataPath)) {
        QMessageBox::critical(this, "错误", "无法创建独立指纹文件夹，请检查主程序权限！");
        return;
    }

    // 3. 构建 JSON 配置对象
    QJsonObject configJson;
    configJson["env_id"] = envId;                         // 环境唯一ID
    configJson["env_name"] = envName;                     // 环境名称
    configJson["created_time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    configJson["chrome_user_data_dir"] = "./chrome-data";  // ?? 核心：相对路径，移动文件夹后仍能定位

    // --- 抓取其他指纹页面控件数据 ---
    configJson["browser_name"] = ui->comboBox_2->currentText();
    configJson["browser_path"] = ui->comboBox_2->currentData().toString();
    configJson["browser_channel"] = ui->comboBox_3->currentText();
    configJson["os_family"] = ui->comboBox_6->currentText();
    configJson["os_version"] = ui->comboBox_8->currentText();
    configJson["cpu_arch"] = ui->comboBox_7->currentText();
    configJson["device_type"] = ui->comboBox_5->currentText();
    configJson["ua_profile"] = ui->comboBox_9->currentText();
    QString userAgent = ui->lineEdit_3->text().trimmed();
    if (userAgent.isEmpty()) {
        userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/146.0.0.0 Safari/537.36";
    }
    configJson["user_agent"] = userAgent;
    configJson["group"] = ui->fontComboBox_2->currentText();
    configJson["tag"] = ui->fontComboBox->currentText();
    configJson["cookie"] = ui->lineEdit_4->text().trimmed();
    configJson["remark"] = ui->lineEdit_5->text().trimmed();
    configJson["proxy_type"] = ui->comboBox_11->currentText();
    configJson["proxy_ip_channel"] = ui->comboBox_12->currentText();
    configJson["proxy_host"] = lineEditProxyHost->text();
    configJson["proxy_port"] = lineEditProxyPort->text();
    configJson["proxy_username"] = lineEditProxyUser->text();
    configJson["proxy_password"] = lineEditProxyPass->text();
    configJson["account_platform"] = ui->comboBox_10->currentText();
    configJson["startup_urls"] = ui->textEdit->toPlainText();
    configJson["webrtc_mode"] = ui->tabWidget_3->tabText(ui->tabWidget_3->currentIndex());
    configJson["timezone_mode"] = ui->tabWidget_4->tabText(ui->tabWidget_4->currentIndex());
    configJson["timezone"] = ui->comboBox_14->currentText();
    configJson["geolocation_mode"] = ui->tabWidget_5->tabText(ui->tabWidget_5->currentIndex());
    configJson["language_mode"] = ui->tabWidget_6->tabText(ui->tabWidget_6->currentIndex());
    configJson["accept_language"] = "zh-CN,zh;q=0.9,en;q=0.8";
    configJson["ui_language_mode"] = ui->tabWidget_7->tabText(ui->tabWidget_7->currentIndex());
    configJson["ui_language"] = "zh-CN";
    configJson["resolution_mode"] = ui->tabWidget_8->tabText(ui->tabWidget_8->currentIndex());
    configJson["resolution"] = "1920,1080";
    configJson["font_mode"] = ui->tabWidget_9->tabText(ui->tabWidget_9->currentIndex());
    configJson["webgl_mode"] = ui->tabWidget_2->tabText(ui->tabWidget_2->currentIndex());
    configJson["webgl_vendor"] = ui->comboBox_13->currentText();
    configJson["webgl_renderer"] = ui->lineEdit_6->text().trimmed();

    // 硬件指纹勾选状态 (这里确保只定义一次 hardwareNoise)
    QJsonObject hardwareNoise;
    hardwareNoise["canvas_noise"] = ui->checkBox->isChecked();
    hardwareNoise["audio_noise"] = ui->checkBox_2->isChecked();
    hardwareNoise["client_rects_noise"] = ui->checkBox_3->isChecked();
    hardwareNoise["webgl_noise"] = ui->checkBox_4->isChecked();
    hardwareNoise["font_noise"] = ui->checkBox_5->isChecked();
    hardwareNoise["media_devices_noise"] = ui->checkBox_6->isChecked();

    // ?? 修复拼写错误，将原来的 configJs0n 改为 configJson
    configJson["hardware_noise"] = hardwareNoise;

    // 4. 将 JSON 写入 config.json 文件
    QString jsonFilePath = currentEnvPath + "/config.json";
    QFile file(jsonFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument jsonDoc(configJson);
        file.write(jsonDoc.toJson(QJsonDocument::Indented)); // 格式化写入
        file.close();
        qDebug() << "成功创建指纹配置并保存到：" << jsonFilePath;
    } else {
        QMessageBox::critical(this, "错误", "无法写入JSON配置文件！");
        return;
    }

    QMessageBox::information(this, "成功", QString("环境【%1】创建成功！\n指纹路径已隔离。").arg(envName));

    writeLog("创建环境", QString("名称: %1, ID: %2, 浏览器: %3, 代理: %4")
             .arg(envName, envId, configJson["browser_name"].toString(), configJson["proxy_type"].toString()));

    // 5. 刷新环境管理列表和自动化管理卡片，并切回环境管理页面
    loadEnvironmentsFromJson();
    QTimer::singleShot(0, this, &MainWindow::refreshEnvCards);

    ui->stackedWidget->setCurrentWidget(ui->page); // 切回列表页
}



// 将 config.json 中存储的 chrome_user_data_dir 解析为绝对路径
// 新版本存储相对路径 "./chrome-data"，旧版本存储绝对路径，此函数兼容两种格式
QString MainWindow::resolveChromeDataPath(const QString &configJsonPath, const QString &storedPath) const
{
    // 如果已经是绝对路径（旧版本兼容）且目录存在，直接返回
    if (QDir::isAbsolutePath(storedPath)) {
        QDir dir(storedPath);
        if (dir.exists()) {
            return storedPath;
        }
        // 绝对路径指向的目录不存在（可能是移动了文件夹），
        // 回退使用相对路径 "./chrome-data" 重新解析
    }

    // 确定要解析的相对路径：新版直接存 "./chrome-data"，旧版绝对路径失效时也回退到此
    QString relativePath = QDir::isAbsolutePath(storedPath) ? "./chrome-data" : storedPath;

    // 相对路径：以 config.json 所在目录为基准解析
    QFileInfo jsonFileInfo(configJsonPath);
    QDir jsonDir = jsonFileInfo.absoluteDir();
    QString resolved = jsonDir.absoluteFilePath(relativePath);
    return QDir::cleanPath(resolved);
}

MainWindow::BrowserLaunchConfig MainWindow::buildLaunchConfig(const QJsonObject &configJson) const
{
    BrowserLaunchConfig config;
    config.envName = configJson["env_name"].toString();
    config.chromeUserDataDir = configJson["chrome_user_data_dir"].toString();
    config.browserPath = configJson["browser_path"].toString();
    config.userAgent = configJson["user_agent"].toString();
    config.acceptLanguage = configJson["accept_language"].toString("zh-CN,zh;q=0.9,en;q=0.8");
    config.uiLanguage = configJson["ui_language"].toString("zh-CN");
    config.timezone = configJson["timezone"].toString();
    config.resolution = configJson["resolution"].toString();

    const QString proxyType = configJson["proxy_type"].toString();
    const QString proxyHost = configJson["proxy_host"].toString();
    const int proxyPort = configJson["proxy_port"].toInt();
    if (!proxyType.isEmpty() && proxyType.compare("No Proxy", Qt::CaseInsensitive) != 0
        && proxyType != QString::fromUtf8("无代理") && !proxyHost.isEmpty() && proxyPort > 0) {
        config.proxyServer = QString("%1://%2:%3").arg(proxyType.toLower(), proxyHost).arg(proxyPort);
        config.proxyUsername = configJson["proxy_username"].toString();
        config.proxyPassword = configJson["proxy_password"].toString();
    }

    const QString webrtcMode = configJson["webrtc_mode"].toString();
    config.disableWebRtc = webrtcMode.contains(QString::fromUtf8("禁用"), Qt::CaseInsensitive)
                            || webrtcMode.contains("Disable", Qt::CaseInsensitive);
    const QString geoMode = configJson["geolocation_mode"].toString();
    config.disableGeolocation = geoMode.contains(QString::fromUtf8("禁用"), Qt::CaseInsensitive)
                                || geoMode.contains("Disable", Qt::CaseInsensitive);

    const QJsonObject noise = configJson["hardware_noise"].toObject();
    const QString webglMode = configJson["webgl_mode"].toString();
    config.disableWebGl = noise["webgl_noise"].toBool(false)
                          || webglMode.contains(QString::fromUtf8("禁用"), Qt::CaseInsensitive)
                          || webglMode.contains("Disable", Qt::CaseInsensitive);

    const QStringList lines = configJson["startup_urls"].toString()
                                  .split(QRegularExpression("[\\r\\n]+"), Qt::SkipEmptyParts);
    for (QString url : lines) {
        url = url.trimmed();
        if (!url.isEmpty()) {
            config.startupUrls.append(url);
        }
    }

    return config;
}



void MainWindow::openBrowserEnvironment(const BrowserLaunchConfig &config, int debugPort)
{
    QString chromeExePath = config.browserPath;
    if (chromeExePath.isEmpty()) {
        chromeExePath = "C:/Program Files/Google/Chrome/Application/chrome.exe";
    }

    if (!QFile::exists(chromeExePath)) {
        QMessageBox::warning(this, "提示", "未找到浏览器程序，请检查新建环境中保存的浏览器路径！");
        return;
    }

    QStringList arguments;
    arguments << QString("--user-data-dir=%1").arg(QDir::toNativeSeparators(config.chromeUserDataDir));
    arguments << QString("--remote-debugging-port=%1").arg(debugPort);

    if (!config.userAgent.isEmpty()) {
        arguments << QString("--user-agent=%1").arg(config.userAgent);
    }
    if (!config.proxyServer.isEmpty()) {
        arguments << QString("--proxy-server=%1").arg(effectiveProxyServer);
    }
    if (!config.acceptLanguage.isEmpty()) {
        arguments << QString("--accept-lang=%1").arg(config.acceptLanguage);
    }
    if (!config.uiLanguage.isEmpty()) {
        arguments << QString("--lang=%1").arg(config.uiLanguage);
    }
    if (!config.resolution.isEmpty()) {
        arguments << QString("--window-size=%1").arg(config.resolution);
    }
    if (config.disableWebRtc) {
        arguments << "--force-webrtc-ip-handling-policy=disable_non_proxied_udp";
        arguments << "--disable-features=WebRtcHideLocalIpsWithMdns";
    }
    if (config.disableGeolocation) {
        arguments << "--deny-permission-prompts";
    }
    if (config.disableWebGl) {
        arguments << "--disable-webgl";
    }
    arguments << "--no-first-run";
    arguments << "--no-default-browser-check";
    arguments << "--skip-first-run-ui";
    arguments << config.startupUrls;

    QProcess *chromeProcess = new QProcess(this);
    if (!config.timezone.isEmpty()) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("TZ", config.timezone);
        chromeProcess->setProcessEnvironment(env);
    }

    chromeProcess->setProperty("my_port", debugPort);
    chromeProcess->setProperty("env_name", config.envName);

    connect(chromeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, chromeProcess, relayProcess]() {
        int deadPort = chromeProcess->property("my_port").toInt();
        QString envName = chromeProcess->property("env_name").toString();
        QString deadPortStr = QString::number(deadPort);

        for (int i = 0; i < ui->tableWidget_2->rowCount(); ++i) {
            QTableWidgetItem *portItem = ui->tableWidget_2->item(i, 5);
            if (portItem && portItem->text() == deadPortStr) {
                ui->tableWidget_2->removeRow(i);
                break;
            }
        }

        for (int i = 0; i < ui->tableWidget_2->rowCount(); ++i) {
            QTableWidgetItem *idxItem = ui->tableWidget_2->item(i, 0);
            if (idxItem) idxItem->setText(QString::number(i + 1));
        }

        writeLog("浏览器关闭", QString("环境: %1, 端口: %2 (进程退出)").arg(envName).arg(deadPort));
        chromeProcess->deleteLater();
        updateEnvCardStatus(envName, false);
    });

    chromeProcess->start(chromeExePath, arguments);

    if (chromeProcess->waitForStarted(3000)) {
        qDebug() << "独立指纹浏览器已拉起，端口:" << debugPort << "参数:" << arguments;
        writeLog("浏览器启动", QString("成功 — 环境: %1, 端口: %2, UA: %3").arg(config.envName).arg(debugPort).arg(config.userAgent.left(60)));
        updateEnvCardStatus(config.envName, true);
    } else {
        qDebug() << "浏览器启动失败:" << chromeProcess->errorString();
        writeLog("浏览器启动", QString("失败 — 环境: %1, 端口: %2, 错误: %3").arg(config.envName).arg(debugPort).arg(chromeProcess->errorString()));
    }
}



void MainWindow::openBrowserEnvironment(const QString &chromeUserDataDir, int debugPort, const QString &customUA, const QString &envName)
{
    QString chromeExePath = "C:/Program Files/Google/Chrome/Application/chrome.exe";

    if (!QFile::exists(chromeExePath)) {
        QMessageBox::warning(this, "提示", "未找到本地 Chrome 浏览器，请检查路径是否正确！");
        return;
    }

    QStringList arguments;
    arguments << QString("--user-data-dir=%1").arg(QDir::toNativeSeparators(chromeUserDataDir));
    arguments << QString("--remote-debugging-port=%1").arg(debugPort);

    if (!customUA.isEmpty()) {
        arguments << QString("--user-agent=%1").arg(customUA);
    }
    arguments << "--no-first-run";
    arguments << "--no-default-browser-check";
    arguments << "--skip-first-run-ui";

    // 1. 创建进程对象
    QProcess *chromeProcess = new QProcess(this);

    // ??【灵魂核心】：把这个浏览器使用的端口暗中绑定在这个进程对象上！
    chromeProcess->setProperty("my_port", debugPort);
    chromeProcess->setProperty("env_name", envName);

    // 2. 核心监听：当浏览器进程结束（不管是崩溃还是被用户手动关闭 ?）
    connect(chromeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, chromeProcess]() {

        // 提取出当前死掉的进程对应的端口号
        int deadPort = chromeProcess->property("my_port").toInt();
        QString envName = chromeProcess->property("env_name").toString();
        QString deadPortStr = QString::number(deadPort);
        qDebug() << "检测到外部浏览器已关闭，正在注销端口记录：" << deadPort;

        // 遍历整个运行表 tableWidget_2，寻找哪一行的端口和它对得上
        for (int i = 0; i < ui->tableWidget_2->rowCount(); ++i) {
            QTableWidgetItem *portItem = ui->tableWidget_2->item(i, 5); // 端口在第 6 列 (索引 5)
            if (portItem && portItem->text() == deadPortStr) {

                // ?? 找到了！直接把这一行在运行表里物理抹去！
                ui->tableWidget_2->removeRow(i);
                qDebug() << "成功移除 tableWidget_2 运行状态行：" << i;
                break;
            }
        }

        // 顺便重新刷新一下 tableWidget_2 的首列“序号”，保持 1、2、3 连续
        for (int i = 0; i < ui->tableWidget_2->rowCount(); ++i) {
            QTableWidgetItem *idxItem = ui->tableWidget_2->item(i, 0);
            if (idxItem) idxItem->setText(QString::number(i + 1));
        }

        // 彻底安全销毁进程指针，防止内存泄漏
        writeLog("浏览器关闭", QString("环境: %1, 端口: %2 (进程退出)").arg(envName).arg(deadPort));
        chromeProcess->deleteLater();

        updateEnvCardStatus(envName, false);

    });

    // 3. 启动进程
    chromeProcess->start(chromeExePath, arguments);

    if (chromeProcess->waitForStarted(3000)) {
        qDebug() << "独立指纹浏览器已成功拉起！端口：" << debugPort;
        writeLog("浏览器启动", QString("成功 (简化版) — 环境: %1, 端口: %2").arg(envName).arg(debugPort));
        updateEnvCardStatus(envName, true); // ← 新增：更新卡片状态为"运行中"
    } else {
        qDebug() << "拉起失败，错误代码：" << chromeProcess->errorString();
        writeLog("浏览器启动", QString("失败 (简化版) — 环境: %1, 错误: %2").arg(envName).arg(chromeProcess->errorString()));
    }
}


void MainWindow::updateEnvCardStatus(const QString &envName, bool running)
{
    // 遍历 m_envLayout 中的所有卡片，找到 env_name 匹配的那个
    if (!m_envLayout) return;

    for (int i = 0; i < m_envLayout->count(); ++i) {
        QLayoutItem *item = m_envLayout->itemAt(i);
        if (!item || !item->widget()) continue;

        QString cardEnvName = item->widget()->property("env_name").toString();
        if (cardEnvName != envName) continue;

        // 找到了匹配的卡片，更新它的状态标签
        // 卡片结构：cardFrame(QVBoxLayout) → headerLayout(QHBoxLayout) → titleLeftLayout → lblStatus
        QFrame *cardFrame = qobject_cast<QFrame*>(item->widget());
        if (!cardFrame) return;

        QVBoxLayout *cardLayout = qobject_cast<QVBoxLayout*>(cardFrame->layout());
        if (!cardLayout || cardLayout->count() == 0) return;

        // headerLayout 是第一个子布局
        QHBoxLayout *headerLayout = qobject_cast<QHBoxLayout*>(cardLayout->itemAt(0)->layout());
        if (!headerLayout || headerLayout->count() == 0) return;

        // titleLeftLayout 是 headerLayout 的第一个子项
        QHBoxLayout *titleLeftLayout = qobject_cast<QHBoxLayout*>(headerLayout->itemAt(0)->layout());
        if (!titleLeftLayout || titleLeftLayout->count() < 2) return;

        QLabel *lblStatus = qobject_cast<QLabel*>(titleLeftLayout->itemAt(1)->widget());
        if (!lblStatus) return;

        if (running) {
            lblStatus->setText("● 运行中");
            lblStatus->setStyleSheet("color: #2ed573; font-size: 13px; font-weight: bold; margin-left: 10px;");
        } else {
            lblStatus->setText("● 未运行");
            lblStatus->setStyleSheet("color: #a4b0be; font-size: 13px; font-weight: bold; margin-left: 10px;");
        }
        writeLog("状态变更", QString("环境: %1 → %2").arg(envName, running ? "运行中" : "未运行"));
        break;
    }
}

// 【核心功能】：从本地 profiles 文件夹遍历读取 JSON 配置并刷到 UI 上
void MainWindow::loadEnvironmentsFromJson()
{
    // 定位到主程序运行根目录下的 profiles 文件夹
    QDir profilesDir(QCoreApplication::applicationDirPath() + "/profiles");
    if (!profilesDir.exists()) {
        // 如果目录不存在，说明还没有创建任何指纹，创建空文件夹并直接返回
        profilesDir.mkpath(".");
        ui->tableWidget->setRowCount(0);
        rightTable->setRowCount(0);
        return;
    }

    // 扫描获取 profiles 目录下的所有子环境文件夹
    QStringList subDirs = profilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 设置双表的总行数
    int rowCount = subDirs.size();
    ui->tableWidget->setRowCount(rowCount);
    rightTable->setRowCount(rowCount);

    int validRow = 0;
    for (int i = 0; i < rowCount; ++i) {
        QString jsonPath = profilesDir.absoluteFilePath(subDirs[i] + "/config.json");
        QFile file(jsonPath);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue; // 如果某个文件夹里的 config.json 坏了或不存在，跳过
        }

        QByteArray jsonData = file.readAll();
        file.close();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) continue;

        QJsonObject configJson = jsonDoc.object();

        // --- 1. 从 JSON 解析真实的指纹配置参数 ---
        QString envId      = configJson["env_id"].toString();
        QString envName    = configJson["env_name"].toString();
        QString remark     = configJson["remark"].toString();
        QString proxyType  = configJson["proxy_type"].toString();
        QString userAgent  = configJson["user_agent"].toString();
        // 核心路径：读取此前保存的用户专属指纹浏览器数据路径
        // 兼容旧版绝对路径 + 新版相对路径，统一解析为绝对路径
        QString chromePath = resolveChromeDataPath(jsonPath, configJson["chrome_user_data_dir"].toString());
        // 同步更新 configJson，确保后续存入 UserRole+3 的缓存 JSON 也是绝对路径
        configJson["chrome_user_data_dir"] = chromePath;

        // --- 2. 填充左侧主表的数据 ---

        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        ui->tableWidget->setItem(validRow, 0, checkItem);

        // 后面各列：根据你实际的列位置填入
        ui->tableWidget->setItem(validRow, 1, new QTableWidgetItem(QString::number(validRow + 1))); // 序号

        // 环境名称（??核心技巧：将 chromePath 隐藏绑定在环境名单元格的 UserRole 里面）
        QTableWidgetItem *nameItem = new QTableWidgetItem(envName);
        nameItem->setData(Qt::UserRole, chromePath);      // 藏入数据目录路径
        nameItem->setData(Qt::UserRole + 1, userAgent);   // 藏入对应的自定义 UA
        nameItem->setData(Qt::UserRole + 2, jsonPath);
        nameItem->setData(Qt::UserRole + 3, QString::fromUtf8(QJsonDocument(configJson).toJson(QJsonDocument::Compact)));
        ui->tableWidget->setItem(validRow, 2, nameItem);

        ui->tableWidget->setItem(validRow, 3, new QTableWidgetItem(envId));     // 账号信息/环境ID
        ui->tableWidget->setItem(validRow, 4, new QTableWidgetItem(proxyType)); // 代理类型
        ui->tableWidget->setItem(validRow, 5, new QTableWidgetItem("127.0.0.1"));// 演示IP
        ui->tableWidget->setItem(validRow, 6, new QTableWidgetItem(remark));    // 备注

        // --- 3. 填充右侧固定表的数据（动态生成带信号槽绑定的“打开”组合键） ---
        // --- 3. 填充右侧固定表的数据（动态生成带信号槽绑定的“打开”组合键） ---
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(10, 2, 10, 2);
        actionLayout->setSpacing(10);

        // 打开按钮
        QPushButton *btnOpen = new QPushButton("打开");
        btnOpen->setCursor(Qt::PointingHandCursor);
        btnOpen->setMinimumWidth(55);
        btnOpen->setStyleSheet(
            "QPushButton { background-color: #2ed573; color: white; border: none; border-radius: 4px; padding: 4px 10px; font-weight: bold; font-size: 12px; }"
            "QPushButton:hover { background-color: #26af5f; }"
            );

        // ??【核心灵魂】：把当前的行号通过 setProperty 传给按钮，方便点击时知道点击的是哪一行
        btnOpen->setProperty("row_index", validRow);

        // ??【核心灵魂】：绑定信号槽到通用的处理槽函数上
        connect(btnOpen, &QPushButton::clicked, this, &MainWindow::on_btnOpen_clicked);

        actionLayout->addWidget(btnOpen);

        // 设置按钮
        // ================== 【设置按钮 + 点击弹出下拉框】 ==================
        QPushButton *btnSetting = new QPushButton("Yes");
        btnSetting->setCursor(Qt::PointingHandCursor);
        btnSetting->setFixedSize(28, 24);

        // 优化后的样式表：
        btnSetting->setStyleSheet(
            "QPushButton {"
            "   background-color: transparent;"
            "   color: #57606f;"
            "   border: 1px solid #ced6e0;"
            "   border-radius: 4px;"
            "   padding: 0px;"               // 核心补丁 1：强制内边距为 0，防止图标被挤向一侧
            "   text-align: center;"         // 核心补丁 2：显式强制内容对齐至正中心
            "}"
            "QPushButton:hover {"
            "   background-color: #f1f2f6;"
            "}"
            "QPushButton::menu-indicator {"  // 核心补丁 3：不仅消去图片，连同小三角占用的空间和位置全部清空
            "   image: none;"
            "   width: 0px;"
            "   subcontrol-position: center;"
            "   subcontrol-origin: padding;"
            "}"
            );

        // 创建下拉菜单
        QMenu *settingMenu = new QMenu(this);
        // 给下拉菜单加点精美的样式
        settingMenu->setStyleSheet(
            "QMenu { background-color: white; border: 1px solid #ced6e0; border-radius: 4px; padding: 4px 0px; }"
            "QMenu::item { padding: 6px 25px 6px 20px; font-size: 12px; color: #333333; }"
            "QMenu::item:selected { background-color: #f1f2f6; color: #1e90ff; }"
            );

        // 创建下拉菜单中的各个选项（Action）
        QAction *actEdit   = settingMenu->addAction("?? 修改环境");
        QAction *actDelete = settingMenu->addAction("... 删除环境");

        // ??【核心灵魂】：把当前行号标记到每个 Action 上，方便点击时知道点的是哪一行
        actEdit->setProperty("row_index", validRow);
        actDelete->setProperty("row_index", validRow);

        // ??【核心灵魂】：把这些动作连接到你的自定义槽函数
        connect(actEdit, &QAction::triggered, this, &MainWindow::on_btnEdit_triggered);
        connect(actDelete, &QAction::triggered, this, &MainWindow::on_btnDelete_triggered);

        // 绑定：把菜单塞进按钮里，变成真正的下拉框按钮
        btnSetting->setMenu(settingMenu);

        actionLayout->addWidget(btnSetting);


        rightTable->setCellWidget(validRow, 0, actionWidget);


        ui->tableWidget->setRowHeight(validRow, 42);
        rightTable->setRowHeight(validRow, 42);

        validRow++;
    }

    // 如果有因损坏跳过的文件，修正最终行数显示
    ui->tableWidget->setRowCount(validRow);
    rightTable->setRowCount(validRow);
}


// 【核心功能】：当点击任何一行的“打开”按钮时触发
void MainWindow::on_btnOpen_clicked()
{
    // 1. 获取发送当前信号的按钮指针
    QPushButton *clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton) return;

    // 2. 提取出保存在按钮中的行号
    int row = clickedButton->property("row_index").toInt();

    // 3. 抓取原始数据
    QTableWidgetItem *nameItem  = ui->tableWidget->item(row, 2);
    QTableWidgetItem *idItem    = ui->tableWidget->item(row, 3);
    QTableWidgetItem *proxyItem = ui->tableWidget->item(row, 4);
    QTableWidgetItem *ipItem    = ui->tableWidget->item(row, 5);

    if (!nameItem) return;

    QString envName    = nameItem->text();
    QString envId      = idItem ? idItem->text() : "未分组";
    QString proxyType  = proxyItem ? proxyItem->text() : "无代理";
    QString ipAddress  = ipItem ? ipItem->text() : "127.0.0.1";

    QString chromeUserDataDir = nameItem->data(Qt::UserRole).toString();
    QString customUA          = nameItem->data(Qt::UserRole + 1).toString();
    QJsonObject launchJson;
    const QString cachedJson = nameItem->data(Qt::UserRole + 3).toString();
    if (!cachedJson.isEmpty()) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(cachedJson.toUtf8(), &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            launchJson = doc.object();
        }
    }
    if (launchJson.isEmpty()) {
        launchJson["env_name"] = envName;
        launchJson["chrome_user_data_dir"] = chromeUserDataDir;
        launchJson["user_agent"] = customUA;
    }

    // 4. 分配端口
    int debugPort = 9200 + row;
    QString targetPortStr = QString::number(debugPort);

    // =========================================================================
    // ? 防重复机制：检测 tableWidget_2 中是否已经存在该端口
    // =========================================================================
    for (int i = 0; i < ui->tableWidget_2->rowCount(); ++i) {
        QTableWidgetItem *runningPortItem = ui->tableWidget_2->item(i, 5);
        if (runningPortItem && runningPortItem->text() == targetPortStr) {
            QMessageBox::warning(this, "提示", QString("环境【%1】已经在运行中（端口：%2），无需重复打开！").arg(envName).arg(debugPort));
            return;
        }
    }

    // =========================================================================
    // ? 动态填充 tableWidget_2
    // =========================================================================
    if (ui->tableWidget_2->columnCount() < 6) {
        ui->tableWidget_2->setColumnCount(6);
        ui->tableWidget_2->setHorizontalHeaderLabels(QStringList() << "序号" << "分组" << "名称" << "代理" << "IP" << "端口");
    }

    int runRowCount = ui->tableWidget_2->rowCount();
    ui->tableWidget_2->insertRow(runRowCount);

    QTableWidgetItem *itemIndex = new QTableWidgetItem(QString::number(runRowCount + 1));
    QTableWidgetItem *itemGroup = new QTableWidgetItem(envId);
    QTableWidgetItem *itemName  = new QTableWidgetItem(envName);
    QTableWidgetItem *itemProxy = new QTableWidgetItem(proxyType);
    QTableWidgetItem *itemIp    = new QTableWidgetItem(ipAddress);
    QTableWidgetItem *itemPort  = new QTableWidgetItem(targetPortStr);

    QList<QTableWidgetItem*> items = {itemIndex, itemGroup, itemName, itemProxy, itemIp, itemPort};
    for (int col = 0; col < items.size(); ++col) {
        items[col]->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget_2->setItem(runRowCount, col, items[col]);
    }
    ui->tableWidget_2->setRowHeight(runRowCount, 35);

    // 5. 调用核心拉起逻辑（这里我们修改了 openBrowserEnvironment 传入机制）
    qDebug() << "准备打开环境：第" << row + 1 << "行，存储目录：" << chromeUserDataDir << "，分配端口：" << debugPort;
    writeLog("打开环境", QString("名称: %1, 端口: %2, 数据目录: %3").arg(envName).arg(debugPort).arg(chromeUserDataDir));
    openBrowserEnvironment(buildLaunchConfig(launchJson), debugPort);
}

// 【下拉菜单：修改环境点击响应】
void MainWindow::on_btnEdit_triggered()
{
    // 1. 获取触发当前动作的 QAction
    QAction *clickedAction = qobject_cast<QAction*>(sender());
    if (!clickedAction) return;

    // 2. 提取出保存在 Action 里的行号
    int row = clickedAction->property("row_index").toInt();

    // 3. 拿到行里的真实数据（比如环境名称）
    QTableWidgetItem *nameItem = ui->tableWidget->item(row, 2);
    if (!nameItem) return;
    QString envName = nameItem->text();

    // 提示测试（实际业务中可以在这里弹出修改窗口或跳转到修改页面）
    QMessageBox::information(this, "修改环境", QString("你点击了第 %1 行【%2】的修改动作").arg(row + 1).arg(envName));
    writeLog("修改环境", QString("行: %1, 名称: %2 (功能待完善)").arg(row + 1).arg(envName));
}

// 【下拉菜单：删除环境点击响应】
void MainWindow::on_btnDelete_triggered()
{
    QAction *clickedAction = qobject_cast<QAction*>(sender());
    if (!clickedAction) return;

    int row = clickedAction->property("row_index").toInt();

    QTableWidgetItem *nameItem = ui->tableWidget->item(row, 2);
    if (!nameItem) return;
    QString envName = nameItem->text();
    // 隐藏绑定的环境路径
    QString chromePath = nameItem->data(Qt::UserRole).toString();

    // 弹出确认删除框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除", QString("确定要删除环境【%1】吗？\n(注意：这将无法恢复！)").arg(envName),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        writeLog("删除环境", QString("名称: %1, 路径: %2").arg(envName, chromePath));
        // 1. 根据路径把本地的独立指纹文件夹以及 config.json 整个删掉
        // 获取环境根目录（chrome-data的上一层）
        QDir envDir(chromePath);
        envDir.cdUp();

        qDebug() << "删除环境 - chromePath:" << chromePath;
        qDebug() << "删除环境 - envDir 绝对路径:" << envDir.absolutePath();
        qDebug() << "删除环境 - envDir 是否存在:" << envDir.exists();

        if (envDir.exists()) {
            // ?? 先尝试终止可能占用该目录的 Chrome 进程
            QString envRootPath = QDir::toNativeSeparators(envDir.absolutePath());
            QProcess killProc;
            killProc.start("taskkill", QStringList()
                << "/f" << "/im" << "chrome.exe"
                << "/t");
            killProc.waitForFinished(2000);

            // 稍等片刻让文件锁释放
            QThread::msleep(500);

            bool removed = envDir.removeRecursively();

            // 如果常规删除失败，使用 cmd 强力删除
            if (!removed) {
                qDebug() << "删除环境 - 常规删除失败，尝试 cmd 强力删除";
                QProcess cmdProc;
                cmdProc.start("cmd.exe", QStringList()
                    << "/c" << "rmdir" << "/s" << "/q"
                    << QDir::toNativeSeparators(envDir.absolutePath()));
                if (cmdProc.waitForFinished(10000)) {
                    removed = !envDir.exists();
                    qDebug() << "删除环境 - cmd 删除结果:" << (removed ? "成功" : "失败");
                }
            }

            qDebug() << "删除环境 - 最终结果:" << (removed ? "成功" : "失败");
        }

        // 2. 物理文件删完后，重新加载一次列表刷新 UI
        loadEnvironmentsFromJson();
        QTimer::singleShot(0, this, &MainWindow::refreshEnvCards);

        QMessageBox::information(this, "成功", "环境已成功删除！");
    }
}



// 【核心功能】：从本地 modules 文件夹遍历读取 JSON 配置并刷到 tableWidget_3 上
void MainWindow::loadModulesFromJson()
{
    QDir modulesDir(QCoreApplication::applicationDirPath() + "/modules");
    if (!modulesDir.exists()) {
        modulesDir.mkpath(".");
        ui->tableWidget_3->setRowCount(0);
        return;
    }

    QStringList subDirs = modulesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 初始化设置 tableWidget_3 的列宽及表头（4列：ID、名字、运行环境、操作）
    ui->tableWidget_3->setColumnCount(4);
    ui->tableWidget_3->setHorizontalHeaderLabels(QStringList() << "模块 ID" << "模块名称" << "运行环境" << "操作");
    ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->tableWidget_3->setColumnWidth(3, 150); // 操作列固定 150px，放两个按钮绰绰有余
    ui->tableWidget_3->verticalHeader()->setVisible(false);

    int validRow = 0;
    ui->tableWidget_3->setRowCount(subDirs.size());

    for (int i = 0; i < subDirs.size(); ++i) {
        // 记录当前子文件夹的绝对路径，方便后面删除时使用
        QString currentModFolderPath = modulesDir.absoluteFilePath(subDirs[i]);
        QString jsonPath = currentModFolderPath + "/config.json";
        QFile file(jsonPath);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        QByteArray jsonData = file.readAll();
        file.close();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) continue;

        QJsonObject moduleJson = jsonDoc.object();
        QString modId   = moduleJson["id"].toString();
        QString modName = moduleJson["name"].toString();
        QString modEnv  = moduleJson["env"].toString();

        // 1. 填充基础文本数据
        QTableWidgetItem *idItem = new QTableWidgetItem(modId);
        QTableWidgetItem *nameItem = new QTableWidgetItem(modName);
        QTableWidgetItem *envItem = new QTableWidgetItem(modEnv);

        idItem->setTextAlignment(Qt::AlignCenter);
        nameItem->setTextAlignment(Qt::AlignCenter);
        envItem->setTextAlignment(Qt::AlignCenter);

        ui->tableWidget_3->setItem(validRow, 0, idItem);
        ui->tableWidget_3->setItem(validRow, 1, nameItem);
        ui->tableWidget_3->setItem(validRow, 2, envItem);

        // 2. 动态创建右侧的操作按钮（删除、运行）
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 2, 4, 2);
        actionLayout->setSpacing(6);

        // 按钮 A：删除按钮（自适应行高）
        QPushButton *btnDelete = new QPushButton("删除");
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setMinimumWidth(46);
        btnDelete->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        btnDelete->setStyleSheet(
            "QPushButton {"
            "  background-color: #ff4757;"
            "  color: white;"
            "  border: none;"
            "  border-radius: 5px;"
            "  padding: 2px 10px;"
            "  font-size: 12px;"
            "  font-weight: 600;"
            "}"
            "QPushButton:hover {"
            "  background-color: #ff6b81;"
            "}"
            "QPushButton:pressed {"
            "  background-color: #e8414e;"
            "}");
        btnDelete->setProperty("row_index", validRow);
        btnDelete->setProperty("module_id", modId);
        btnDelete->setProperty("module_path", currentModFolderPath);

        // 按钮 B：运行按钮（自适应行高）
        QPushButton *btnRun = new QPushButton("运行");
        btnRun->setCursor(Qt::PointingHandCursor);
        btnRun->setMinimumWidth(46);
        btnRun->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        btnRun->setStyleSheet(
            "QPushButton {"
            "  background-color: #0984e3;"
            "  color: white;"
            "  border: none;"
            "  border-radius: 5px;"
            "  padding: 2px 10px;"
            "  font-size: 12px;"
            "  font-weight: 600;"
            "}"
            "QPushButton:hover {"
            "  background-color: #0770c4;"
            "}"
            "QPushButton:pressed {"
            "  background-color: #065da6;"
            "}");
        btnRun->setProperty("row_index", validRow);
        btnRun->setProperty("module_id", modId);
        btnRun->setProperty("module_path", currentModFolderPath);

        actionLayout->addWidget(btnDelete); // 将删除按钮添加进布局
        actionLayout->addWidget(btnRun);
        actionWidget->setLayout(actionLayout);

        // 将组合按钮塞进第 4 列 (索引 3)
        ui->tableWidget_3->setCellWidget(validRow, 3, actionWidget);
        ui->tableWidget_3->setRowHeight(validRow, 42);

        // 3. 绑定按钮点击事件到槽函数
        //  绑定删除逻辑
        connect(btnDelete, &QPushButton::clicked, this, [this, btnDelete]() {
            QString modId = btnDelete->property("module_id").toString();
            QString modPath = btnDelete->property("module_path").toString();

            // 弹出确认删除框，防止误触
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "确认删除",
                                          QString("确定要物理删除模块【%1】吗？\n(注意：这将无法恢复！)").arg(modId),
                                          QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                QDir dir(modPath);
                if (dir.exists()) {
                    // 物理递归删除整个模块文件夹（包含里面的 config.json 和其它所有脚本文件）
                    if (dir.removeRecursively()) {
                        qDebug() << "成功删除本地模块文件夹：" << modPath;
                        writeLog("删除模块", QString("成功 — ID: %1, 路径: %2").arg(modId, modPath));
                        QMessageBox::information(this, "成功", "模块已成功删除！");
                    } else {
                        QMessageBox::critical(this, "错误", "无法删除模块文件夹，请检查文件是否被占用或权限不足！");
                    }
                }

                // 物理文件删完后，重新加载一次列表刷新 UI 界面
                loadModulesFromJson();
            }
        });



        // 绑定运行逻辑
        connect(btnRun, &QPushButton::clicked, this, [this, btnRun]() {
            int row = btnRun->property("row_index").toInt();
            QString modId = btnRun->property("module_id").toString();
            QString modPath = btnRun->property("module_path").toString();

            if (modPath.isEmpty()) {
                QMessageBox::warning(this, "Run Module", "Module folder was not found.");
                return;
            }

            QFile configFile(QDir(modPath).absoluteFilePath("config.json"));
            if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "Run Module", "Cannot read module config.json.");
                return;
            }

            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(configFile.readAll(), &parseError);
            configFile.close();
            if (parseError.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
                QMessageBox::warning(this, "Run Module", QString("Invalid config.json: %1").arg(parseError.errorString()));
                return;
            }

            QJsonObject moduleJson = jsonDoc.object();
            QJsonArray formatArray = moduleJson["format"].toArray();

            QStringList fieldNames;
            for (const QJsonValue &value : formatArray) {
                QString fieldName = value.toString().trimmed();
                if (!fieldName.isEmpty() && !fieldNames.contains(fieldName)) {
                    fieldNames << fieldName;
                }
            }

            QStringList scriptCandidates;
            QString configuredScript = moduleJson["script"].toString().trimmed();
            if (configuredScript.isEmpty()) {
                configuredScript = moduleJson["entry"].toString().trimmed();
            }
            if (!configuredScript.isEmpty()) {
                scriptCandidates << configuredScript;
            }

            QDir moduleDir(modPath);
            QFileInfoList pyFiles = moduleDir.entryInfoList(QStringList() << "*.py", QDir::Files, QDir::Name);
            for (const QFileInfo &fileInfo : pyFiles) {
                scriptCandidates << fileInfo.fileName();
            }

            QString scriptPath;
            for (const QString &candidate : scriptCandidates) {
                QFileInfo scriptInfo(candidate);
                QString candidatePath = scriptInfo.isAbsolute() ? candidate : moduleDir.absoluteFilePath(candidate);
                if (QFile::exists(candidatePath)) {
                    scriptPath = candidatePath;
                    break;
                }
            }

            if (scriptPath.isEmpty()) {
                QMessageBox::warning(this, "Run Module", "No runnable script was found. Configure script/entry or add a .py file.");
                return;
            }

            QDialog dialog(this);
            dialog.setWindowTitle(QString("Run Module - %1").arg(modId));
            dialog.resize(420, 260);

            QVBoxLayout *dialogLayout = new QVBoxLayout(&dialog);
            QLabel *hintLabel = new QLabel(QString("Module: %1 (row %2)").arg(modId).arg(row + 1), &dialog);
            dialogLayout->addWidget(hintLabel);

            QFormLayout *formLayout = new QFormLayout();
            QComboBox *portCombo = new QComboBox(&dialog);
            for (int i = 0; i < ui->tableWidget_2->rowCount(); ++i) {
                QTableWidgetItem *nameItem = ui->tableWidget_2->item(i, 2);
                QTableWidgetItem *portItem = ui->tableWidget_2->item(i, 5);
                if (!portItem || portItem->text().trimmed().isEmpty()) continue;

                QString port = portItem->text().trimmed();
                QString envName = nameItem ? nameItem->text().trimmed() : QString("Env%1").arg(i + 1);
                portCombo->addItem(QString("%1 (%2)").arg(envName, port), port);
            }

            if (portCombo->count() == 0) {
                QMessageBox::warning(this, "Run Module", "No browser environment is running. Open an environment first.");
                return;
            }

            formLayout->addRow("HOST:", portCombo);

            // QList<QPair<QString, QLineEdit*>> editors;
            // for (const QString &fieldName : fieldNames) {
            //     QLineEdit *editor = new QLineEdit(&dialog);
            //     editor->setPlaceholderText(QString("Enter %1").arg(fieldName));
            //     editors.append(qMakePair(fieldName, editor));
            //     formLayout->addRow(fieldName + ":", editor);
            // }

            QList<QPair<QString, QLineEdit*>> editors;
            // 硬编码固定三个必填参数：video title desc，不依赖config.json的format
            QStringList fixedFields = {"video", "title", "desc"};
            for (const QString &fieldName : fixedFields) {
                if (fieldName == "video") {
                    // video 字段：QLineEdit + 浏览按钮
                    QWidget *videoContainer = new QWidget(&dialog);
                    QHBoxLayout *videoLayout = new QHBoxLayout(videoContainer);
                    videoLayout->setContentsMargins(0, 0, 0, 0);
                    videoLayout->setSpacing(6);
                    QLineEdit *editor = new QLineEdit(&dialog);
                    editor->setPlaceholderText("选择视频或图片文件");
                    QPushButton *btnBrowse = new QPushButton("浏览...", &dialog);
                    btnBrowse->setCursor(Qt::PointingHandCursor);
                    btnBrowse->setFixedWidth(80);
                    btnBrowse->setStyleSheet(
                        "QPushButton { background-color: #0984e3; color: white; border: none;"
                        "  border-radius: 4px; padding: 6px 12px; font-size: 12px; font-weight: 600; }"
                        "QPushButton:hover { background-color: #0770c4; }");
                    connect(btnBrowse, &QPushButton::clicked, &dialog, [editor]() {
                        QString filePath = QFileDialog::getOpenFileName(
                            nullptr, "选择视频或图片文件", QString(),
                            "视频文件 (*.mp4 *.avi *.mov *.mkv *.wmv *.flv *.webm);;"
                            "图片文件 (*.png *.jpg *.jpeg *.gif *.bmp *.webp);;"
                            "所有文件 (*.*)");
                        if (!filePath.isEmpty()) {
                            editor->setText(filePath);
                        }
                    });
                    videoLayout->addWidget(editor, 1);
                    videoLayout->addWidget(btnBrowse);
                    editors.append(qMakePair(fieldName, editor));
                    formLayout->addRow(fieldName + ":", videoContainer);
                } else {
                    QLineEdit *editor = new QLineEdit(&dialog);
                    if(fieldName == "title")
                        editor->setPlaceholderText("输入视频标题");
                    else if(fieldName == "desc")
                        editor->setPlaceholderText("输入视频简介");
                    editors.append(qMakePair(fieldName, editor));
                    formLayout->addRow(fieldName + ":", editor);
                }
            }



            dialogLayout->addLayout(formLayout);

            QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
            buttonBox->button(QDialogButtonBox::Ok)->setText("Run");
            buttonBox->button(QDialogButtonBox::Cancel)->setText("Cancel");
            dialogLayout->addWidget(buttonBox);

            connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
            connect(buttonBox, &QDialogButtonBox::accepted, &dialog, [&dialog, &editors]() {
                for (const auto &pair : editors) {
                    if (pair.second->text().trimmed().isEmpty()) {
                        QMessageBox::warning(&dialog, "Run Module", QString("%1 is required.").arg(pair.first));
                        pair.second->setFocus();
                        return;
                    }
                }
                dialog.accept();
            });

            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            QString hostPort = portCombo->currentData().toString();
            QJsonObject runInput;
            runInput["HOST"] = hostPort;
            for (const auto &pair : editors) {
                runInput[pair.first] = pair.second->text().trimmed();
            }

            // QString runInputPath = moduleDir.absoluteFilePath(QString("run_input_%1.json").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz")));
            // QFile runInputFile(runInputPath);
            // if (runInputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            //     runInputFile.write(QJsonDocument(runInput).toJson(QJsonDocument::Indented));
            //     runInputFile.close();
            // } else {
            //     QMessageBox::warning(this, "Run Module", "Cannot write runtime input file.");
            //     return;
            // }

            QString runner = moduleJson["runner"].toString().trimmed();
            if (runner.isEmpty()) {
                runner = moduleJson["runtime"].toString().trimmed();
            }
            if (runner.isEmpty()) {
                runner = "python";
            }

            QStringList arguments;

            // 【第一位置必须放脚本文件】
            arguments << scriptPath;
            // host端口参数
            arguments << "--host" << hostPort;
            // 自定义字段参数
            for (const auto &pair : editors) {
                arguments << QString("--%1").arg(pair.first) << pair.second->text().trimmed();
            }


            // arguments  << "--host" << hostPort;
            // for (const auto &pair : editors) {
            //     arguments << QString("--%1").arg(pair.first) << pair.second->text().trimmed();
            // }

            // ========== 终端风格输出窗口 ==========
            QDialog *termDlg = new QDialog(this);
            termDlg->setWindowTitle(QString("模块运行 - %1").arg(modId));
            termDlg->setFixedSize(640, 500);
            // 生命周期由 deleteLater() 管理，不用 WA_DeleteOnClose

            QVBoxLayout *termLay = new QVBoxLayout(termDlg);
            termLay->setSpacing(10);

            QLabel *lblTermTitle = new QLabel(termDlg);
            lblTermTitle->setWordWrap(true);
            lblTermTitle->setText(QString("<b>模块: %1</b>&nbsp;&nbsp;|&nbsp;&nbsp;脚本: %2&nbsp;&nbsp;|&nbsp;&nbsp;HOST: %3&nbsp;&nbsp;|&nbsp;&nbsp;状态: 运行中")
                                  .arg(modId, QFileInfo(scriptPath).fileName(), hostPort));
            termLay->addWidget(lblTermTitle);

            QProgressBar *bar = new QProgressBar(termDlg);
            bar->setRange(0, 0); // 不确定进度（忙等动画）
            bar->setTextVisible(false);
            bar->setFixedHeight(18);
            termLay->addWidget(bar);

            QTextEdit *logEdit = new QTextEdit(termDlg);
            logEdit->setReadOnly(true);
            logEdit->setStyleSheet(
                "QTextEdit {"
                "  background-color: #1e1e1e; color: #d4d4d4;"
                "  font-family: 'Consolas', 'Courier New', monospace;"
                "  font-size: 12px; border: 1px solid #333;"
                "  border-radius: 4px; padding: 8px;"
                "}");
            termLay->addWidget(logEdit);

            QHBoxLayout *btnLay = new QHBoxLayout();
            btnLay->addStretch();
            QPushButton *btnClose = new QPushButton("关闭", termDlg);
            btnClose->setEnabled(false);
            btnClose->setStyleSheet(
                "QPushButton { background-color: #1e90ff; color: white; border: none;"
                "  border-radius: 4px; padding: 8px 28px; font-size: 13px; font-weight: bold; }"
                "QPushButton:hover { background-color: #3742fa; }"
                "QPushButton:disabled { background-color: #a0a0a0; }");
            btnLay->addWidget(btnClose);
            termLay->addLayout(btnLay);

            auto appendLog = [logEdit](const QString &text) {
                logEdit->append(text);
                QScrollBar *sb = logEdit->verticalScrollBar();
                if (sb) sb->setValue(sb->maximum());
            };

            // 启动信息
            appendLog(QString("$ %1 %2").arg(runner, arguments.join(" ")));
            appendLog(QString("  工作目录: %1").arg(modPath));
            appendLog("");

            QProcess *moduleProcess = new QProcess(termDlg);
            moduleProcess->setWorkingDirectory(modPath);
            moduleProcess->setProcessChannelMode(QProcess::MergedChannels);

            QProcessEnvironment processEnv = QProcessEnvironment::systemEnvironment();
            processEnv.insert("HOST", hostPort);
            for (const auto &pair : editors) {
                processEnv.insert(pair.first, pair.second->text().trimmed());
            }
            moduleProcess->setProcessEnvironment(processEnv);

            connect(moduleProcess, &QProcess::readyReadStandardOutput, termDlg, [this, moduleProcess, modId, appendLog]() {
                QString output = QString::fromLocal8Bit(moduleProcess->readAllStandardOutput());
                if (!output.trimmed().isEmpty()) {
                    appendLog(output.trimmed());
                    writeLog(QString("模块输出[%1]").arg(modId), output.trimmed());
                }
            });

            connect(moduleProcess, &QProcess::errorOccurred, termDlg, [this, moduleProcess, modId, appendLog, bar, btnClose](QProcess::ProcessError error) {
                QString errOutput = QString::fromLocal8Bit(moduleProcess->readAll());
                appendLog(QString("\n? 进程错误: %1 (错误类型: %2)").arg(moduleProcess->errorString()).arg(error));
                writeLog("模块错误", QString("单独运行 — %1, 错误类型: %2, 描述: %3")
                         .arg(modId).arg(error).arg(moduleProcess->errorString()));
                if (!errOutput.trimmed().isEmpty()) {
                    appendLog(errOutput.trimmed());
                    writeLog(QString("模块错误详情[%1]").arg(modId), errOutput.trimmed());
                }
                bar->setRange(0, 100);
                bar->setValue(0);
                bar->setFormat("启动失败");
                btnClose->setEnabled(true);
            });

            connect(moduleProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), termDlg,
                    [this, moduleProcess, modId, appendLog, bar, btnClose, lblTermTitle](int exitCode, QProcess::ExitStatus exitStatus) {
                        // 读取剩余输出
                        QString remaining = QString::fromLocal8Bit(moduleProcess->readAll());
                        if (!remaining.trimmed().isEmpty()) {
                            appendLog(remaining.trimmed());
                            writeLog(QString("模块输出[%1]").arg(modId), remaining.trimmed());
                        }

                        bar->setRange(0, 100);
                        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                            bar->setValue(100);
                            bar->setFormat("? 执行成功");
                            appendLog("\n? 模块执行完毕 (exitCode: 0)");
                            writeLog("模块完成", QString("单独运行 — %1, exitCode: 0").arg(modId));
                            lblTermTitle->setText(lblTermTitle->text().replace("运行中", "已完成 ?"));
                        } else {
                            bar->setValue(0);
                            bar->setFormat(QString("? 执行失败 (exitCode: %1)").arg(exitCode));
                            appendLog(QString("\n? 模块执行失败 (exitCode: %1, exitStatus: %2)")
                                      .arg(exitCode).arg(exitStatus));
                            writeLog("模块失败", QString("单独运行 — %1, exitCode: %2, exitStatus: %3")
                                     .arg(modId).arg(exitCode).arg(exitStatus));
                            lblTermTitle->setText(lblTermTitle->text().replace("运行中", "执行失败 ?"));
                        }
                        btnClose->setEnabled(true);
                        moduleProcess->deleteLater();
                    });

            moduleProcess->start(runner, arguments);
            if (!moduleProcess->waitForStarted(3000)) {
                QString errOutput = QString::fromLocal8Bit(moduleProcess->readAll());
                appendLog(QString("\n? 脚本启动失败: %1").arg(moduleProcess->errorString()));
                appendLog(QString("  Runner: %1").arg(runner));
                appendLog(QString("  参数: %1").arg(arguments.join(" ")));
                writeLog("模块失败", QString("单独运行 — %1, 启动失败: %2")
                         .arg(modId).arg(moduleProcess->errorString()));
                if (!errOutput.trimmed().isEmpty()) {
                    appendLog(errOutput.trimmed());
                    writeLog(QString("模块启动失败详情[%1]").arg(modId), errOutput.trimmed());
                }
                bar->setRange(0, 100);
                bar->setValue(0);
                bar->setFormat("启动失败");
                btnClose->setEnabled(true);
                moduleProcess->deleteLater();
            } else {
                appendLog("$ 进程已启动，等待输出...\n");
                writeLog("模块运行", QString("单独运行 — %1, runner: %2, 参数: %3")
                         .arg(modId, runner, arguments.join(" ")));
            }

            // 禁止执行期间关闭对话框（X 按钮拦截）
            termDlg->setWindowFlags(termDlg->windowFlags() & ~Qt::WindowCloseButtonHint);
            connect(btnClose, &QPushButton::clicked, termDlg, &QDialog::accept);
            termDlg->exec();
            termDlg->deleteLater();
        });

        validRow++;
    }
    ui->tableWidget_3->setRowCount(validRow);
}



void MainWindow::refreshEnvCards()
{
    if (!m_envLayout) {
        qDebug() << "[refreshEnvCards] m_envLayout 为空，跳过";
        return;
    }

    qDebug() << "\n===== [refreshEnvCards] 开始刷新 =====";
    int totalItems = m_envLayout->count();
    qDebug() << "[refreshEnvCards] 旧布局总项数:" << totalItems;

    // 1. 先遍历收集卡片的 widget 指针（只找 QFrame 类型的卡片，跳过 stretch 等 spacer）
    QList<QWidget*> oldCards;
    for (int i = totalItems - 1; i >= 0; --i) {
        QLayoutItem *item = m_envLayout->itemAt(i);
        if (!item) continue;
        QWidget *w = item->widget();
        if (!w) {
            qDebug() << "[refreshEnvCards] 跳过非widget项(可能是stretch), index:" << i;
            continue;
        }
        // 检查是否是 EnvCard
        QFrame *cardFrame = qobject_cast<QFrame*>(w);
        if (cardFrame && cardFrame->objectName() == "EnvCard") {
            QString cardName = cardFrame->property("env_name").toString();
            QString cardPath = cardFrame->property("config_path").toString();
            qDebug() << "[refreshEnvCards] 找到旧卡片 index:" << i << "名称:" << cardName;
            oldCards.prepend(cardFrame); // prepend 保持原有顺序
        }
    }
    qDebug() << "[refreshEnvCards] 共找到" << oldCards.size() << "张旧卡片";

    // 2. 从布局中移除并销毁所有旧卡片
    for (QWidget *card : oldCards) {
        m_envLayout->removeWidget(card);
        card->hide();
        delete card;
    }

    // 清理 stretch 等非 widget 残留项（全部清除，后面再补 stretch）
    while (m_envLayout->count() > 0) {
        QLayoutItem *item = m_envLayout->takeAt(0);
        delete item;
    }

    // 2. 重新读取 profiles/ 重建卡片
    QDir profilesDir(QCoreApplication::applicationDirPath() + "/profiles");
    QStringList envSubDirs = profilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    qDebug() << "[refreshEnvCards] profiles/ 下子文件夹:" << envSubDirs;

    int newCardCount = 0;
    for (const QString &subDirName : envSubDirs) {
        QString jsonFilePath = profilesDir.absoluteFilePath(subDirName + "/config.json");
        QFile file(jsonFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "[refreshEnvCards] 跳过(无法读取):" << jsonFilePath;
            continue;
        }

        QJsonObject configJson = QJsonDocument::fromJson(file.readAll()).object();
        file.close();

        EnvironmentInfo info;
        info.name = configJson["env_name"].toString();
        info.configPath = jsonFilePath;
        info.status = "未运行";
        info.group = "默认组";

        QJsonArray modsArray = configJson["modules"].toArray();
        for (int m = 0; m < modsArray.size(); ++m) {
            info.modules.append(modsArray[m].toString());
        }

        qDebug() << "[refreshEnvCards] 创建新卡片:" << info.name << "配置文件:" << jsonFilePath;
        createEnvCard(info);
        newCardCount++;
    }

    qDebug() << "[refreshEnvCards] 共创建" << newCardCount << "张新卡片";

    // 3. 补回弹簧 stretch
    m_envLayout->addStretch();

    qDebug() << "[refreshEnvCards] 当前布局项数(含弹簧):" << m_envLayout->count();
    qDebug() << "===== [refreshEnvCards] 刷新完成 =====\n";
}



// 辅助函数：递归拷贝整个文件夹（包含所有子目录和文件）
bool copyDirectoryRecursively(const QString &srcPath, const QString &dstPath)
{
    QDir srcDir(srcPath);
    if (!srcDir.exists()) return false;

    QDir dstDir(dstPath);
    if (!dstDir.exists()) {
        if (!dstDir.mkpath(".")) return false;
    }

    // 遍历源目录下的所有文件和文件夹（排除 . 和 .. 目录）
    QFileInfoList list = srcDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (QFileInfo fileInfo, list) {
        QString srcSubPath = fileInfo.absoluteFilePath();
        QString dstSubPath = dstDir.absoluteFilePath(fileInfo.fileName());

        if (fileInfo.isDir()) {
            // 如果是文件夹，递归调用自身
            if (!copyDirectoryRecursively(srcSubPath, dstSubPath)) {
                return false;
            }
        } else {
            // 如果是文件，直接拷贝
            if (QFile::exists(dstSubPath)) {
                QFile::remove(dstSubPath); // 存在同名文件先删除
            }
            if (!QFile::copy(srcSubPath, dstSubPath)) {
                return false;
            }
        }
    }
    return true;
}



//加载程序包
void MainWindow::on_pushButton_10_clicked()
{
    QFileDialog dialog(this, tr("选择模块文件夹"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);

    for (QListView *view : dialog.findChildren<QListView*>()) {
        view->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    for (QTreeView *view : dialog.findChildren<QTreeView*>()) {
        view->setSelectionMode(QAbstractItemView::MultiSelection);
    }

    if (dialog.exec() != QDialog::Accepted) return;

    QStringList folderPaths = dialog.selectedFiles();
    folderPaths.removeDuplicates();
    if (folderPaths.isEmpty()) return;

    QDir runDir(QCoreApplication::applicationDirPath());
    QString modulesRoot = runDir.absoluteFilePath("modules");

    int successCount = 0;
    QStringList failedMessages;

    for (const QString &folderPath : folderPaths) {
        QString jsonFilePath = QDir(folderPath).absoluteFilePath("config.json");
        QFile file(jsonFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            failedMessages << QString("%1：未检测到有效的 config.json").arg(QDir::toNativeSeparators(folderPath));
            writeLog("导入模块", QString("失败 — 未检测到 config.json，来源: %1").arg(folderPath));
            continue;
        }

        QByteArray jsonData = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
        if (parseError.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
            failedMessages << QString("%1：config.json 格式错误（%2）")
                                  .arg(QDir::toNativeSeparators(folderPath), parseError.errorString());
            writeLog("导入模块", QString("失败 — config.json 格式错误，来源: %1").arg(folderPath));
            continue;
        }

        QJsonObject moduleJson = jsonDoc.object();
        QString modId = moduleJson["id"].toString().trimmed();
        QString modName = moduleJson["name"].toString().trimmed();

        if (modId.isEmpty() || modName.isEmpty()) {
            failedMessages << QString("%1：缺失必须的 id 或 name 字段").arg(QDir::toNativeSeparators(folderPath));
            writeLog("导入模块", QString("失败 — 缺失 id 或 name，来源: %1").arg(folderPath));
            continue;
        }

        QString targetModPath = QDir(modulesRoot).absoluteFilePath(modId);
        QDir targetDir(targetModPath);
        if (targetDir.exists()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "覆盖提示",
                QString("模块 ID【%1】已存在，是否覆盖旧模块？").arg(modId),
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) {
                failedMessages << QString("%1：用户取消覆盖").arg(modName);
                writeLog("导入模块", QString("跳过 — 用户取消覆盖，名称: %1, ID: %2").arg(modName, modId));
                continue;
            }

            if (!targetDir.removeRecursively()) {
                failedMessages << QString("%1：旧模块删除失败").arg(modName);
                writeLog("导入模块", QString("失败 — 旧模块删除失败，名称: %1, ID: %2").arg(modName, modId));
                continue;
            }
        }

        if (copyDirectoryRecursively(folderPath, targetModPath)) {
            successCount++;
            writeLog("导入模块", QString("成功 — 名称: %1, ID: %2, 来源: %3").arg(modName, modId, folderPath));
        } else {
            failedMessages << QString("%1：文件拷贝失败").arg(modName);
            writeLog("导入模块", QString("失败 — 文件拷贝失败，名称: %1, ID: %2, 来源: %3").arg(modName, modId, folderPath));
        }
    }

    loadModulesFromJson();

    if (failedMessages.isEmpty()) {
        QMessageBox::information(this, "成功", QString("已成功导入 %1 个模块。").arg(successCount));
    } else {
        QMessageBox::warning(
            this,
            "导入完成",
            QString("成功导入 %1 个模块，%2 个失败/跳过：\n\n%3")
                .arg(successCount)
                .arg(failedMessages.size())
                .arg(failedMessages.join("\n")));
    }
}
//问题1
//创建运行日志时，没有权限在c盘写入

//问题2 没找到原因
//启动cmd运行模块没有问题
//使用qt运行就炸了
//自己的电脑上没有问题
//别人电脑上出现这样(不是所有模块)


