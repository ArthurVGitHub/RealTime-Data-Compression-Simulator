#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CompressorRunner.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include "ResultsTableDialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), currentAlgorithm("DRH") {
    ui->setupUi(this);

    // Populate combo box
    ui->algorithmComboBox->addItems({"DRH"}); // Add more as you implement them

    // Connect combo box change
    connect(ui->algorithmComboBox, &QComboBox::currentTextChanged,
            this, &MainWindow::onAlgorithmChanged);

    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::on_runButton_clicked);

    connect(ui->saveCsvButton, &QPushButton::clicked, this, &MainWindow::on_saveCsvButton_clicked);
    connect(ui->visualizeButton, &QPushButton::clicked, this, &MainWindow::on_visualizeButton_clicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onAlgorithmChanged(const QString &text) {
    runner.setAlgorithm(text.toStdString());
}


void MainWindow::on_runButton_clicked() {
    QString filename = ui->fileLineEdit->text();
    int windowSize = 10;

    runner.setAlgorithm(currentAlgorithm.toStdString());
    runner.runCompression(filename.toStdString(), windowSize);

    QString summary = QString::fromStdString(runner.getSummaryText());
    ui->resultsTextEdit->setText(summary);
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
    int windowSize = 10; // Or get from UI

    ResultsTableDialog dialog(csvData, windowSize, this);
    dialog.exec(); // Modal, blocks until closed
    // No need to 'delete' if you use stack allocation
}
