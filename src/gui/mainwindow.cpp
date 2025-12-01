#include "mainWindow.h"
#include "errors.h"
#include "huffmanTree.h"
#include <QThread>
#include <QApplication>
#include <QFileInfo>
#include <QScrollBar>
#include <QMimeData>
#include <QUrl>
#include <QStandardPaths>
#include <filesystem>
#include <cmath>

namespace fs = std::filesystem;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

void MainWindow::setupUI() {
    // Apply Dark Theme
    this->setStyleSheet(
        "QMainWindow { background-color: #1e1e1e; color: #ffffff; }"
        "QLabel { color: #ffffff; font-size: 14px; }"
        "QPushButton { "
        "   background-color: #007acc; color: white; border: none; "
        "   border-radius: 5px; padding: 10px 20px; font-size: 14px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #005f9e; }"
        "QPushButton:pressed { background-color: #004a80; }"
        "QPushButton:disabled { background-color: #3d3d3d; color: #888888; }"
        "#dropZone { "
        "   background-color: #252526; border: 2px dashed #555555; border-radius: 10px; "
        "   color: #aaaaaa; font-size: 16px; padding: 40px; "
        "}"
        "#dropZone:hover { border-color: #007acc; color: #ffffff; background-color: #2d2d30; }"
        "#saveButton { background-color: #2ea043; }" // Green for save
        "#saveButton:hover { background-color: #238636; }"
        "#actionButton { background-color: #007acc; font-size: 16px; padding: 15px; }"
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
    layout->setSpacing(20);
    layout->setContentsMargins(30, 30, 30, 30);
    
    // Title
    QLabel *title = new QLabel("HuffPressor", this);
    title->setAlignment(Qt::AlignCenter);
    QFont font = title->font();
    font.setBold(true);
    font.setPointSize(24);
    title->setFont(font);
    layout->addWidget(title);

    // Drop Zone
    dropZone = new QPushButton(this);
    dropZone->setObjectName("dropZone");
    dropZone->setText("Drag & Drop File/Folder Here\nor Click to Browse");
    dropZone->setCursor(Qt::PointingHandCursor);
    dropZone->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(dropZone);

    // File Info Label (Hidden initially)
    fileInfoLabel = new QLabel("", this);
    fileInfoLabel->setAlignment(Qt::AlignCenter);
    fileInfoLabel->setStyleSheet("font-size: 16px; color: #cccccc; margin: 10px;");
    fileInfoLabel->setVisible(false);
    layout->addWidget(fileInfoLabel);

    // Smart Action Button (Hidden initially)
    actionButton = new QPushButton("Action", this);
    actionButton->setObjectName("actionButton");
    actionButton->setCursor(Qt::PointingHandCursor);
    actionButton->setVisible(false);
    layout->addWidget(actionButton);

    // Progress
    progressBar = new QProgressBar(this);
    progressBar->setValue(0);
    progressBar->setTextVisible(false);
    progressBar->setFixedHeight(5);
    layout->addWidget(progressBar);

    // Save Button (Hidden initially)
    saveButton = new QPushButton("Download / Save File", this);
    saveButton->setObjectName("saveButton");
    saveButton->setCursor(Qt::PointingHandCursor);
    saveButton->setVisible(false);
    layout->addWidget(saveButton);

    // Status
    statusLabel = new QLabel("Ready", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel);

    // Log Output
    logOutput = new QTextEdit(this);
    logOutput->setReadOnly(true);
    logOutput->setMaximumHeight(150);
    layout->addWidget(logOutput);

    // Connections
    connect(dropZone, &QPushButton::clicked, this, &MainWindow::selectFile);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveFile);
    // actionButton connection is dynamic or handled in a slot

    setMinimumSize(700, 600);

    // Threading Setup
    workerThread = new QThread(this);
    worker = new Worker();
    worker->moveToThread(workerThread);

    // Connect Signals
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &MainWindow::requestCompression, worker, &Worker::processCompression);
    connect(this, &MainWindow::requestDecompression, worker, &Worker::processDecompression);
    connect(worker, &Worker::progressUpdated, progressBar, &QProgressBar::setValue, Qt::QueuedConnection);
    connect(worker, &Worker::logMessage, this, [this](const QString& msg){
        log(msg.toStdString());
    }, Qt::QueuedConnection);
    connect(worker, &Worker::operationFinished, this, &MainWindow::handleResults, Qt::QueuedConnection);

    workerThread->start();

    // Enable Drag & Drop
    setAcceptDrops(true);
}

uint64_t MainWindow::getPathSize(const QString& path) {
    std::string p = path.toStdString();
    if (!fs::exists(p)) return 0;
    
    if (fs::is_regular_file(p)) {
        return fs::file_size(p);
    }
    
    if (fs::is_directory(p)) {
        uint64_t size = 0;
        for (const auto& entry : fs::recursive_directory_iterator(p)) {
            if (entry.is_regular_file()) {
                size += entry.file_size();
            }
        }
        return size;
    }
    return 0;
}

QString MainWindow::formatSize(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double size = static_cast<double>(bytes);
    while (size >= 1024 && i < 4) {
        size /= 1024;
        i++;
    }
    return QString::number(size, 'f', 2) + " " + units[i];
}

void MainWindow::updateSmartUI() {
    if (selectedFilePath.isEmpty()) return;

    QFileInfo fi(selectedFilePath);
    originalSize = getPathSize(selectedFilePath);
    
    // Update Drop Zone Text
    dropZone->setText("Selected:\n" + fi.fileName());

    // Update Info Label
    fileInfoLabel->setText("Original Size: " + formatSize(originalSize));
    fileInfoLabel->setVisible(true);

    // Determine Action
    actionButton->disconnect(); // Remove old connections
    
    if (fi.suffix() == "huff") {
        // It's a compressed file -> Decompress
        actionButton->setText("Decompress File");
        connect(actionButton, &QPushButton::clicked, this, &MainWindow::startDecompression);
        isCompressionMode = false; // Will be set to false
    } else {
        // It's a file or folder -> Compress
        actionButton->setText("Compress " + (fi.isDir() ? QString("Folder") : QString("File")));
        connect(actionButton, &QPushButton::clicked, this, &MainWindow::startCompression);
        isCompressionMode = true; // Will be set to true
    }
    
    actionButton->setVisible(true);
    saveButton->setVisible(false);
    progressBar->setValue(0);
    statusLabel->setText("Ready");
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        if (!urlList.isEmpty()) {
            QString fileName = urlList.first().toLocalFile();
            if (!fileName.isEmpty()) {
                selectedFilePath = fileName;
                updateSmartUI();
                log("Selected: " + fileName.toStdString());
            }
        }
    }
}

MainWindow::~MainWindow() {
    workerThread->quit();
    workerThread->wait();
}

void MainWindow::log(const std::string& message) {
    logOutput->append(QString::fromStdString(message));
    QScrollBar *sb = logOutput->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void MainWindow::setButtonsEnabled(bool enabled) {
    actionButton->setEnabled(enabled);
    dropZone->setEnabled(enabled);
}

void MainWindow::updateDropZoneText() {
    // Deprecated by updateSmartUI, keeping empty or removing usage
}

void MainWindow::selectFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select File");
    if (!fileName.isEmpty()) {
        selectedFilePath = fileName;
        updateSmartUI();
    }
}

void MainWindow::startCompression() {
    if (selectedFilePath.isEmpty()) return;

    // Generate Temp Output Path
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    currentTempFile = tempDir + "/huffpressor_temp.huff";
    isCompressionMode = true;

    statusLabel->setText("Compressing...");
    progressBar->setValue(0);
    logOutput->clear();
    log("Starting compression...");
    
    setButtonsEnabled(false);
    saveButton->setVisible(false);
    emit requestCompression(selectedFilePath, currentTempFile);
}

void MainWindow::startDecompression() {
    if (selectedFilePath.isEmpty()) return;

    // Generate Temp Output Path
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    currentTempFile = tempDir + "/huffpressor_temp.decompressed"; // Will be renamed/extracted later
    isCompressionMode = false;

    statusLabel->setText("Decompressing...");
    progressBar->setValue(0);
    logOutput->clear();
    log("Starting decompression...");

    setButtonsEnabled(false);
    saveButton->setVisible(false);
    emit requestDecompression(selectedFilePath, currentTempFile);
}

void MainWindow::handleResults(bool success, const QString& message) {
    setButtonsEnabled(true);
    statusLabel->setText(success ? "Processing Complete" : "Operation Failed");
    log(message.toStdString());
    
    if (success) {
        saveButton->setVisible(true);
        saveButton->setFocus();
        actionButton->setVisible(false); // Hide action button to focus on saving

        // Show New Size Stats
        uint64_t newSize = getPathSize(currentTempFile);
        QString stats = "Original: " + formatSize(originalSize) + "  ➜  New: " + formatSize(newSize);
        
        if (isCompressionMode && originalSize > 0) {
            double ratio = (1.0 - (double)newSize / originalSize) * 100.0;
            stats += QString(" (Saved %1%)").arg(ratio, 0, 'f', 1);
        }
        
        fileInfoLabel->setText(stats);
        fileInfoLabel->setStyleSheet("font-size: 16px; color: #4ec9b0; margin: 10px; font-weight: bold;");

    } else {
        QMessageBox::critical(this, "Error", message);
    }
}

void MainWindow::saveFile() {
    QString filter = isCompressionMode ? "HuffPressor Files (*.huff)" : "All Files (*.*)";
    QString defaultName = selectedFilePath;
    
    if (isCompressionMode) {
        defaultName += ".huff";
    } else {
        if (defaultName.endsWith(".huff")) {
            defaultName = defaultName.left(defaultName.length() - 5);
        } else {
            defaultName += ".decompressed";
        }
    }

    QString destination = QFileDialog::getSaveFileName(this, "Save File", defaultName, filter);
    if (destination.isEmpty()) return;

    // Move/Copy temp file to destination
    QFile::remove(destination); // Overwrite if exists
    if (QFile::copy(currentTempFile, destination)) {
        QMessageBox::information(this, "Saved", "File saved successfully!");
        log("File saved to: " + destination.toStdString());
    } else {
        QMessageBox::critical(this, "Error", "Failed to save file. Check permissions.");
        log("Error: Failed to save file to " + destination.toStdString());
    }
}
