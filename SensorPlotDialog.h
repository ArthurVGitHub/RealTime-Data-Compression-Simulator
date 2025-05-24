//
// Created by arthu on 23/05/2025.
//

#ifndef THESIS_SENSORPLOTDIALOG_H
#define THESIS_SENSORPLOTDIALOG_H
// SensorPlotDialog.h
#include <QDialog>
#include "graphLib/qcustomplot.h"

class SensorPlotDialog : public QDialog {
Q_OBJECT
public:
    SensorPlotDialog(const std::map<std::string, std::vector<double>>& originals,
                     const std::map<std::string, std::vector<double>>& decompressed,
                     QWidget* parent = nullptr);
};

#endif //THESIS_SENSORPLOTDIALOG_H
