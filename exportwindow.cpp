#include "exportwindow.h"
#include "ui_exportwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

/*exportWindow::exportWindow(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::exportWindow)
{
    ui->setupUi(this);
}*/
exportWindow::exportWindow(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::exportWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::Window);
}

exportWindow::~exportWindow()
{
    delete ui;
}

void exportWindow::setDecompressedData(const std::map<std::string, std::vector<double>>& data) {
    decompressedData = data;
}

void exportWindow::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export EMODnet Data", QDir::homePath(), "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file for writing.");
        return;
    }
    QTextStream out(&file);

    // Write metadata and header
    out << "// " << ui->plainTextEdit->toPlainText() << "\n";
    out << "Cruise\tStation\tType\tYYYY-MM-DDThh:mm:ss.sss\tLongitude [degrees_east]\tLatitude [degrees_north]\tLOCAL_CDI_ID\tEDMO_code\tBot. Depth [m]";
    for (const auto& [sensor, vec] : decompressedData) {
        out << "\t" << QString::fromStdString(sensor) << "\tQV:SEADATANET";
    }
    out << "\n";

    // Find the maximum number of rows
    size_t maxRows = 0;
    for (const auto& [_, vec] : decompressedData)
        if (vec.size() > maxRows) maxRows = vec.size();

    // Write data rows
    for (size_t i = 0; i < maxRows; ++i) {
        out << ui->CruisEdit->text() << "\t"
            << ui->StationEdit->text() << "\t"
            << ui->TypeEdit->text() << "\t"
            << ui->DateEdit->text() << "\t"
            << ui->LongitudeEdit->text() << "\t"
            << ui->LatitudeEdit->text() << "\t"
            << ui->LocalEdit->text() << "\t"
            << ui->EDMOEdit->text() << "\t"
            << ui->BotDepthEdit->text();

        for (const auto& [_, vec] : decompressedData) {
            if (i < vec.size())
                out << "\t" << vec[i] << "\t1";
            else
                out << "\t\t";
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export", "EMODnet data exported!");
}
