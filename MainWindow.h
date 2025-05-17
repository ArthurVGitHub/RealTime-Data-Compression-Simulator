#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
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
    void on_saveCsvButton_clicked();
    void on_runButton_clicked();
    void onAlgorithmChanged(const QString &text);
    void on_visualizeButton_clicked();

private:
    int initialWindowSize = 20;
    std::unique_ptr<Ui::MainWindow> ui;   // <<-- keep this private!
    QString currentAlgorithm;
    CompressorRunner runner;
};

#endif // MAINWINDOW_H
