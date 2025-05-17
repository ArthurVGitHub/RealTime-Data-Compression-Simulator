#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "CompressorRunner.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_runButton_clicked();
    void onAlgorithmChanged(const QString &text);

private:
    Ui::MainWindow *ui;
    QString currentAlgorithm;
    CompressorRunner runner;
};

#endif // MAINWINDOW_H

