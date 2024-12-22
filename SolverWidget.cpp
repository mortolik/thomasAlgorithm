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

    m_infoText = new QTextEdit(this);
    m_infoText->setReadOnly(true);

    m_resultsTable = new QTableWidget(this);

    m_plot = new QtCharts::QChartView(new QtCharts::QChart(), this);

    m_solveButton = new QPushButton("Solve", this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* inputLayout = new QHBoxLayout();

    inputLayout->addWidget(new QLabel("Number of divisions (n):"));
    inputLayout->addWidget(m_spinBoxN);
    inputLayout->addWidget(m_solveButton);

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_infoText);
    mainLayout->addWidget(m_plot);
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

    // Отображение результатов
    displayResults(result);
}

void SolverWidget::displayResults(const SolverModel::Result& result) {
    // Справка
    QString info;
    info += QString("Number of divisions (n): %1\n").arg(result.x.size() - 1);
    info += QString("Maximum error (ε1): %1\n").arg(result.maxError);

    double maxErrorX = result.x[std::distance(result.u.begin(),
                                              std::max_element(result.u.begin(), result.u.end()))];
    info += QString("Maximum deviation at x = %1\n").arg(maxErrorX);

    m_infoText->setText(info);

    // Таблица
    m_resultsTable->clear();
    m_resultsTable->setRowCount(result.x.size());
    m_resultsTable->setColumnCount(3);
    m_resultsTable->setHorizontalHeaderLabels({"x_i", "u(x_i)", "v(x_i)"});

    for (size_t i = 0; i < result.x.size(); ++i) {
        m_resultsTable->setItem(i, 0, new QTableWidgetItem(QString::number(result.x[i])));
        m_resultsTable->setItem(i, 1, new QTableWidgetItem(QString::number(result.u[i])));
    }
    m_resultsTable->resizeColumnsToContents();

    // Графики
    auto* chart = new QtCharts::QChart();
    auto* series = new QtCharts::QLineSeries();

    for (size_t i = 0; i < result.x.size(); ++i) {
        series->append(result.x[i], result.u[i]);
    }

    chart->addSeries(series);
    chart->setTitle("Numerical Solution");
    chart->createDefaultAxes();

    // Настройка осей
    auto* xAxis = new QtCharts::QValueAxis();
    auto* yAxis = new QtCharts::QValueAxis();
    xAxis->setTitleText("x");
    yAxis->setTitleText("u(x)");

    chart->setAxisX(xAxis, series);
    chart->setAxisY(yAxis, series);

    m_plot->setChart(chart);
}
