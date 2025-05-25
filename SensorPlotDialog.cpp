//
// Created by arthu on 23/05/2025.
//

#include "SensorPlotDialog.h"

SensorPlotDialog::SensorPlotDialog(
        const std::map<std::string, std::vector<double>>& originals,
        const std::map<std::string, std::vector<double>>& decompressed,
        const std::map<std::string, std::vector<double>>& crPerWindow,
        QWidget* parent
) : QDialog(parent)
{
    setWindowTitle("Sensor Plots");
    setMinimumSize(1000, 600);

    QScrollArea* scrollArea = new QScrollArea(this);
    QWidget* scrollContent = new QWidget;
    QVBoxLayout* mainLayout = new QVBoxLayout(scrollContent);

    for (const auto& [sensor, origData] : originals) {
        if (decompressed.count(sensor) == 0) continue;

        // Container voor deze sensor
        QWidget* sensorWidget = new QWidget;
        QVBoxLayout* sensorLayout = new QVBoxLayout(sensorWidget);

        QCustomPlot* plot = new QCustomPlot(sensorWidget);

        // 1. Data voorbereiden
        QVector<double> x, yOrig, yDecomp, yCR;
        //const auto& origData = originals.at(sensor);
        const auto& decompData = decompressed.at(sensor);
        const auto& crVec = crPerWindow.at(sensor);

        int n = origData.size();
        for (int i = 0; i < n; ++i) {
            x.push_back(i);
            yOrig.push_back(origData[i]);
            yDecomp.push_back(decompData[i]);
            // CR per window is meestal minder punten (1 waarde per window)
            // Je kunt deze over de window plotten, of interpoleren over alle samples
            if (i < crVec.size()) {
                yCR.push_back(crVec[i]);
            } else {
                yCR.push_back(crVec.back()); // laatste waarde herhalen
            }
        }

// 2. Originele data
        plot->addGraph();
        plot->graph(0)->setData(x, yOrig);
        plot->graph(0)->setPen(QPen(Qt::blue));
        plot->graph(0)->setName("Originel (" + QString::fromStdString(sensor) + ")");


// 3. Gedecomprimeerde data
        plot->addGraph();
        plot->graph(1)->setData(x, yDecomp);
        plot->graph(1)->setPen(QPen(Qt::green));
        plot->graph(1)->setName("Decompressede (" + QString::fromStdString(sensor) + ")");

// 4. CR-curve (optioneel schalen als de schaal heel anders is)
        plot->addGraph();
        plot->graph(2)->setData(x, yCR);
        plot->graph(2)->setPen(QPen(Qt::darkMagenta, 2, Qt::DashLine));
        plot->graph(2)->setName("CR");

// 5. Labels en legenda
        plot->xAxis->setLabel("Sample index");
        plot->yAxis->setLabel("Value / CR");
        plot->legend->setVisible(true);
        plot->rescaleAxes();
        plot->plotLayout()->insertRow(0);
        plot->plotLayout()->addElement(0, 0, new QCPTextElement(plot, "Originel, decompressed en CR", QFont("sans", 10, QFont::Bold)));
        plot->setMinimumHeight(250);

// Voeg toe aan je layout
        sensorLayout->addWidget(plot);
        mainLayout->addWidget(sensorWidget);

    }

    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);

    QVBoxLayout* dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(scrollArea);
}
