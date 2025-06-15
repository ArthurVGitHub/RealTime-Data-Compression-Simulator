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

/*// 2. Original data
        plot->addGraph();
        plot->graph(0)->setData(x, yOrig);
        plot->graph(0)->setPen(QPen(Qt::blue));
        plot->graph(0)->setName("Original (" + QString::fromStdString(sensor) + ")");


// 3. Decompressed data
        plot->addGraph();
        plot->graph(1)->setData(x, yDecomp);
        plot->graph(1)->setPen(QPen(Qt::green));
        plot->graph(1)->setName("Decompressed (" + QString::fromStdString(sensor) + ")");

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
        plot->plotLayout()->addElement(0, 0, new QCPTextElement(plot, "Original, decompressed and CR", QFont("sans", 10, QFont::Bold)));
        plot->setMinimumHeight(250);*/
// Enable right y-axis
        plot->yAxis2->setVisible(true);
        plot->yAxis->setLabel("CR");
        plot->yAxis2->setLabel("Sensor Value");

// Original data (right y-axis)
        plot->addGraph(plot->xAxis, plot->yAxis2);
        plot->graph(0)->setData(x, yOrig);
        plot->graph(0)->setPen(QPen(Qt::blue));
        plot->graph(0)->setName("Original (" + QString::fromStdString(sensor) + ")");

// Decompressed data (right y-axis)
        plot->addGraph(plot->xAxis, plot->yAxis2);
        plot->graph(1)->setData(x, yDecomp);
        plot->graph(1)->setPen(QPen(Qt::green));
        plot->graph(1)->setName("Decompressed (" + QString::fromStdString(sensor) + ")");

// CR curve (left y-axis)
        plot->addGraph(); // attaches to xAxis, yAxis by default
        plot->graph(2)->setData(x, yCR);
        plot->graph(2)->setPen(QPen(Qt::darkMagenta, 2, Qt::DashLine));
        plot->graph(2)->setName("CR");

// Labels, legend, etc.
        plot->xAxis->setLabel("Sample index");
        plot->legend->setVisible(true);
        plot->plotLayout()->insertRow(0);
        plot->plotLayout()->addElement(0, 0, new QCPTextElement(plot, "Original, decompressed and CR", QFont("sans", 10, QFont::Bold)));
        plot->setMinimumHeight(250);

// Rescale axes
        plot->yAxis->rescale(true);   // CR axis (left)
        plot->yAxis2->rescale(true);  // Value axis (right)
        plot->xAxis->rescale(true);

// Voeg toe aan je layout
        sensorLayout->addWidget(plot);
        mainLayout->addWidget(sensorWidget);

    }

    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);

    QVBoxLayout* dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(scrollArea);
}