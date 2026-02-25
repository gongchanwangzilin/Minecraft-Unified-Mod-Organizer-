/**
 * Minecraft Unifier - Desktop GUI
 * 桌面端GUI - Qt6界面
 */

#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <string>
#include <vector>

namespace mcu {
namespace gui {
namespace desktop {

// 工作线程
class WorkerThread : public QThread {
    Q_OBJECT
public:
    enum class TaskType {
        PACK_MOD,
        INJECT_GAME,
        CONVERT_RESOURCE,
        LOAD_RESOURCE_PACK
    };
    
    WorkerThread(TaskType type, const std::string& input, const std::string& output);
    
signals:
    void progressUpdated(int value);
    void taskCompleted(bool success, const QString& message);
    void logMessage(const QString& message);
    
protected:
    void run() override;
    
private:
    TaskType taskType_;
    std::string input_;
    std::string output_;
    
    void PackMod();
    void InjectGame();
    void ConvertResource();
    void LoadResourcePack();
};

// 主窗口
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
private slots:
    void OnPackModClicked();
    void OnInjectGameClicked();
    void OnConvertResourceClicked();
    void OnLoadResourcePackClicked();
    void OnClearLogClicked();
    void OnAboutClicked();
    void OnTaskCompleted(bool success, const QString& message);
    void OnProgressUpdated(int value);
    void OnLogMessage(const QString& message);
    
private:
    void SetupUI();
    void SetupMenuBar();
    void SetupStatusBar();
    void SetupTabWidget();
    void SetupPackModTab();
    void SetupInjectGameTab();
    void SetupConvertResourceTab();
    void SetupResourcePackTab();
    void SetupLogTab();
    
    void LogMessage(const QString& message);
    void UpdateProgress(int value);
    
    // UI组件
    QWidget* centralWidget_;
    QVBoxLayout* mainLayout_;
    QTabWidget* tabWidget_;
    QProgressBar* progressBar_;
    QLabel* statusLabel_;
    
    // 打包模组Tab
    QWidget* packModTab_;
    QPushButton* selectModFileButton_;
    QPushButton* packModButton_;
    QLabel* modFileLabel_;
    QTextEdit* packModOutput_;
    
    // 注入游戏Tab
    QWidget* injectGameTab_;
    QPushButton* selectGameFileButton_;
    QPushButton* injectGameButton_;
    QLabel* gameFileLabel_;
    QTextEdit* injectGameOutput_;
    
    // 转换资源Tab
    QWidget* convertResourceTab_;
    QPushButton* selectResourceFileButton_;
    QPushButton* convertResourceButton_;
    QLabel* resourceFileLabel_;
    QTextEdit* convertResourceOutput_;
    
    // 资源包Tab
    QWidget* resourcePackTab_;
    QPushButton* selectPackFileButton_;
    QPushButton* loadPackButton_;
    QPushButton* unloadPackButton_;
    QLabel* packFileLabel_;
    QListWidget* loadedPacksList_;
    QTextEdit* resourcePackOutput_;
    
    // 日志Tab
    QWidget* logTab_;
    QTextEdit* logTextEdit_;
    QPushButton* clearLogButton_;
    
    // 工作线程
    WorkerThread* workerThread_;
    QMutex workerMutex_;
    
    // 文件路径
    std::string selectedModFile_;
    std::string selectedGameFile_;
    std::string selectedResourceFile_;
    std::string selectedPackFile_;
};

} // namespace desktop
} // namespace gui
} // namespace mcu
