#include "TestTaskWidget.hpp"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLogValueAxis>
#include <QTabWidget>
#include <QDebug>
#include <limits>
#include <cmath>

using namespace QtCharts;

TestTaskWidget::TestTaskWidget(QWidget* parent)
    : QWidget(parent), m_model(nullptr) {
    setupUI();
}

void TestTaskWidget::setModel(SolverModel* model) {
    m_model = model;
}

void TestTaskWidget::setupUI() {
    QLabel* labelN = new QLabel("Количество разбиений (n):", this);
    m_spinBoxN = new QSpinBox(this);
    m_spinBoxN->setRange(2, 1000000);
    m_spinBoxN->setValue(10);

    QLabel* labelEpsilon = new QLabel("Точность (epsilon):", this);
    m_spinBoxEpsilon = new QDoubleSpinBox(this);
    m_spinBoxEpsilon->setRange(1e-12, 1.0);
    m_spinBoxEpsilon->setDecimals(12);
    m_spinBoxEpsilon->setSingleStep(1e-6);
    m_spinBoxEpsilon->setValue(0.5e-6);

    m_solveButton = new QPushButton("Решить", this);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(labelN);
    inputLayout->addWidget(m_spinBoxN);
    inputLayout->addWidget(labelEpsilon);
    inputLayout->addWidget(m_spinBoxEpsilon);
    inputLayout->addWidget(m_solveButton);

    m_infoText = new QTextEdit(this);
    m_infoText->setMaximumHeight(100);
    m_infoText->setReadOnly(true);

    // Создание вкладок
    m_tabWidget = new QTabWidget(this);

    // Вкладка "Таблица"
    QWidget* tableTab = new QWidget(this);
    QVBoxLayout* tableLayout = new QVBoxLayout(tableTab);
    m_resultsTable = new QTableWidget(this);
    m_resultsTable->setColumnCount(4);
    m_resultsTable->setHorizontalHeaderLabels({"x_i", "u(x_i)", "v(x_i)", "u(x_i) - v(x_i)"});
    m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableLayout->addWidget(m_resultsTable);
    tableTab->setLayout(tableLayout);

    // Вкладка "Графики"
    QWidget* plotTab = new QWidget(this);
    QVBoxLayout* plotLayout = new QVBoxLayout(plotTab);
    m_plot = new QChartView(new QChart(), this);
    m_plot->setRenderHint(QPainter::Antialiasing);
    m_errorPlot = new QChartView(new QChart(), this);
    m_errorPlot->setRenderHint(QPainter::Antialiasing);
    plotLayout->addWidget(new QLabel("График решений:"));
    plotLayout->addWidget(m_plot);
    plotLayout->addWidget(new QLabel("График погрешности:"));
    plotLayout->addWidget(m_errorPlot);
    plotTab->setLayout(plotLayout);

    // Добавление вкладок
    m_tabWidget->addTab(tableTab, "Таблица");
    m_tabWidget->addTab(plotTab, "Графики");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_infoText);
    mainLayout->addWidget(m_tabWidget);

    setLayout(mainLayout);

    connect(m_solveButton, &QPushButton::clicked, this, &TestTaskWidget::onSolveButtonClicked);
}

void TestTaskWidget::onSolveButtonClicked() {
    if (!m_model) {
        QMessageBox::warning(this, "Предупреждение", "Модель не установлена.");
        return;
    }

    SolverModel::Params params;
    params.mu1 = 0.0;
    params.mu2 = 0.0;
    params.xi = 0.5;
    params.n = m_spinBoxN->value();
    params.epsilon = m_spinBoxEpsilon->value();

    try {
        m_model->setParams(params);
        SolverModel::Result result = m_model->solve();
        displayResults(result);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
    }
}

void TestTaskWidget::displayResults(const SolverModel::Result& result) {
    double maxDeviation = 0.0;
    double maxDeviationPoint = 0.0;
    for (size_t i = 0; i < result.x.size(); ++i) {
        double deviation = std::abs(result.u[i] - result.analytical[i]);
        if (deviation > maxDeviation) {
            maxDeviation = deviation;
            maxDeviationPoint = result.x[i];
        }
    }

    QString info;
    info += QString("Для решения задачи использована равномерная сетка с числом разбиений n = %1.\n").arg(result.x.size() - 1);
    info += "Задача должна быть решена с погрешностью не более ε = 0.5⋅10⁻⁶.\n";
    info += QString("Задача решена с погрешностью ε₁ = %1.\n").arg(result.maxError);
    info += QString("Максимальное отклонение аналитического и численного решений наблюдается в точке x = %1.\n").arg(maxDeviationPoint);
    m_infoText->setText(info);

    m_resultsTable->setRowCount(result.x.size());
    for (size_t i = 0; i < result.x.size(); ++i) {
        m_resultsTable->setItem(i, 0, new QTableWidgetItem(QString::number(result.x[i])));
        m_resultsTable->setItem(i, 1, new QTableWidgetItem(QString::number(result.analytical[i])));
        m_resultsTable->setItem(i, 2, new QTableWidgetItem(QString::number(result.u[i])));
        m_resultsTable->setItem(i, 3, new QTableWidgetItem(QString::number(result.u[i] - result.analytical[i])));
    }
    m_resultsTable->resizeColumnsToContents();

    QChart* chart = m_plot->chart();
    chart->removeAllSeries();

    QLineSeries* numericalSeries = new QLineSeries();
    QLineSeries* analyticalSeries = new QLineSeries();

    for (size_t i = 0; i < result.x.size(); ++i) {
        numericalSeries->append(result.x[i], result.u[i]);
        analyticalSeries->append(result.x[i], result.analytical[i]);
    }

    chart->addSeries(numericalSeries);
    chart->addSeries(analyticalSeries);
    chart->setTitle("Сравнение аналитического и численного решений");
    chart->createDefaultAxes();
    numericalSeries->setName("Численное решение");
    analyticalSeries->setName("Аналитическое решение");
    chart->legend()->show();

    QChart* errorChart = m_errorPlot->chart();
    errorChart->removeAllSeries();

    QLineSeries* errorSeries = new QLineSeries();
    for (size_t i = 0; i < result.x.size(); ++i) {
        errorSeries->append(result.x[i], std::abs(result.u[i] - result.analytical[i]));
    }

    errorChart->addSeries(errorSeries);
    errorChart->setTitle("График погрешности");
    errorChart->createDefaultAxes();
    errorSeries->setName("Погрешность");
    errorChart->legend()->show();
}
