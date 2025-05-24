//
// Created by arthu on 23/05/2025.
//

#include "SensorPlotDialog.h"

SensorPlotDialog::SensorPlotDialog(const std::map<std::string, std::vector<double>>& originals,
                                   const std::map<std::string, std::vector<double>>& decompressed,
                                   QWidget* parent)
        : QDialog(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QTabWidget* tabs = new QTabWidget(this);

    for (const auto& [sensor, origData] : originals) {
        QWidget* tab = new QWidget;
        QVBoxLayout* tabLayout = new QVBoxLayout(tab);
        QCustomPlot* plot = new QCustomPlot;
        QVector<double> x, yOrig, yDecomp;

        for (int i = 0; i < origData.size(); ++i) {
            x.push_back(i);
            yOrig.push_back(origData[i]);
            yDecomp.push_back(decompressed.at(sensor)[i]);
        }

        plot->addGraph();
        plot->graph(0)->setData(x, yOrig);
        plot->graph(0)->setPen(QPen(Qt::blue));
        plot->graph(0)->setName("Original");

        plot->addGraph();
        plot->graph(1)->setData(x, yDecomp);
        plot->graph(1)->setPen(QPen(Qt::red));
        plot->graph(1)->setName("Decompressed");

        plot->legend->setVisible(true);
        plot->xAxis->setLabel("Measurement");
        plot->yAxis->setLabel("Value");
        plot->rescaleAxes();

        tabLayout->addWidget(plot);
        tabs->addTab(tab, QString::fromStdString(sensor));
    }
    setMinimumSize(800, 600);
    layout->addWidget(tabs);
    setLayout(layout);
}
