//
// Created by arthu on 17/05/2025.
//

#include "ResultsTableDialog.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QLabel>

ResultsTableDialog::ResultsTableDialog(const QString& csvData, int windowSize, QWidget *parent)
        : QDialog(parent), table(new QTableWidget(this))
{
    setWindowTitle("Compression Results");
    resize(1000, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);

    // Add window size label
    QLabel *windowLabel = new QLabel(QString("Initial Window Size When Adaptive Window Is Enabled: %1").arg(windowSize), this);
    layout->addWidget(windowLabel);

    // Setup table
    layout->addWidget(table);
    setupTable(csvData, windowSize);
}

void ResultsTableDialog::setupTable(const QString& csvData, int windowSize) {
    QStringList lines = csvData.split("\n", Qt::SkipEmptyParts);
    if (lines.isEmpty()) return;

    // Headers
    QStringList headers = lines[0].split(",");
    int winSizeCol = headers.indexOf("Window Size");
    if (winSizeCol != -1)
        headers.removeAt(winSizeCol);
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);

    // Data rows
    table->setRowCount(lines.size() - 1);

    int invalidCount = 0;

    for (int i = 1; i < lines.size(); ++i) {
        QStringList fields = lines[i].split(",");
        if (winSizeCol != -1 && winSizeCol < fields.size())
            fields.removeAt(winSizeCol);

        for (int j = 0; j < fields.size(); ++j) {
            QTableWidgetItem *item = new QTableWidgetItem(fields[j]);
            table->setItem(i-1, j, item);
        }
        // Validation highlighting and counting
        // "Valid" column is now at (fields.size() - 1)
        QTableWidgetItem *validItem = table->item(i-1, fields.size() - 1);
        if (validItem && validItem->text().toLower() != "true") {
            validItem->setBackground(Qt::red);
            validItem->setForeground(Qt::white);
            ++invalidCount;
        }
    }

    // Formatting
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->setAlternatingRowColors(true);
    table->setStyleSheet("QTableWidget { font: 10pt; }");

    // Add summary statistics
    QLabel *statsLabel = new QLabel(QString("Total Sensors: %1 | Invalid: %2")
                                            .arg(table->rowCount())
                                            .arg(invalidCount), this);
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(this->layout());
    if (layout) layout->insertWidget(1, statsLabel);
}
