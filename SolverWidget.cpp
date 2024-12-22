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

    m_taskSelector = new QComboBox(this);
    m_taskSelector->addItem("Test Task");
    m_taskSelector->addItem("Main Task");

    m_infoText = new QTextEdit(this);
    m_infoText->setReadOnly(true);

    m_resultsTable = new QTableWidget(this);

    m_plot = new QtCharts::QChartView(new QtCharts::QChart(), this);
    m_errorPlot = new QtCharts::QChartView(new QtCharts::QChart(), this);

    m_solveButton = new QPushButton("Solve", this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* inputLayout = new QHBoxLayout();

    inputLayout->addWidget(new QLabel("Number of divisions (n):"));
    inputLayout->addWidget(m_spinBoxN);
    inputLayout->addWidget(new QLabel("Task type:"));
    inputLayout->addWidget(m_taskSelector);
    inputLayout->addWidget(m_solveButton);

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_infoText);
    mainLayout->addWidget(m_plot);
    mainLayout->addWidget(m_errorPlot);
    mainLayout->addWidget(m_resultsTable);

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

    m_model->setParams(params);

    // Выполнение решения
    SolverModel::Result result = m_model->solve();

    // Выполнение решения на удвоенной сетке для основной задачи
    SolverModel::Result refinedResult;
    if (m_taskSelector->currentIndex() == 1) { // Main Task
        params.n *= 2;
        m_model->setParams(params);
        refinedResult = m_model->solve();
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
        double maxError = 0.0;
        for (size_t i = 0; i < result.x.size(); ++i) {
            maxError = std::max(maxError, std::abs(result.u[i] - refinedResult.u[i * 2]));
        }
        info += QString("Refined grid error (ε2): %1\n").arg(maxError);
    }

    m_infoText->setText(info);

    // Таблица
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
