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
    // Apply Cyberpunk / Futuristic Theme
    this->setStyleSheet(
        "QMainWindow { "
        "   background: qradialgradient(cx:0.5, cy:0.5, radius: 1, fx:0.5, fy:0.5, stop:0 #1b2735, stop:1 #090a0f); "
        "   font-family: 'Segoe UI', sans-serif; "
        "}"
        "QLabel { color: #e0e0e0; font-size: 14px; background: transparent; }"
        
        // Action Buttons (Neon Blue/Purple)
        "QPushButton { "
        "   background-color: rgba(0, 0, 0, 0.3); "
        "   color: #00e5ff; border: 1px solid #00e5ff; "
        "   border-radius: 8px; padding: 12px 24px; font-size: 14px; font-weight: bold; letter-spacing: 1px;"
        "}"
        "QPushButton:hover { "
        "   background-color: rgba(0, 229, 255, 0.1); "
        "   color: #ffffff; "
        "   border: 1px solid #ffffff; "
        "}"
        "QPushButton:pressed { background-color: rgba(0, 229, 255, 0.2); }"
        "QPushButton:disabled { border-color: #444; color: #444; }"
        
        // Drop Zone (Holographic)
        "#dropZone { "
        "   background-color: rgba(0, 0, 0, 0.2); border: 2px dashed #00e5ff; border-radius: 16px; "
        "   color: #00e5ff; font-size: 20px; padding: 40px; font-weight: bold;"
        "}"
        "#dropZone:hover { "
        "   background-color: rgba(0, 229, 255, 0.05); border-color: #ffffff; color: #ffffff; "
        "}"
        
        // Save Button (Neon Green)
        "#saveButton { border-color: #00ff00; color: #00ff00; }"
        "#saveButton:hover { background-color: rgba(0, 255, 0, 0.1); color: #ffffff; border-color: #ffffff; }"
        
        // Action Button (Neon Pink)
        "#actionButton { border-color: #ff00ff; color: #ff00ff; }"
        "#actionButton:hover { background-color: rgba(255, 0, 255, 0.1); color: #ffffff; border-color: #ffffff; }"
        
        // Home Cards (Glass Panels)
        "#homeBtn { "
        "   background-color: rgba(255, 255, 255, 0.03); "
        "   border: 1px solid rgba(255, 255, 255, 0.1); "
        "   border-radius: 16px; font-size: 18px; padding: 20px; text-align: center; color: #e0e0e0;"
        "}"
        "#homeBtn:hover { "
        "   background-color: rgba(255, 255, 255, 0.08); "
        "   border: 1px solid #00e5ff; color: #ffffff;"
        "}"
        
        "#backButton { border: none; color: #888; text-align: left; padding: 0; }"
        "#backButton:hover { color: #00e5ff; background: transparent; border: none; }"
        
        "QProgressBar { "
        "   border: 1px solid #333; background-color: #000; border-radius: 4px; height: 6px; text-align: center;"
        "}"
        "QProgressBar::chunk { "
        "   background-color: #00e5ff; border-radius: 4px; "
        "}"
        
        "QTextEdit { "
        "   background-color: rgba(0, 0, 0, 0.5); color: #00ff00; border: 1px solid #333; "
        "   border-radius: 8px; font-family: 'Consolas', monospace; font-size: 12px; padding: 12px;"
        "}"
    );

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget);

    setupHomePage();
    setupProcessPage();

    stackedWidget->setCurrentWidget(homePage);

    setMinimumSize(800, 600);

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

void MainWindow::setupHomePage() {
    homePage = new QWidget(this);
    QVBoxLayout *homeLayout = new QVBoxLayout(homePage);
    homeLayout->setSpacing(20);
    homeLayout->setAlignment(Qt::AlignCenter);
    homeLayout->setContentsMargins(40, 40, 40, 40);

    // Title
    QLabel *title = new QLabel("HuffPressor", homePage);
    title->setAlignment(Qt::AlignCenter);
    QFont font("Segoe UI");
    font.setBold(true);
    font.setPointSize(42);
    title->setFont(font);
    title->setStyleSheet("color: #00e5ff; letter-spacing: 3px; text-shadow: 0 0 20px rgba(0, 229, 255, 0.5);");
    homeLayout->addWidget(title);

    QLabel *subtitle = new QLabel("Advanced Huffman Compression Tool", homePage);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("color: #66b3cc; font-size: 16px; margin-bottom: 20px;");
    homeLayout->addWidget(subtitle);

    homeLayout->addStretch(1);

    // Grid Layout for Buttons
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(30);
    gridLayout->setAlignment(Qt::AlignCenter);

    // Row 1: Compression
    compressFileBtn = new QPushButton("📄\nCompress File", homePage);
    compressFileBtn->setObjectName("homeBtn");
    compressFileBtn->setCursor(Qt::PointingHandCursor);
    compressFileBtn->setMinimumSize(220, 140);

    compressFolderBtn = new QPushButton("📂\nCompress Folder", homePage);
    compressFolderBtn->setObjectName("homeBtn");
    compressFolderBtn->setCursor(Qt::PointingHandCursor);
    compressFolderBtn->setMinimumSize(220, 140);

    // Row 2: Decompression
    decompressFileBtn = new QPushButton("🔓\nDecompress File", homePage);
    decompressFileBtn->setObjectName("homeBtn");
    decompressFileBtn->setCursor(Qt::PointingHandCursor);
    decompressFileBtn->setMinimumSize(220, 140);

    decompressFolderBtn = new QPushButton("📦\nDecompress Folder", homePage);
    decompressFolderBtn->setObjectName("homeBtn");
    decompressFolderBtn->setCursor(Qt::PointingHandCursor);
    decompressFolderBtn->setMinimumSize(220, 140);

    gridLayout->addWidget(compressFileBtn, 0, 0);
    gridLayout->addWidget(compressFolderBtn, 0, 1);
    gridLayout->addWidget(decompressFileBtn, 1, 0);
    gridLayout->addWidget(decompressFolderBtn, 1, 1);

    homeLayout->addLayout(gridLayout);

    homeLayout->addStretch(2);

    // Footer
    QLabel *footer = new QLabel(homePage);
    footer->setTextFormat(Qt::RichText);
    footer->setTextInteractionFlags(Qt::TextBrowserInteraction);
    footer->setOpenExternalLinks(true);
    footer->setAlignment(Qt::AlignCenter);
    footer->setText(
        "<span style='color: #555; font-size: 11px;'>v1.0.0 • C++ & Qt • Created by </span>"
        "<a href='https://github.com/CaptainOverride' style='color: #00e5ff; text-decoration: none; font-weight: bold; font-size: 11px;'>CaptainOverride</a>"
    );
    homeLayout->addWidget(footer);

    // Connections
    connect(compressFileBtn, &QPushButton::clicked, this, [this](){ 
        switchToProcessPage(false); 
        isCompressionMode = true;
        dropZone->setText("Drag && Drop File Here\nor Click to Browse");
        hintLabel->setText("Supported: .txt, .md, .cpp, .py, .json, .xml, .html, .css, .log, .csv");
        hintLabel->setStyleSheet("color: #00e5ff; font-weight: bold; font-size: 13px;");
    });
    
    connect(compressFolderBtn, &QPushButton::clicked, this, [this](){ 
        switchToProcessPage(true); 
        isCompressionMode = true;
        dropZone->setText("Drag && Drop Folder Here\nor Click to Browse");
        hintLabel->setText("Supported: All Folder Types (Text-heavy Recommended)");
        hintLabel->setStyleSheet("color: #00e5ff; font-weight: bold; font-size: 13px;");
    });
    
    connect(decompressFileBtn, &QPushButton::clicked, this, [this](){ 
        switchToProcessPage(false); 
        isCompressionMode = false;
        dropZone->setText("Drag && Drop .hpf File Here\nor Click to Browse");
        hintLabel->setText("Target: Single Compressed File (.hpf)");
        hintLabel->setStyleSheet("color: #00e5ff; font-weight: bold; font-size: 13px;");
    });

    connect(decompressFolderBtn, &QPushButton::clicked, this, [this](){ 
        switchToProcessPage(true); 
        isCompressionMode = false;
        dropZone->setText("Drag && Drop .hpa Archive Here\nor Click to Browse");
        hintLabel->setText("Target: Compressed Folder Archive (.hpa)");
        hintLabel->setStyleSheet("color: #00e5ff; font-weight: bold; font-size: 13px;");
    });

    stackedWidget->addWidget(homePage);
}

void MainWindow::setupProcessPage() {
    processPage = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(processPage);
    layout->setSpacing(20);
    layout->setContentsMargins(30, 30, 30, 30);

    // Back Button
    backButton = new QPushButton("⬅ Home", processPage);
    backButton->setObjectName("backButton");
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setFixedWidth(120);
    connect(backButton, &QPushButton::clicked, this, &MainWindow::goBack);
    layout->addWidget(backButton);

    // Drop Zone
    dropZone = new QPushButton(this);
    dropZone->setObjectName("dropZone");
    dropZone->setCursor(Qt::PointingHandCursor);
    dropZone->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(dropZone);

    // Hint Label
    hintLabel = new QLabel("", processPage);
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setStyleSheet("color: #888; font-size: 12px; margin-top: -10px; margin-bottom: 10px;");
    layout->addWidget(hintLabel);

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

    stackedWidget->addWidget(processPage);
}

void MainWindow::switchToProcessPage(bool folderMode) {
    isFolderMode = folderMode;
    selectedFilePath.clear();
    
    // Reset UI
    fileInfoLabel->setVisible(false);
    actionButton->setVisible(false);
    saveButton->setVisible(false);
    progressBar->setValue(0);
    logOutput->clear();
    statusLabel->setText("Ready");

    if (isFolderMode) {
        dropZone->setText("Drag & Drop FOLDER Here\nor Click to Browse");
        hintLabel->setText("Supports all folder types (Text-heavy folders recommended)");
    } else {
        dropZone->setText("Drag & Drop FILE Here\nor Click to Browse");
        hintLabel->setText("Supported Formats: .txt, .md, .cpp, .py, .json, .xml, .html, .css, .log, .csv...");
    }

    stackedWidget->setCurrentWidget(processPage);
}

void MainWindow::goBack() {
    stackedWidget->setCurrentWidget(homePage);
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

bool MainWindow::isTextFile(const QString& path) {
    QStringList allowed = { "txt", "md", "cpp", "h", "c", "hpp", "py", "java", "js", "ts", "html", "css", "json", "xml", "log", "csv", "cmake", "yaml", "yml", "ini", "bat", "sh" };
    QString suffix = QFileInfo(path).suffix().toLower();
    return allowed.contains(suffix);
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
    
    QString suffix = fi.suffix().toLower();
    if (suffix == "hpf" || suffix == "hpa") {
        // It's a compressed file -> Decompress
        actionButton->setText("Decompress " + (suffix == "hpa" ? QString("Archive") : QString("File")));
        connect(actionButton, &QPushButton::clicked, this, &MainWindow::startDecompression);
        isCompressionMode = false; 
    } else {
        // It's a file or folder -> Compress
        if (isFolderMode && !fi.isDir()) {
            QMessageBox::warning(this, "Invalid Input", "You selected 'Compress Folder' but dropped a file.\nPlease drop a folder.");
            selectedFilePath.clear();
            updateSmartUI(); // Reset
            return;
        }
        if (!isFolderMode && fi.isDir()) {
            QMessageBox::warning(this, "Invalid Input", "You selected 'Compress File' but dropped a folder.\nPlease drop a file.");
            selectedFilePath.clear();
            updateSmartUI(); // Reset
            return;
        }

        // Enforce Text-Only Restriction for Files
        if (!isFolderMode && !fi.isDir()) {
            if (!isTextFile(selectedFilePath)) {
                 QMessageBox::information(this, "Optimization Notice", 
                    "HuffPressor is designed for text-based files.\n"
                    "Binary files (images, videos, etc.) are already compressed and won't benefit from Huffman coding.\n\n"
                    "Please select a text file (e.g., .txt, .cpp, .py) to see the magic!");
                 selectedFilePath.clear();
                 updateSmartUI();
                 return;
            }
        }

        actionButton->setText("Compress " + (fi.isDir() ? QString("Folder") : QString("File")));
        connect(actionButton, &QPushButton::clicked, this, &MainWindow::startCompression);
        isCompressionMode = true; 
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
                // If on home page, auto-detect mode if it's a compressed file
                if (stackedWidget->currentWidget() == homePage) {
                    QString suffix = QFileInfo(fileName).suffix().toLower();
                    if (suffix == "hpf" || suffix == "hpa") {
                        switchToProcessPage(false); // Mode doesn't matter for decompress really
                        selectedFilePath = fileName;
                        updateSmartUI();
                        return;
                    }
                    return;
                }

                // Validate file type based on current mode
                QFileInfo fi(fileName);
                QString suffix = fi.suffix().toLower();
                
                // If in compression mode, only accept text files or folders
                if (isCompressionMode) {
                    if (!fi.isDir() && !isTextFile(fileName)) {
                        QMessageBox::warning(this, "Invalid File Type", 
                            "Only text files are accepted for compression.\n"
                            "Supported: .txt, .md, .cpp, .py, .json, .xml, .html, .css, .log, .csv, etc.");
                        return;
                    }
                } else {
                    // In decompression mode, only accept .hpf or .hpa files
                    if (suffix != "hpf" && suffix != "hpa") {
                        QMessageBox::warning(this, "Invalid File Type", 
                            "Only HuffPressor files (.hpf or .hpa) are accepted for decompression.");
                        return;
                    }
                }

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
    backButton->setEnabled(enabled);
}

void MainWindow::selectFile() {
    QString fileName;
    if (isFolderMode) {
        if (isCompressionMode) {
            // Compressing a folder
            fileName = QFileDialog::getExistingDirectory(this, "Select Folder to Compress");
        } else {
            // Decompressing an archive - show .hpa files
            fileName = QFileDialog::getOpenFileName(this, "Select Archive to Decompress", "", 
                "HuffPressor Archive (*.hpa)");
        }
    } else {
        // Different filters based on compression vs decompression mode
        if (isCompressionMode) {
            // For compression: only allow text files
            fileName = QFileDialog::getOpenFileName(this, "Select File to Compress", "", 
                "Text Files (*.txt *.md *.cpp *.h *.c *.hpp *.py *.java *.js *.ts *.html *.css *.json *.xml *.log *.csv *.cmake *.yaml *.yml *.ini *.bat *.sh)");
        } else {
            // For decompression: only allow .hpf files
            fileName = QFileDialog::getOpenFileName(this, "Select Compressed File", "", 
                "HuffPressor File (*.hpf)");
        }
    }
    
    if (!fileName.isEmpty()) {
        selectedFilePath = fileName;
        updateSmartUI();
    }
}

void MainWindow::startCompression() {
    if (selectedFilePath.isEmpty()) return;

    // Generate Temp Output Path
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString ext = isFolderMode ? ".hpa" : ".hpf";
    currentTempFile = tempDir + "/huffpressor_temp" + ext;
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
    QFileInfo tempFi(currentTempFile);
    
    if (tempFi.isDir()) {
        // We are saving a FOLDER (Extracted Archive)
        // Ask user where to put it
        QString targetDir = QFileDialog::getExistingDirectory(this, "Select Destination Folder for Extraction");
        if (targetDir.isEmpty()) return;

        // Determine the output folder name
        // Use the original archive name (e.g., "Data" from "Data.hpa")
        QFileInfo originalFi(selectedFilePath);
        QString folderName = originalFi.completeBaseName();
        if (folderName.isEmpty()) folderName = "Decompressed_Output";

        QString destination = targetDir + "/" + folderName;

        // Check if destination already exists
        if (fs::exists(destination.toStdString())) {
            auto reply = QMessageBox::question(this, "Overwrite?", 
                "Folder '" + folderName + "' already exists in the destination.\nDo you want to overwrite it?",
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) return;
            
            // Remove existing to overwrite
            try {
                fs::remove_all(destination.toStdString());
            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error", QString("Failed to remove existing folder: ") + e.what());
                return;
            }
        }

        // Copy recursively
        auto copyRecursively = [](const fs::path& src, const fs::path& dest) {
            try {
                fs::copy(src, dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                return true;
            } catch (...) { return false; }
        };

        if (copyRecursively(currentTempFile.toStdString(), destination.toStdString())) {
             QMessageBox::information(this, "Saved", "Folder extracted successfully to:\n" + destination);
             log("Folder saved to: " + destination.toStdString());
        } else {
             QMessageBox::critical(this, "Error", "Failed to save folder.");
        }

    } else {
        // We are saving a FILE
        QString filter = isCompressionMode ? "HuffPressor File (*.hpf);;HuffPressor Archive (*.hpa);;All Files (*.*)" : "All Files (*.*)";
        QString defaultName = selectedFilePath;
        
        if (isCompressionMode) {
            defaultName += (isFolderMode ? ".hpa" : ".hpf");
        } else {
            // Decompression: Remove extension
            if (defaultName.endsWith(".hpf", Qt::CaseInsensitive) || defaultName.endsWith(".hpa", Qt::CaseInsensitive)) {
                defaultName = defaultName.left(defaultName.length() - 4);
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
}

