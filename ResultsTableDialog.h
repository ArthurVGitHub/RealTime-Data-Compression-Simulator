//
// Created by arthu on 17/05/2025.
//

#ifndef THESIS_RESULTSTABLEDIALOG_H
#define THESIS_RESULTSTABLEDIALOG_H
#include <QDialog>
#include <QTableWidget>

class ResultsTableDialog : public QDialog {
Q_OBJECT
public:
    explicit ResultsTableDialog(const QString& csvData, int windowSize, QWidget *parent = nullptr);

private:
    QTableWidget *table;
    void setupTable(const QString& csvData, int windowSize);
};

#endif //THESIS_RESULTSTABLEDIALOG_H
