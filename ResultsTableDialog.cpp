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
    QLabel *windowLabel = new QLabel(QString("Window Size: %1").arg(windowSize), this);
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
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);

    // Data rows
    table->setRowCount(lines.size() - 1);

    int invalidCount = 0; // <-- Add this

    for (int i = 1; i < lines.size(); ++i) {
        QStringList fields = lines[i].split(",");
        for (int j = 0; j < fields.size(); ++j) {
            QTableWidgetItem *item = new QTableWidgetItem(fields[j]);
            table->setItem(i-1, j, item);
        }
        // Validation highlighting and counting
        QTableWidgetItem *validItem = table->item(i-1, 7); // "Valid" column
        if (validItem && validItem->text().toLower() != "true") {
            validItem->setBackground(Qt::red);
            validItem->setForeground(Qt::white);
            ++invalidCount; // <-- Count invalid
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
    // Insert below window size label (which is at index 0)
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(this->layout());
    if (layout) layout->insertWidget(1, statsLabel);
}