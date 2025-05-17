#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CompressorRunner.h"
MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow), currentAlgorithm("DRH")
{
    ui->setupUi(this);

    // Populate combo box
    ui->algorithmComboBox->addItems({"DRH"}); // Add more as you implement them

    // Connect combo box change
    connect(ui->algorithmComboBox, &QComboBox::currentTextChanged,
            this, &MainWindow::onAlgorithmChanged);

    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::on_runButton_clicked);
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

    CompressorRunner runner;
    runner.setAlgorithm(currentAlgorithm.toStdString()); // NEW METHOD
    runner.runCompression(filename.toStdString(), windowSize);

    QString summary = QString::fromStdString(runner.getSummaryText());
    ui->resultsTextEdit->setText(summary);
}
