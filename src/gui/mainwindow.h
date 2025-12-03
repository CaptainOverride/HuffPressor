#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QProgressBar>
#include <QLineEdit>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include "compressor.h"
#include "decompressor.h"
#include "worker.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void selectFile();
    void startCompression();
    void startDecompression();
    void handleResults(bool success, const QString& message);

    void saveFile();

signals:
    void requestCompression(const QString& input, const QString& output);
    void requestDecompression(const QString& input, const QString& output);

private:
    QWidget *centralWidget;
    QVBoxLayout *layout;
    // UI Elements
    QStackedWidget *stackedWidget;
    QWidget *homePage;
    QWidget *processPage;

    // Home Page
    QPushButton *compressFileBtn;
    QPushButton *compressFolderBtn;

    // Process Page
    QPushButton *backButton;
    QPushButton *dropZone;      // The "Google Drive" style box
    QLabel *fileInfoLabel;      // Shows filename and original size
    QPushButton *actionButton;  // Single smart button (Compress or Decompress)
    QPushButton *saveButton;    // The "Download" button
    QProgressBar *progressBar;
    QTextEdit *logOutput;
    QLabel *statusLabel;

    // State
    QString selectedFilePath;
    QString currentTempFile;
    bool isCompressionMode;     // To know if we are saving a .huff or .decompressed
    bool isFolderMode;          // True if user selected "Compress Folder"
    uint64_t originalSize;

    QThread* workerThread;
    Worker* worker;

    void setupUI();
    void setupHomePage();
    void setupProcessPage();
    
    void log(const std::string& message);
    void setButtonsEnabled(bool enabled);
    
    // Helpers
    uint64_t getPathSize(const QString& path);
    QString formatSize(uint64_t bytes);
    void updateSmartUI(); // Decides which button to show
    void switchToProcessPage(bool folderMode);
    void goBack();
    bool isTextFile(const QString& path);
};

#endif // MAINWINDOW_H
