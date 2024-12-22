#include "SolverWidget.hpp"
#include <QLabel>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLogValueAxis>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

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
    m_spinBoxN->setRange(2, 1000000);
    m_spinBoxN->setValue(10);

    m_spinBoxEpsilon = new QDoubleSpinBox(this);
    m_spinBoxEpsilon->setRange(1e-12, 1.0);
    m_spinBoxEpsilon->setDecimals(12);
    m_spinBoxEpsilon->setSingleStep(1e-6);
    m_spinBoxEpsilon->setValue(1e-6);

    m_taskSelector = new QComboBox(this);
    m_taskSelector->addItem("Test Task");
    m_taskSelector->addItem("Main Task");

    m_infoText = new QTextEdit(this);
    m_infoText->setReadOnly(true);

    m_resultsTable = new QTableWidget(this);
    m_refinedResultsTable = new QTableWidget(this); // Инициализация новой таблицы

    m_plot = new QtCharts::QChartView(new QtCharts::QChart(), this);
    m_errorPlot = new QtCharts::QChartView(new QtCharts::QChart(), this);
    m_logErrorPlot = new QtCharts::QChartView(new QtCharts::QChart(), this); // Инициализация нового графика

    m_solveButton = new QPushButton("Solve", this);
    m_helpButton = new QPushButton("Help", this); // Инициализация кнопки справки

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* inputLayout = new QHBoxLayout();

    inputLayout->addWidget(new QLabel("Number of divisions (n):"));
    inputLayout->addWidget(m_spinBoxN);
    inputLayout->addWidget(new QLabel("Accuracy (epsilon):"));
    inputLayout->addWidget(m_spinBoxEpsilon);
    inputLayout->addWidget(new QLabel("Task type:"));
    inputLayout->addWidget(m_taskSelector);
    inputLayout->addWidget(m_solveButton);
    inputLayout->addWidget(m_helpButton); // Добавление кнопки справки

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_infoText);
    mainLayout->addWidget(m_resultsTable);
    mainLayout->addWidget(m_refinedResultsTable); // Добавление новой таблицы
    mainLayout->addWidget(m_plot);
    mainLayout->addWidget(m_errorPlot);
    mainLayout->addWidget(m_logErrorPlot); // Добавление нового графика

    setLayout(mainLayout);

    connect(m_solveButton, &QPushButton::clicked, this, &SolverWidget::onSolveButtonClicked);
    connect(m_helpButton, &QPushButton::clicked, this, &SolverWidget::onHelpButtonClicked);
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

    try {
        m_model->setParams(params);

        // Выполнение решения
        SolverModel::Result result;
        SolverModel::Result refinedResult;

        if (m_taskSelector->currentIndex() == 0) { // Test Task
            result = m_model->solve();
        } else { // Main Task
            result = m_model->solveWithAccuracy(params.epsilon);

            // Правильное присвоение уточнённых результатов
            refinedResult.x = result.x;
            refinedResult.u = result.uRefined;
            refinedResult.analytical = result.analytical;
            refinedResult.maxError = result.maxErrorRefined;
        }

        // Отображение результатов
        displayResults(result, refinedResult);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
    }
}

void SolverWidget::displayResults(const SolverModel::Result& result, const SolverModel::Result& refinedResult) {
    // Справка
    QString info;
    info += QString("Количество разбиений (n): %1\n").arg(result.x.size() - 1);
    info += QString("Максимальная ошибка (ε1): %1\n").arg(result.maxError);

    if (!refinedResult.x.empty()) {
        double maxError = m_model->calculateGridError(result, refinedResult);
        info += QString("Максимальная ошибка на уточнённой сетке (ε2): %1\n").arg(maxError);
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

    // Таблица для основной задачи с уточнённой сеткой
    if (!refinedResult.u.empty()) {
        m_refinedResultsTable->clear();
        int refinedSize = refinedResult.x.size();
        int displaySize = refinedSize / 2 + 1;
        m_refinedResultsTable->setRowCount(displaySize);
        m_refinedResultsTable->setColumnCount(4);
        m_refinedResultsTable->setHorizontalHeaderLabels({"x_{2i}", "u2(x_{2i})", "v2(x_{2i})", "u2 - v2"});

        for (size_t i = 0; i < displaySize; ++i) {
            int idx = 2 * i;
            if (idx >= refinedSize) idx = refinedSize - 1; // Последний индекс
            m_refinedResultsTable->setItem(i, 0, new QTableWidgetItem(QString::number(refinedResult.x[idx])));
            m_refinedResultsTable->setItem(i, 1, new QTableWidgetItem(QString::number(refinedResult.u[idx])));
            m_refinedResultsTable->setItem(i, 2, new QTableWidgetItem(QString::number(refinedResult.analytical[idx])));
            m_refinedResultsTable->setItem(i, 3, new QTableWidgetItem(QString::number(refinedResult.u[idx] - refinedResult.analytical[idx])));
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
    chart->setTitle("Решения");
    chart->createDefaultAxes();

    numericalSeries->setName("Численное решение");
    analyticalSeries->setName("Аналитическое решение");

    m_plot->setChart(chart);

    // График ошибки
    auto* errorChart = new QtCharts::QChart();
    auto* errorSeries = new QtCharts::QLineSeries();

    for (size_t i = 0; i < result.x.size(); ++i) {
        errorSeries->append(result.x[i], std::abs(result.u[i] - result.analytical[i]));
    }

    errorChart->addSeries(errorSeries);
    errorChart->setTitle("Ошибка между решениями");
    errorChart->createDefaultAxes();

    errorSeries->setName("Ошибка");

    m_errorPlot->setChart(errorChart);

    // График ошибки vs n (логарифмический)
    if (!result.convergenceData.empty()) {
        auto* logErrorChart = new QtCharts::QChart();
        auto* logErrorSeries = new QtCharts::QLineSeries();

        for (const auto& data : result.convergenceData) {
            logErrorSeries->append(data.n, data.error);
        }

        logErrorChart->addSeries(logErrorSeries);
        logErrorChart->setTitle("Ошибка vs Количество разбиений (n)");
        logErrorChart->createDefaultAxes();

        // Установка логарифмической шкалы
        auto axisX = new QtCharts::QLogValueAxis;
        axisX->setTitleText("Количество разбиений (n)");
        logErrorChart->setAxisX(axisX, logErrorSeries);

        auto axisY = new QtCharts::QLogValueAxis;
        axisY->setTitleText("Ошибка");
        logErrorChart->setAxisY(axisY, logErrorSeries);

        logErrorSeries->setName("Ошибка");

        m_logErrorPlot->setChart(logErrorChart);
    }
}

void SolverWidget::onHelpButtonClicked() {
    QString helpText;
    helpText += "Программа решает краевую задачу методом прогонки (метод Томаса).\n";
    helpText += "Основные функции:\n";
    helpText += "1. Решение задачи на заданной сетке.\n";
    helpText += "2. Решение задачи с уточнением сетки для достижения заданной точности.\n";
    helpText += "3. Отображение решений и ошибок в табличной форме и на графиках.\n\n";
    helpText += "Использование:\n";
    helpText += "1. Установите количество разбиений (n) и точность (epsilon).\n";
    helpText += "2. Выберите тип задачи (Test Task или Main Task).\n";
    helpText += "3. Нажмите кнопку 'Solve' для выполнения решения.\n";

    QMessageBox::information(this, "Справка", helpText);
}
