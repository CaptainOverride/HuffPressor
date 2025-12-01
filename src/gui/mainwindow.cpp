#include "mainwindow.h"
#include "errors.h"
#include "huffmanTree.h"
#include <QThread>
#include <QApplication>
#include <QFileInfo>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    // Apply Dark Theme
    this->setStyleSheet(
        "QMainWindow { background-color: #1e1e1e; color: #ffffff; }"
        "QLabel { color: #ffffff; font-size: 14px; }"
        "QLineEdit { "
        "   background-color: #2d2d2d; color: #ffffff; border: 1px solid #3d3d3d; "
        "   border-radius: 5px; padding: 8px; font-size: 14px; "
        "}"
        "QPushButton { "
        "   background-color: #007acc; color: white; border: none; "
        "   border-radius: 5px; padding: 10px 20px; font-size: 14px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #005f9e; }"
        "QPushButton:pressed { background-color: #004a80; }"
        "QProgressBar { "
        "   border: 1px solid #3d3d3d; border-radius: 5px; text-align: center; color: white;"
        "   background-color: #2d2d2d;"
        "}"
        "QProgressBar::chunk { background-color: #007acc; border-radius: 4px; }"
        "QTextEdit { "
        "   background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #3d3d3d; "
        "   border-radius: 5px; font-family: Consolas, monospace; font-size: 12px;"
        "}"
    );

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    layout = new QVBoxLayout(centralWidget);
    
    // Title
    QLabel *title = new QLabel("HuffPressor - File Compression Tool", this);
    title->setAlignment(Qt::AlignCenter);
    QFont font = title->font();
    font.setBold(true);
    font.setPointSize(16);
    title->setFont(font);
    layout->addWidget(title);

    // File Selection
    QHBoxLayout *fileLayout = new QHBoxLayout();
    filePathInput = new QLineEdit(this);
    filePathInput->setPlaceholderText("Select a file...");
    browseButton = new QPushButton("Browse", this);
    fileLayout->addWidget(filePathInput);
    fileLayout->addWidget(browseButton);
    layout->addLayout(fileLayout);

    // Buttons
    QHBoxLayout *actionLayout = new QHBoxLayout();
    compressButton = new QPushButton("Compress", this);
    decompressButton = new QPushButton("Decompress", this);
    actionLayout->addWidget(compressButton);
    actionLayout->addWidget(decompressButton);
    layout->addLayout(actionLayout);

    // Progress
    progressBar = new QProgressBar(this);
    progressBar->setValue(0);
    layout->addWidget(progressBar);

    // Status
    statusLabel = new QLabel("Ready", this);
    layout->addWidget(statusLabel);

    // Log Output
    logOutput = new QTextEdit(this);
    logOutput->setReadOnly(true);
    layout->addWidget(logOutput);

    // Connections
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::selectFile);
    connect(compressButton, &QPushButton::clicked, this, &MainWindow::startCompression);
    connect(decompressButton, &QPushButton::clicked, this, &MainWindow::startDecompression);

    setMinimumSize(600, 500);
}

void MainWindow::log(const std::string& message) {
    logOutput->append(QString::fromStdString(message));
    // Auto-scroll to bottom
    QScrollBar *sb = logOutput->verticalScrollBar();
    sb->setValue(sb->maximum());
    QApplication::processEvents();
}

void MainWindow::selectFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select File");
    if (!fileName.isEmpty()) {
        filePathInput->setText(fileName);
    }
}

void MainWindow::startCompression() {
    QString inputFile = filePathInput->text();
    if (inputFile.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a file first.");
        return;
    }

    QString outputFile = inputFile + ".huff";
    
    statusLabel->setText("Compressing...");
    progressBar->setValue(0);
    logOutput->clear();
    log("Starting compression for: " + inputFile.toStdString());
    
    // TODO: Move to background thread
    Compressor compressor;
    HuffmanTree tree;
    
    compressor.setLogger([this](const std::string& msg){
        log(msg);
    });

    compressor.setProgressCallback([this](float p){
        progressBar->setValue(static_cast<int>(p));
        QApplication::processEvents(); // Keep UI responsive (temporary hack)
    });

    if (compressor.readFileAndBuildFrequency(inputFile.toStdString()) != ErrorCode::Success) {
        statusLabel->setText("Failed to read file.");
        log("Error: Failed to read file.");
        return;
    }

    tree.build(compressor.getFrequencyMap());
    tree.generateCodes();

    if (compressor.compressFile(inputFile.toStdString(), outputFile.toStdString(), tree.getHuffmanCodes(), tree.getRoot()) == ErrorCode::Success) {
        statusLabel->setText("Compression Complete!");
        log("Compression successful! Output: " + outputFile.toStdString());
        QMessageBox::information(this, "Success", "File compressed successfully!");
    } else {
        statusLabel->setText("Compression Failed.");
        log("Error: Compression failed.");
    }
}

void MainWindow::startDecompression() {
    QString inputFile = filePathInput->text();
    if (inputFile.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a file first.");
        return;
    }

    QString outputFile = inputFile + ".decompressed"; 
    // Ideally strip .huff extension if present
    if (inputFile.endsWith(".huff")) {
        outputFile = inputFile.left(inputFile.length() - 5);
    }

    statusLabel->setText("Decompressing...");
    progressBar->setValue(0);
    logOutput->clear();
    log("Starting decompression for: " + inputFile.toStdString());

    // TODO: Move to background thread
    Decompressor decompressor;
    
    decompressor.setLogger([this](const std::string& msg){
        log(msg);
    });

    decompressor.setProgressCallback([this](float p){
        progressBar->setValue(static_cast<int>(p));
        QApplication::processEvents();
    });

    if (decompressor.decompressFile(inputFile.toStdString(), outputFile.toStdString()) == ErrorCode::Success) {
        statusLabel->setText("Decompression Complete!");
        log("Decompression successful! Output: " + outputFile.toStdString());
        QMessageBox::information(this, "Success", "File decompressed successfully!");
    } else {
        statusLabel->setText("Decompression Failed.");
        log("Error: Decompression failed.");
    }
}
