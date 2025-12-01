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
#include "compressor.h"
#include "decompressor.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectFile();
    void startCompression();
    void startDecompression();

private:
    QWidget *centralWidget;
    QVBoxLayout *layout;
    QLabel *statusLabel;
    QLineEdit *filePathInput;
    QPushButton *browseButton;
    QPushButton *compressButton;
    QPushButton *decompressButton;
    QProgressBar *progressBar;
    QTextEdit *logOutput;

    void setupUI();
    void log(const std::string& message);
};

#endif // MAINWINDOW_H
