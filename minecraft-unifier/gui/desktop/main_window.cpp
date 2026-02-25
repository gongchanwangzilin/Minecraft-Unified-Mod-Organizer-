/**
 * Minecraft Unifier - Desktop GUI Implementation
 * 桌面端GUI实现 - Qt6界面
 */

#include "main_window.h"
#include <packer/windows/netease_packer.h>
#include <injector/windows/pe_injector.h>
#include <injector/linux/elf_injector.h>
#include <core/resources/resource_manager.h>
#include <core/render/shader_converter.h>
#include <iostream>

namespace mcu {
namespace gui {
namespace desktop {

// ==================== WorkerThread ====================

WorkerThread::WorkerThread(TaskType type, const std::string& input, const std::string& output)
    : taskType_(type)
    , input_(input)
    , output_(output) {
}

void WorkerThread::run() {
    switch (taskType_) {
        case TaskType::PACK_MOD:
            PackMod();
            break;
        case TaskType::INJECT_GAME:
            InjectGame();
            break;
        case TaskType::CONVERT_RESOURCE:
            ConvertResource();
            break;
        case TaskType::LOAD_RESOURCE_PACK:
            LoadResourcePack();
            break;
    }
}

void WorkerThread::PackMod() {
    emit logMessage("开始打包模组...");
    emit progressUpdated(10);
    
    packer::windows::NeteasePacker packer;
    
    emit logMessage("正在初始化打包器...");
    if (!packer.Initialize()) {
        emit taskCompleted(false, "打包器初始化失败");
        return;
    }
    emit progressUpdated(20);
    
    emit logMessage("正在打包模组: " + QString::fromStdString(input_));
    if (!packer.PackMod(input_, output_)) {
        emit taskCompleted(false, "模组打包失败");
        return;
    }
    emit progressUpdated(80);
    
    emit logMessage("正在转换Java类...");
    if (!packer.ConvertJavaClasses()) {
        emit logMessage("警告: Java类转换失败，但继续打包");
    }
    emit progressUpdated(90);
    
    emit logMessage("正在转换资源...");
    if (!packer.ConvertResources()) {
        emit logMessage("警告: 资源转换失败，但继续打包");
    }
    emit progressUpdated(100);
    
    emit taskCompleted(true, "模组打包成功: " + QString::fromStdString(output_));
}

void WorkerThread::InjectGame() {
    emit logMessage("开始注入游戏...");
    emit progressUpdated(10);
    
#ifdef _WIN32
    injector::windows::PEInjector injector;
    emit logMessage("使用Windows PE注入器");
#elif defined(__linux__)
    injector::linux::ELFInjector injector;
    emit logMessage("使用Linux ELF注入器");
#else
    emit taskCompleted(false, "不支持的平台");
    return;
#endif
    
    emit logMessage("正在初始化注入器...");
    if (!injector.Initialize()) {
        emit taskCompleted(false, "注入器初始化失败");
        return;
    }
    emit progressUpdated(20);
    
    emit logMessage("正在注入游戏: " + QString::fromStdString(input_));
    if (!injector.Inject(input_, output_)) {
        emit taskCompleted(false, "游戏注入失败");
        return;
    }
    emit progressUpdated(100);
    
    emit taskCompleted(true, "游戏注入成功: " + QString::fromStdString(output_));
}

void WorkerThread::ConvertResource() {
    emit logMessage("开始转换资源...");
    emit progressUpdated(10);
    
    core::resources::ResourceManager& manager = core::resources::ResourceManager::instance();
    
    emit logMessage("正在初始化资源管理器...");
    if (!manager.Initialize()) {
        emit taskCompleted(false, "资源管理器初始化失败");
        return;
    }
    emit progressUpdated(20);
    
    emit logMessage("正在转换资源: " + QString::fromStdString(input_));
    if (!manager.ConvertResource(input_, output_)) {
        emit taskCompleted(false, "资源转换失败");
        return;
    }
    emit progressUpdated(100);
    
    emit taskCompleted(true, "资源转换成功: " + QString::fromStdString(output_));
}

void WorkerThread::LoadResourcePack() {
    emit logMessage("开始加载资源包...");
    emit progressUpdated(10);
    
    core::resources::ResourcePackManager packManager;
    
    emit logMessage("正在加载资源包: " + QString::fromStdString(input_));
    if (!packManager.LoadResourcePack(input_)) {
        emit taskCompleted(false, "资源包加载失败");
        return;
    }
    emit progressUpdated(50);
    
    emit logMessage("正在应用资源包...");
    std::string packId = fs::path(input_).stem().string();
    if (!packManager.ApplyResourcePack(packId)) {
        emit taskCompleted(false, "资源包应用失败");
        return;
    }
    emit progressUpdated(100);
    
    emit taskCompleted(true, "资源包加载成功");
}

// ==================== MainWindow ====================

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , workerThread_(nullptr) {
    SetupUI();
    SetupMenuBar();
    SetupStatusBar();
    
    setWindowTitle("我的世界统一器 - Minecraft Unifier");
    resize(1024, 768);
}

MainWindow::~MainWindow() {
    if (workerThread_) {
        workerThread_->quit();
        workerThread_->wait();
        delete workerThread_;
    }
}

void MainWindow::SetupUI() {
    centralWidget_ = new QWidget(this);
    setCentralWidget(centralWidget_);
    
    mainLayout_ = new QVBoxLayout(centralWidget_);
    
    SetupTabWidget();
    
    // 进度条
    progressBar_ = new QProgressBar(this);
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    mainLayout_->addWidget(progressBar_);
}

void MainWindow::SetupMenuBar() {
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // 文件菜单
    QMenu* fileMenu = menuBar->addMenu("文件(&F)");
    
    QAction* exitAction = fileMenu->addAction("退出(&X)");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    // 帮助菜单
    QMenu* helpMenu = menuBar->addMenu("帮助(&H)");
    
    QAction* aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::OnAboutClicked);
}

void MainWindow::SetupStatusBar() {
    QStatusBar* statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    
    statusLabel_ = new QLabel("就绪", this);
    statusBar->addWidget(statusLabel_);
}

void MainWindow::SetupTabWidget() {
    tabWidget_ = new QTabWidget(this);
    mainLayout_->addWidget(tabWidget_);
    
    SetupPackModTab();
    SetupInjectGameTab();
    SetupConvertResourceTab();
    SetupResourcePackTab();
    SetupLogTab();
}

void MainWindow::SetupPackModTab() {
    packModTab_ = new QWidget(this);
    
    QVBoxLayout* layout = new QVBoxLayout(packModTab_);
    
    // 选择模组文件
    QHBoxLayout* fileLayout = new QHBoxLayout();
    selectModFileButton_ = new QPushButton("选择模组文件", this);
    modFileLabel_ = new QLabel("未选择文件", this);
    modFileLabel_->setWordWrap(true);
    fileLayout->addWidget(selectModFileButton_);
    fileLayout->addWidget(modFileLabel_, 1);
    layout->addLayout(fileLayout);
    
    // 打包按钮
    packModButton_ = new QPushButton("开始打包", this);
    packModButton_->setEnabled(false);
    layout->addWidget(packModButton_);
    
    // 输出
    packModOutput_ = new QTextEdit(this);
    packModOutput_->setReadOnly(true);
    layout->addWidget(packModOutput_, 1);
    
    tabWidget_->addTab(packModTab_, "打包模组");
    
    connect(selectModFileButton_, &QPushButton::clicked, this, &MainWindow::OnPackModClicked);
    connect(packModButton_, &QPushButton::clicked, [this]() {
        QString outputFile = QFileDialog::getSaveFileName(this, "保存打包文件", "", "*.cmc");
        if (outputFile.isEmpty()) {
            return;
        }
        
        if (workerThread_) {
            workerThread_->quit();
            workerThread_->wait();
            delete workerThread_;
        }
        
        workerThread_ = new WorkerThread(WorkerThread::TaskType::PACK_MOD,
                                         selectedModFile_, outputFile.toStdString());
        connect(workerThread_, &WorkerThread::progressUpdated, this, &MainWindow::OnProgressUpdated);
        connect(workerThread_, &WorkerThread::taskCompleted, this, &MainWindow::OnTaskCompleted);
        connect(workerThread_, &WorkerThread::logMessage, this, &MainWindow::OnLogMessage);
        workerThread_->start();
        
        packModButton_->setEnabled(false);
    });
}

void MainWindow::SetupInjectGameTab() {
    injectGameTab_ = new QWidget(this);
    
    QVBoxLayout* layout = new QVBoxLayout(injectGameTab_);
    
    // 选择游戏文件
    QHBoxLayout* fileLayout = new QHBoxLayout();
    selectGameFileButton_ = new QPushButton("选择游戏文件", this);
    gameFileLabel_ = new QLabel("未选择文件", this);
    gameFileLabel_->setWordWrap(true);
    fileLayout->addWidget(selectGameFileButton_);
    fileLayout->addWidget(gameFileLabel_, 1);
    layout->addLayout(fileLayout);
    
    // 注入按钮
    injectGameButton_ = new QPushButton("开始注入", this);
    injectGameButton_->setEnabled(false);
    layout->addWidget(injectGameButton_);
    
    // 输出
    injectGameOutput_ = new QTextEdit(this);
    injectGameOutput_->setReadOnly(true);
    layout->addWidget(injectGameOutput_, 1);
    
    tabWidget_->addTab(injectGameTab_, "注入游戏");
    
    connect(selectGameFileButton_, &QPushButton::clicked, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "选择游戏文件", "", 
#ifdef _WIN32
            "*.exe"
#elif defined(__linux__)
            "*"
#endif
        );
        if (!file.isEmpty()) {
            selectedGameFile_ = file.toStdString();
            gameFileLabel_->setText(file);
            injectGameButton_->setEnabled(true);
        }
    });
    
    connect(injectGameButton_, &QPushButton::clicked, [this]() {
        QString outputFile = QFileDialog::getSaveFileName(this, "保存注入文件", "", 
#ifdef _WIN32
            "*.exe"
#elif defined(__linux__)
            "*"
#endif
        );
        if (outputFile.isEmpty()) {
            return;
        }
        
        if (workerThread_) {
            workerThread_->quit();
            workerThread_->wait();
            delete workerThread_;
        }
        
        workerThread_ = new WorkerThread(WorkerThread::TaskType::INJECT_GAME,
                                         selectedGameFile_, outputFile.toStdString());
        connect(workerThread_, &WorkerThread::progressUpdated, this, &MainWindow::OnProgressUpdated);
        connect(workerThread_, &WorkerThread::taskCompleted, this, &MainWindow::OnTaskCompleted);
        connect(workerThread_, &WorkerThread::logMessage, this, &MainWindow::OnLogMessage);
        workerThread_->start();
        
        injectGameButton_->setEnabled(false);
    });
}

void MainWindow::SetupConvertResourceTab() {
    convertResourceTab_ = new QWidget(this);
    
    QVBoxLayout* layout = new QVBoxLayout(convertResourceTab_);
    
    // 选择资源文件
    QHBoxLayout* fileLayout = new QHBoxLayout();
    selectResourceFileButton_ = new QPushButton("选择资源文件", this);
    resourceFileLabel_ = new QLabel("未选择文件", this);
    resourceFileLabel_->setWordWrap(true);
    fileLayout->addWidget(selectResourceFileButton_);
    fileLayout->addWidget(resourceFileLabel_, 1);
    layout->addLayout(fileLayout);
    
    // 转换按钮
    convertResourceButton_ = new QPushButton("开始转换", this);
    convertResourceButton_->setEnabled(false);
    layout->addWidget(convertResourceButton_);
    
    // 输出
    convertResourceOutput_ = new QTextEdit(this);
    convertResourceOutput_->setReadOnly(true);
    layout->addWidget(convertResourceOutput_, 1);
    
    tabWidget_->addTab(convertResourceTab_, "转换资源");
    
    connect(selectResourceFileButton_, &QPushButton::clicked, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "选择资源文件", "", "*.*");
        if (!file.isEmpty()) {
            selectedResourceFile_ = file.toStdString();
            resourceFileLabel_->setText(file);
            convertResourceButton_->setEnabled(true);
        }
    });
    
    connect(convertResourceButton_, &QPushButton::clicked, [this]() {
        QString outputFile = QFileDialog::getSaveFileName(this, "保存转换文件", "", "*.*");
        if (outputFile.isEmpty()) {
            return;
        }
        
        if (workerThread_) {
            workerThread_->quit();
            workerThread_->wait();
            delete workerThread_;
        }
        
        workerThread_ = new WorkerThread(WorkerThread::TaskType::CONVERT_RESOURCE,
                                         selectedResourceFile_, outputFile.toStdString());
        connect(workerThread_, &WorkerThread::progressUpdated, this, &MainWindow::OnProgressUpdated);
        connect(workerThread_, &WorkerThread::taskCompleted, this, &MainWindow::OnTaskCompleted);
        connect(workerThread_, &WorkerThread::logMessage, this, &MainWindow::OnLogMessage);
        workerThread_->start();
        
        convertResourceButton_->setEnabled(false);
    });
}

void MainWindow::SetupResourcePackTab() {
    resourcePackTab_ = new QWidget(this);
    
    QVBoxLayout* layout = new QVBoxLayout(resourcePackTab_);
    
    // 选择资源包文件
    QHBoxLayout* fileLayout = new QHBoxLayout();
    selectPackFileButton_ = new QPushButton("选择资源包文件", this);
    packFileLabel_ = new QLabel("未选择文件", this);
    packFileLabel_->setWordWrap(true);
    fileLayout->addWidget(selectPackFileButton_);
    fileLayout->addWidget(packFileLabel_, 1);
    layout->addLayout(fileLayout);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    loadPackButton_ = new QPushButton("加载资源包", this);
    loadPackButton_->setEnabled(false);
    unloadPackButton_ = new QPushButton("卸载资源包", this);
    unloadPackButton_->setEnabled(false);
    buttonLayout->addWidget(loadPackButton_);
    buttonLayout->addWidget(unloadPackButton_);
    layout->addLayout(buttonLayout);
    
    // 已加载的资源包列表
    loadedPacksList_ = new QListWidget(this);
    layout->addWidget(loadedPacksList_);
    
    // 输出
    resourcePackOutput_ = new QTextEdit(this);
    resourcePackOutput_->setReadOnly(true);
    layout->addWidget(resourcePackOutput_, 1);
    
    tabWidget_->addTab(resourcePackTab_, "资源包管理");
    
    connect(selectPackFileButton_, &QPushButton::clicked, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "选择资源包文件", "", "*.zip *.mcpack *.mcworld");
        if (!file.isEmpty()) {
            selectedPackFile_ = file.toStdString();
            packFileLabel_->setText(file);
            loadPackButton_->setEnabled(true);
        }
    });
    
    connect(loadPackButton_, &QPushButton::clicked, [this]() {
        if (workerThread_) {
            workerThread_->quit();
            workerThread_->wait();
            delete workerThread_;
        }
        
        workerThread_ = new WorkerThread(WorkerThread::TaskType::LOAD_RESOURCE_PACK,
                                         selectedPackFile_, "");
        connect(workerThread_, &WorkerThread::progressUpdated, this, &MainWindow::OnProgressUpdated);
        connect(workerThread_, &WorkerThread::taskCompleted, this, &MainWindow::OnTaskCompleted);
        connect(workerThread_, &WorkerThread::logMessage, this, &MainWindow::OnLogMessage);
        workerThread_->start();
        
        loadPackButton_->setEnabled(false);
    });
}

void MainWindow::SetupLogTab() {
    logTab_ = new QWidget(this);
    
    QVBoxLayout* layout = new QVBoxLayout(logTab_);
    
    logTextEdit_ = new QTextEdit(this);
    logTextEdit_->setReadOnly(true);
    layout->addWidget(logTextEdit_, 1);
    
    clearLogButton_ = new QPushButton("清空日志", this);
    layout->addWidget(clearLogButton_);
    
    tabWidget_->addTab(logTab_, "日志");
    
    connect(clearLogButton_, &QPushButton::clicked, this, &MainWindow::OnClearLogClicked);
}

void MainWindow::OnPackModClicked() {
    QString file = QFileDialog::getOpenFileName(this, "选择模组文件", "", "*.jar *.zip");
    if (!file.isEmpty()) {
        selectedModFile_ = file.toStdString();
        modFileLabel_->setText(file);
        packModButton_->setEnabled(true);
    }
}

void MainWindow::OnInjectGameClicked() {
    // 已在SetupInjectGameTab中实现
}

void MainWindow::OnConvertResourceClicked() {
    // 已在SetupConvertResourceTab中实现
}

void MainWindow::OnLoadResourcePackClicked() {
    // 已在SetupResourcePackTab中实现
}

void MainWindow::OnClearLogClicked() {
    logTextEdit_->clear();
}

void MainWindow::OnAboutClicked() {
    QMessageBox::about(this, "关于我的世界统一器",
        "我的世界统一器 v2.0.0\n\n"
        "一个跨平台的我的世界模组统一工具，支持Java版、基岩版和网易版之间的资源互通。\n\n"
        "功能特性：\n"
        "• 模组打包：将Java版/网易版模组打包为.cmc格式\n"
        "• 游戏注入：将统一器注入到游戏中\n"
        "• 资源转换：转换不同版本之间的资源格式\n"
        "• 资源包管理：加载和管理资源包\n\n"
        "联系方式：jqyh1026@outlook.com\n\n"
        "版权声明：\n"
        "Mojang Studios 拥有《我的世界》的版权，本项目未经授权。\n"
        "个人使用可直接使用，非个人使用、商用需获得授权。");
}

void MainWindow::OnTaskCompleted(bool success, const QString& message) {
    if (success) {
        LogMessage("[成功] " + message);
        statusLabel_->setText("任务完成");
    } else {
        LogMessage("[失败] " + message);
        statusLabel_->setText("任务失败");
    }
    
    // 重新启用按钮
    packModButton_->setEnabled(!selectedModFile_.empty());
    injectGameButton_->setEnabled(!selectedGameFile_.empty());
    convertResourceButton_->setEnabled(!selectedResourceFile_.empty());
    loadPackButton_->setEnabled(!selectedPackFile_.empty());
}

void MainWindow::OnProgressUpdated(int value) {
    progressBar_->setValue(value);
}

void MainWindow::OnLogMessage(const QString& message) {
    LogMessage(message);
}

void MainWindow::LogMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString logMessage = "[" + timestamp + "] " + message;
    
    logTextEdit_->append(logMessage);
    
    // 根据当前Tab输出到相应的文本框
    int currentIndex = tabWidget_->currentIndex();
    switch (currentIndex) {
        case 0: // 打包模组
            packModOutput_->append(logMessage);
            break;
        case 1: // 注入游戏
            injectGameOutput_->append(logMessage);
            break;
        case 2: // 转换资源
            convertResourceOutput_->append(logMessage);
            break;
        case 3: // 资源包
            resourcePackOutput_->append(logMessage);
            break;
        default:
            break;
    }
}

void MainWindow::UpdateProgress(int value) {
    progressBar_->setValue(value);
}

} // namespace desktop
} // namespace gui
} // namespace mcu
