#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
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
    QLabel *statusLabel;
    
    // UI Elements
    QPushButton *dropZone;      // The "Google Drive" style box
    QPushButton *compressButton;
    QPushButton *decompressButton;
    QPushButton *saveButton;    // The "Download" button
    QProgressBar *progressBar;
    QTextEdit *logOutput;

    // State
    QString selectedFilePath;
    QString currentTempFile;
    bool isCompressionMode;     // To know if we are saving a .huff or .decompressed

    QThread* workerThread;
    Worker* worker;

    void setupUI();
    void log(const std::string& message);
    void setButtonsEnabled(bool enabled);
    void updateDropZoneText();
};

#endif // MAINWINDOW_H
