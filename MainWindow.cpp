#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QSpinBox>
#include "ResultsTableDialog.h"
#include "SensorPlotDialog.h"
#include <QVBoxLayout>
#include "graphLib/QCustomPlot.h"
MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent),
          ui(std::make_unique<Ui::MainWindow>()),
          currentAlgorithm("DRH"),
          runner(new CompressorRunner())
{
    ui->setupUi(this);
    ui->windowSizeSpinBox->setMinimum(1);
    ui->updateIntervalSpinBox->setValue(5); // Set default to 5

    // Move runner to worker thread
    runner->moveToThread(&workerThread);
    workerThread.start();

    // Only connect signals that do NOT use the auto-connect slot naming
    connect(ui->algorithmComboBox, &QComboBox::currentTextChanged,
            this, &MainWindow::onAlgorithmChanged);

    connect(ui->windowSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onWindowSizeChanged);

    // Disable window size spinbox when adaptive is enabled
    connect(ui->adaptiveCheckBox, &QCheckBox::toggled, ui->windowSizeSpinBox, &QSpinBox::setDisabled);


    // Connect CR updates (queued is automatic when objects are in different threads)
    connect(runner, &CompressorRunner::crUpdated, this, &MainWindow::updateLiveCrDisplay);

//    connect(runner, &CompressorRunner::compressionFinished, this, [this]() {
//        QString summary = QString::fromStdString(runner->getSummaryText());
//        ui->resultsTextEdit->setText(summary);
//    });
    connect(runner, &CompressorRunner::compressionFinished, this, [this]() {
        QString summary = QString::fromStdString(runner->getSummaryText());
        ui->resultsTextEdit->setText(summary);
        runFinishedLabel->setText("Run finished!"); // <-- Update the label
    });
    connect(ui->updateIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::updateLiveCrTitle);

    // Create scroll area and layout for live CR display
    QScrollArea* scrollArea = new QScrollArea(this);
    QWidget* scrollWidget = new QWidget(scrollArea);
    liveCrLayout = new QVBoxLayout(scrollWidget);
    scrollWidget->setLayout(liveCrLayout);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    ui->verticalLayout->addWidget(scrollArea);

    liveCrTitleLabel = new QLabel(this);
    liveCrTitleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    liveCrLayout->insertWidget(0, liveCrTitleLabel); // Insert at the top
    updateLiveCrTitle(ui->updateIntervalSpinBox->value()); // Set initial text

    runFinishedLabel = new QLabel(this);
    runFinishedLabel->setAlignment(Qt::AlignCenter);
    runFinishedLabel->setStyleSheet("font-weight: bold; color: green;");
    ui->verticalLayout->addWidget(runFinishedLabel);
    runFinishedLabel->clear();
}

MainWindow::~MainWindow() {
    workerThread.quit();
    workerThread.wait();
    delete runner;
}

// In on_runButton_clicked:
void MainWindow::on_runButton_clicked() {
    runFinishedLabel->clear();
    qDebug() << "runButton clicked!";
    int updateInterval = ui->updateIntervalSpinBox->value();
    clearLiveCrDisplay(); // Clear previous run's labels
    updateLiveCrTitle(ui->updateIntervalSpinBox->value());

    QString filename = ui->fileLineEdit->text();
    QString selectedAlgorithm = ui->algorithmComboBox->currentText();

    runner->setAlgorithm(selectedAlgorithm.toStdString());

    bool useAdaptive = ui->adaptiveCheckBox->isChecked();
    int windowSize = ui->windowSizeSpinBox->value();


//    QMetaObject::invokeMethod(runner, [this, filename, windowSize, useAdaptive, updateInterval]() {
//        runner->runCompression(filename.toStdString(), windowSize, useAdaptive, updateInterval);
//        emit runner->compressionFinished();
//    });
    QMetaObject::invokeMethod(runner, [this, filename, windowSize, useAdaptive, updateInterval]() {
        runner->runCompression(filename.toStdString(), windowSize, useAdaptive, updateInterval);
        emit runner->compressionFinished();
    });

}

void MainWindow::onAlgorithmChanged(const QString &text) {
    runner->setAlgorithm(text.toStdString());
}

void MainWindow::onWindowSizeChanged(int size) {
    // Optional: handle window size changes here (e.g., validation, logging)
}

void MainWindow::on_saveCsvButton_clicked() {
    if (runner->getResults().empty()) {
        QMessageBox::information(this, "No Data", "Run compression first to see results.");
        return;
    }

    QString defaultName = QDir::homePath() + "/compression_results.csv";
    QString fileName = QFileDialog::getSaveFileName(
            this,
            "Save Results as CSV",
            defaultName,
            "CSV Files (*.csv);;All Files (*)"
    );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Error", "Cannot open file for writing.");
        return;
    }

    QString dataCharacteristicsCsv = QString::fromStdString(runner->getDataCharacteristicsText());
    QString compressionResultsCsv = QString::fromStdString(runner->getSummaryText());
    QString combinedCsv = dataCharacteristicsCsv + "\n\n" + compressionResultsCsv;

    QTextStream out(&file);
    out << combinedCsv;

    file.close();
    QMessageBox::information(this, "Saved", "Results saved to:\n" + fileName);
}


void MainWindow::on_visualizeButton_clicked() {
    if (runner->getResults().empty()) {
        QMessageBox::information(this, "No Data", "Run compression first to see results.");
        return;
    }

    // Generate both CSV texts
    QString dataCharacteristicsCsv = QString::fromStdString(runner->getDataCharacteristicsText());
    QString compressionResultsCsv = QString::fromStdString(runner->getSummaryText());

    // Pass both to the dialog
    ResultsTableDialog dialog(dataCharacteristicsCsv, compressionResultsCsv, this);
    dialog.exec();
}

/*void MainWindow::on_visualizeButton_clicked() {
    if (runner->getResults().empty()) {
        QMessageBox::information(this, "No Data", "Run compression first to see results.");
        return;
    }

    QString csvData = QString::fromStdString(runner->getSummaryText());
    ResultsTableDialog dialog(csvData, initialWindowSize, this);
    dialog.exec(); // Modal, blocks until closed
}*/

void MainWindow::on_visualizeGraphButton_clicked() {
    if (runner->getResults().empty()) {
        QMessageBox::information(this, "No Data", "Run compression first to see results.");
        return;
    }

    auto originals = runner->getOriginalData();
    auto decompressed = runner->getDecompressedData();
    auto crPerWindow = runner->getCRPerWindow();

    SensorPlotDialog dialog(originals, decompressed, crPerWindow, this);
    dialog.exec();
}

void MainWindow::updateLiveCrDisplay(const QString& sensorName, double cr) {
    // Create or update labels for each sensor
    if (!liveCrLayout) return; // Guard against null
    if (!liveCrLabels.contains(sensorName)) {
        QLabel* label = new QLabel(this);
        liveCrLabels[sensorName] = label;
        liveCrLayout->addWidget(label);
    }
    liveCrLabels[sensorName]->setText(
            QString("%1: %2").arg(sensorName).arg(cr, 0, 'f', 2)
    );
}

void MainWindow::clearLiveCrDisplay() {
    if (!liveCrLayout) return;
    for (auto& [name, label] : liveCrLabels) {
        liveCrLayout->removeWidget(label);
        delete label;
    }
    liveCrLabels.clear();
}

void MainWindow::updateLiveCrTitle(int interval) {
    liveCrTitleLabel->setText(QString("Real-Time Compression Ratio (CR) (Every %1 Windows)").arg(interval));
}