#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QVBoxLayout>
#include <QLabel>
#include "CompressorRunner.h"
#include <QThread>

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
    void on_visualizeGraphButton_clicked();
    void updateLiveCrDisplay(const QString &sensorName, double cr);
private:
    int initialWindowSize = 20;
    QLabel* liveCrTitleLabel = nullptr; // Add this

    std::unique_ptr<Ui::MainWindow> ui;   // <<-- keep this private!
    QString currentAlgorithm;
    //CompressorRunner runner;
    void onWindowSizeChanged(int size);

    QThread workerThread;
    CompressorRunner* runner;

    std::map<QString, QLabel*> liveCrLabels;  // Maps sensor names to their labels
    QVBoxLayout* liveCrLayout;  // Added in Qt Designer
    void clearLiveCrDisplay();

    void updateLiveCrTitle(int interval);
};

#endif // MAINWINDOW_H
