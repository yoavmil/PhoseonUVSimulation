#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLineSeries>
#include <QtMath>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tab_1->layout()->addWidget(windowContainer = QWidget::createWindowContainer(window3D = new Widget3D, this));

    ui->uvHeightSpinBox->setValue(window3D->UVHeight());
    ui->uvOffsetSpinBox->setValue(window3D->UVOffset());
    ui->uvAngleSpinBox->setValue(window3D->UVDegrees());
    ui->shieldRadiuspinBox->setValue(window3D->ShieldRadius());
    ui->shieldHeightSpinBox->setValue(window3D->ShieldHeight());
    ui->fragmentTextEdit->setText(window3D->getSurfaceFrag().data());

    connect(ui->uvHeightSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::uvHeightSpinBox_valueChanged);
    connect(ui->uvOffsetSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::uvOffsetSpinBox_valueChanged);
    connect(ui->uvAngleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::uvAngleSpinBox_valueChanged);
    connect(ui->shieldRadiuspinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::shieldRadiuspinBox_valueChanged);
    connect(ui->shieldHeightSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::shieldHeightSpinBox_valueChanged);
    connect(ui->fragmentCompileButton, &QPushButton::clicked, this, &MainWindow::fragmentCompileButton_clicked);
    connect(ui->uvTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::uvTypeChanged);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabIndexChanged);

    //chart stuff
    ui->tab_3->layout()->removeWidget(ui->chartWidgetStub);
    ui->tab_3->layout()->addWidget(chartView = new QChartView(this));
    chartView->setRenderHint(QPainter::Antialiasing);

    connect(ui->exportChartButton, &QPushButton::clicked, this, &MainWindow::exportChart);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::uvHeightSpinBox_valueChanged(double arg1)
{
    window3D->setUVHeight(arg1);
}

void MainWindow::uvOffsetSpinBox_valueChanged(double arg1)
{
    window3D->setUVOffset(arg1);
}

void MainWindow::uvAngleSpinBox_valueChanged(double arg1)
{
    window3D->setUVDegrees(arg1);
}

void MainWindow::shieldRadiuspinBox_valueChanged(double arg1)
{
    window3D->setShieldRadius(arg1);
}

void MainWindow::shieldHeightSpinBox_valueChanged(double arg1)
{
    window3D->setShieldHeight(arg1);
}

void MainWindow::fragmentCompileButton_clicked()
{
    window3D->setSurfaceFrag(ui->fragmentTextEdit->toPlainText().toStdString());
    window3D->compileSurfaceProgram(window3D->getSurfaceVert(), window3D->getSurfaceFrag());
    window3D->renderLater();
}

void MainWindow::tabIndexChanged(int i)
{
    if (i == 0) {
        window3D->SetPerspective(true);
        ui->tab_1->layout()->addWidget(windowContainer);
    }
    if (i == 1) {
        window3D->SetPerspective(false);
        ui->tab_2->layout()->addWidget(windowContainer);
    }
    if (i == 2) {
        calcChart();
    }
}

void MainWindow::uvTypeChanged(int i)
{
    window3D->uvType = Widget3D::UVType (i);
    window3D->renderLater();
}

void MainWindow::calcChart()
{
    chartView->chart()->removeAllSeries();

    vector<vector<float>> map = window3D->GetMap();

    QLineSeries* series;

    series = new QLineSeries();
    series->setName("intesity VS angle");
    for (float i = 0; i < 360; i++) {
        float value = 0;
        for (float r = 0; r < map.size() / 2; r += map.size() / 10) {
            size_t x = r * cos(qDegreesToRadians(i)) + map.size() / 2;
            size_t y = r * sin(qDegreesToRadians(i)) + map.size() / 2;
            value += map[y][x];
        }
        series->append(i, value);
    }
    chartView->chart()->addSeries(series);
    chartView->chart()->createDefaultAxes();
    chartView->chart()->axisY()->setMin(0);
}

void MainWindow::exportChart()
{

}
