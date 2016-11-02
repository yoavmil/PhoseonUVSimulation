#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include "widget3d.h"

using namespace QtCharts;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void uvHeightSpinBox_valueChanged(double arg1);
    void uvOffsetSpinBox_valueChanged(double arg1);
    void uvAngleSpinBox_valueChanged(double arg1);
    void shieldRadiuspinBox_valueChanged(double arg1);
    void shieldHeightSpinBox_valueChanged(double arg1);
    void fragmentCompileButton_clicked();
    void tabIndexChanged(int i);
    void uvTypeChanged(int i);

private:
    Ui::MainWindow *ui;

    //chart stuff
    QChartView* chartView;
    void calcChart();
    void exportChart();

    QWidget* windowContainer;
    Widget3D* window3D;
};

#endif // MAINWINDOW_H
