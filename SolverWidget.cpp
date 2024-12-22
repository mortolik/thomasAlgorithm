// SolverWidget.cpp
#include "SolverWidget.hpp"
#include <QLabel>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

using namespace QtCharts;

SolverWidget::SolverWidget(QWidget* parent)
    : QWidget(parent), m_model(nullptr) {
    setupUI();
}

void SolverWidget::setModel(SolverModel* model) {
    m_model = model;
}

void SolverWidget::setupUI() {
    // Настройка интерфейса
    m_spinBoxN = new QSpinBox(this);
    m_spinBoxN->setRange(2, 10000);
    m_spinBoxN->setValue(10);

    m_spinBoxEpsilon = new QDoubleSpinBox(this);
    m_spinBoxEpsilon->setRange(0, 1e-2);
    m_spinBoxEpsilon->setDecimals(10);
    m_spinBoxEpsilon->setSingleStep(1e-6);
    m_spinBoxEpsilon->setValue(1e-6);

    m_taskSelector = new QComboBox(this);
    m_taskSelector->addItem("Test Task");
    m_taskSelector->addItem("Main Task");

    m_infoText = new QTextEdit(this);
    m_infoText->setReadOnly(true);

    m_resultsTable = new QTableWidget(this);

    m_plot = new QtCharts::QChartView(new QtCharts::QChart(), this);
    m_errorPlot = new QtCharts::QChartView(new QtCharts::QChart(), this);

    m_solveButton = new QPushButton("Solve", this);

    m_refinedResultsTable = new QTableWidget(this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* inputLayout = new QHBoxLayout();

    inputLayout->addWidget(new QLabel("Number of divisions (n):"));
    inputLayout->addWidget(m_spinBoxN);
    inputLayout->addWidget(new QLabel("Accuracy (epsilon):"));
    inputLayout->addWidget(m_spinBoxEpsilon);
    inputLayout->addWidget(new QLabel("Task type:"));
    inputLayout->addWidget(m_taskSelector);
    inputLayout->addWidget(m_solveButton);

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_infoText);
    mainLayout->addWidget(m_plot);
    mainLayout->addWidget(m_errorPlot);
    mainLayout->addWidget(m_resultsTable);
    mainLayout->addWidget(m_refinedResultsTable);

    setLayout(mainLayout);

    connect(m_solveButton, &QPushButton::clicked, this, &SolverWidget::onSolveButtonClicked);
}

void SolverWidget::onSolveButtonClicked() {
    if (!m_model) return;

    // Установка параметров модели
    SolverModel::Params params;
    params.mu1 = 0.0; // Граничное условие
    params.mu2 = 0.0;
    params.xi = 0.5; // Точка разрыва для 4-го варианта
    params.n = m_spinBoxN->value();
    params.epsilon = m_spinBoxEpsilon->value();

    m_model->setParams(params);

    // Выполнение решения
    SolverModel::Result result;
    SolverModel::Result refinedResult;

    if (m_taskSelector->currentIndex() == 0) { // Test Task
        result = m_model->solve();
    } else { // Main Task
        result = m_model->solve();
        refinedResult = m_model->solveWithAccuracy(params.epsilon);
    }

    // Отображение результатов
    displayResults(result, refinedResult);
}

void SolverWidget::displayResults(const SolverModel::Result& result, const SolverModel::Result& refinedResult) {
    // Справка
    QString info;
    info += QString("Number of divisions (n): %1\n").arg(result.x.size() - 1);
    info += QString("Maximum error (ε1): %1\n").arg(result.maxError);

    if (!refinedResult.x.empty()) {
        double maxError = m_model->calculateGridError(result, refinedResult);
        info += QString("Refined grid error (ε2): %1\n").arg(maxError);
    }

    m_infoText->setText(info);

    // Таблица основных результатов
    m_resultsTable->clear();
    m_resultsTable->setRowCount(result.x.size());
    m_resultsTable->setColumnCount(4);
    m_resultsTable->setHorizontalHeaderLabels({"x_i", "u(x_i)", "v(x_i)", "u(x_i) - v(x_i)"});

    for (size_t i = 0; i < result.x.size(); ++i) {
        m_resultsTable->setItem(i, 0, new QTableWidgetItem(QString::number(result.x[i])));
        m_resultsTable->setItem(i, 1, new QTableWidgetItem(QString::number(result.u[i])));
        m_resultsTable->setItem(i, 2, new QTableWidgetItem(QString::number(result.analytical[i])));
        m_resultsTable->setItem(i, 3, new QTableWidgetItem(QString::number(result.u[i] - result.analytical[i])));
    }
    m_resultsTable->resizeColumnsToContents();

    // Таблица для основной задачи с удвоенной сеткой
    if (!refinedResult.uRefined.empty()) {
        m_refinedResultsTable->clear();
        int refinedSize = refinedResult.x.size();
        m_refinedResultsTable->setRowCount(refinedSize / 2 + 1);
        m_refinedResultsTable->setColumnCount(4);
        m_refinedResultsTable->setHorizontalHeaderLabels({"x_{2i}", "u2(x_{2i})", "v2(x_{2i})", "u2 - v2"});

        for (size_t i = 0; i < m_refinedResultsTable->rowCount(); ++i) {
            int idx = 2 * i;
            if (idx >= refinedSize) idx = refinedSize - 1; // Последний индекс
            m_refinedResultsTable->setItem(i, 0, new QTableWidgetItem(QString::number(refinedResult.x[idx])));
            m_refinedResultsTable->setItem(i, 1, new QTableWidgetItem(QString::number(refinedResult.uRefined[idx])));
            m_refinedResultsTable->setItem(i, 2, new QTableWidgetItem(QString::number(refinedResult.analytical[idx])));
            m_refinedResultsTable->setItem(i, 3, new QTableWidgetItem(QString::number(refinedResult.uRefined[idx] - refinedResult.analytical[idx])));
        }
        m_refinedResultsTable->resizeColumnsToContents();
    }

    // График решений
    auto* chart = new QtCharts::QChart();
    auto* numericalSeries = new QtCharts::QLineSeries();
    auto* analyticalSeries = new QtCharts::QLineSeries();

    for (size_t i = 0; i < result.x.size(); ++i) {
        numericalSeries->append(result.x[i], result.u[i]);
        analyticalSeries->append(result.x[i], result.analytical[i]);
    }

    chart->addSeries(numericalSeries);
    chart->addSeries(analyticalSeries);
    chart->setTitle("Solutions");
    chart->createDefaultAxes();

    numericalSeries->setName("Numerical Solution");
    analyticalSeries->setName("Analytical Solution");

    m_plot->setChart(chart);

    // График ошибки
    auto* errorChart = new QtCharts::QChart();
    auto* errorSeries = new QtCharts::QLineSeries();

    for (size_t i = 0; i < result.x.size(); ++i) {
        errorSeries->append(result.x[i], std::abs(result.u[i] - result.analytical[i]));
    }

    errorChart->addSeries(errorSeries);
    errorChart->setTitle("Error between Solutions");
    errorChart->createDefaultAxes();

    errorSeries->setName("Error");

    m_errorPlot->setChart(errorChart);
}
