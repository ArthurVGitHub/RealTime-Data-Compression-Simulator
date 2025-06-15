#ifndef EXPORTWINDOW_H
#define EXPORTWINDOW_H

#include <QWidget>
#include <map>
#include <vector>
#include <QString>

namespace Ui {
    class exportWindow;
}

class exportWindow : public QWidget
{
Q_OBJECT

public:
    explicit exportWindow(QWidget *parent = nullptr);
    ~exportWindow();

    // Setter for decompressed data
    void setDecompressedData(const std::map<std::string, std::vector<double>>& data);

private slots:
    void on_pushButton_clicked();

private:
    Ui::exportWindow *ui;
    std::map<std::string, std::vector<double>> decompressedData;
};

#endif // EXPORTWINDOW_H
