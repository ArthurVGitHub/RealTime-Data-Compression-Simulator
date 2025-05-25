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
          ui(std::make_unique<Ui::MainWindow>()),   // <<-- correct initialization
          currentAlgorithm("DRH")
{
    ui->setupUi(this);
    ui->windowSizeSpinBox->setMinimum(1);

    // Populate combo box
    //ui->algorithmComboBox->addItems({"DRH"}); // Add more as you implement them done graphically now

    // Connect combo box change
    connect(ui->algorithmComboBox, &QComboBox::currentTextChanged,
            this, &MainWindow::onAlgorithmChanged);

    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::on_runButton_clicked);

    connect(ui->saveCsvButton, &QPushButton::clicked, this, &MainWindow::on_saveCsvButton_clicked);
    connect(ui->visualizeButton, &QPushButton::clicked, this, &MainWindow::on_visualizeButton_clicked);
    connect(ui->visualizeGraphButton, &QPushButton::clicked, this, &MainWindow::on_visualizeGraphButton_clicked);
    connect(ui->windowSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onWindowSizeChanged);
    // Disable window size spinbox when adaptive is enabled
    connect(ui->adaptiveCheckBox, &QCheckBox::toggled, ui->windowSizeSpinBox, &QSpinBox::setDisabled);

}

MainWindow::~MainWindow() = default; // unique_ptr auto-deletes ui

void MainWindow::onAlgorithmChanged(const QString &text) {
    runner.setAlgorithm(text.toStdString());
}


void MainWindow::on_runButton_clicked() {
    // 1. Get user input
    QString filename = ui->fileLineEdit->text();
    QString selectedAlgorithm = ui->algorithmComboBox->currentText();

    runner.setAlgorithm(selectedAlgorithm.toStdString());

    bool useAdaptive = ui->adaptiveCheckBox->isChecked();
    int windowSize = ui->windowSizeSpinBox->value();

    // 2. Run compression
    runner.runCompression(filename.toStdString(), windowSize, useAdaptive);

    // 3. Update summary text
    QString summary = QString::fromStdString(runner.getSummaryText());
    ui->resultsTextEdit->setText(summary);
    qDebug() << "Running with algorithm:" << selectedAlgorithm;
}

void MainWindow::onWindowSizeChanged(int size) {
    // Optional: handle window size changes here (e.g., validation, logging)
}


void MainWindow::on_saveCsvButton_clicked() {
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

    QTextStream out(&file);
    out << QString::fromStdString(runner.getSummaryText());

    file.close();
    QMessageBox::information(this, "Saved", "Results saved to:\n" + fileName);
}

void MainWindow::on_visualizeButton_clicked() {
    if (runner.getResults().empty()) {
        QMessageBox::information(this, "No Data", "Run compression first to see results.");
        return;
    }

    QString csvData = QString::fromStdString(runner.getSummaryText());
    //int windowSize = ui->windowSizeSpinBox->value();
    ResultsTableDialog dialog(csvData, initialWindowSize, this);
    dialog.exec(); // Modal, blocks until closed
    // No need to 'delete' if you use stack allocation

}
void MainWindow::on_visualizeGraphButton_clicked() {
    if (runner.getResults().empty()) {
        QMessageBox::information(this, "No Data", "Run compression first to see results.");
        return;
    }

    auto originals = runner.getOriginalData();
    auto decompressed = runner.getDecompressedData();
    auto crPerWindow = runner.getCRPerWindow();

    SensorPlotDialog dialog(originals, decompressed, crPerWindow, this);
    dialog.exec();
}