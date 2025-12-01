#include "mainwindow.h"
#include "errors.h"
#include "huffmanTree.h"
#include <QThread>
#include <QApplication>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
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

    // Connections
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::selectFile);
    connect(compressButton, &QPushButton::clicked, this, &MainWindow::startCompression);
    connect(decompressButton, &QPushButton::clicked, this, &MainWindow::startDecompression);

    setMinimumSize(500, 300);
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
    
    // TODO: Move to background thread
    Compressor compressor;
    HuffmanTree tree;
    
    compressor.setProgressCallback([this](float p){
        progressBar->setValue(static_cast<int>(p));
        QApplication::processEvents(); // Keep UI responsive (temporary hack)
    });

    if (compressor.readFileAndBuildFrequency(inputFile.toStdString()) != ErrorCode::Success) {
        statusLabel->setText("Failed to read file.");
        return;
    }

    tree.build(compressor.getFrequencyMap());
    tree.generateCodes();

    if (compressor.compressFile(inputFile.toStdString(), outputFile.toStdString(), tree.getHuffmanCodes(), tree.getRoot()) == ErrorCode::Success) {
        statusLabel->setText("Compression Complete!");
        QMessageBox::information(this, "Success", "File compressed successfully!");
    } else {
        statusLabel->setText("Compression Failed.");
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

    // TODO: Move to background thread
    Decompressor decompressor;
    
    decompressor.setProgressCallback([this](float p){
        progressBar->setValue(static_cast<int>(p));
        QApplication::processEvents();
    });

    if (decompressor.decompressFile(inputFile.toStdString(), outputFile.toStdString()) == ErrorCode::Success) {
        statusLabel->setText("Decompression Complete!");
        QMessageBox::information(this, "Success", "File decompressed successfully!");
    } else {
        statusLabel->setText("Decompression Failed.");
    }
}
