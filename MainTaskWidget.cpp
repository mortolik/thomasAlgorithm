#include "MainTaskWidget.hpp"
#include <QLabel>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLogValueAxis>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

using namespace QtCharts;

MainTaskWidget::MainTaskWidget(QWidget* parent)
    : QWidget(parent), m_model(nullptr) {
    setupUI();
}

void MainTaskWidget::setModel(SolverModel* model) {
    m_model = model;
}

void MainTaskWidget::setupUI() {
    // Инициализация элементов управления
    m_spinBoxN = new QSpinBox(this);
    m_spinBoxN->setRange(2, 1000000);
    m_spinBoxN->setValue(10);

    m_spinBoxEpsilon = new QDoubleSpinBox(this);
    m_spinBoxEpsilon->setRange(1e-12, 1.0);
    m_spinBoxEpsilon->setDecimals(12);
    m_spinBoxEpsilon->setSingleStep(1e-6);
    m_spinBoxEpsilon->setValue(1e-6);

    m_infoText = new QTextEdit(this);
    m_infoText->setReadOnly(true);

    m_solveButton = new QPushButton("Solve", this);
    m_helpButton = new QPushButton("Help", this);

    // Создание вкладок для результатов
    m_resultsTabWidget = new QTabWidget(this);

    // Вкладки для таблиц
    QWidget* tablesTab = new QWidget();
    QVBoxLayout* tablesLayout = new QVBoxLayout(tablesTab);

    m_resultsTable = new QTableWidget(this);
    m_refinedResultsTable = new QTableWidget(this);

    tablesLayout->addWidget(new QLabel("Main Results:"));
    tablesLayout->addWidget(m_resultsTable);
    tablesLayout->addWidget(new QLabel("Refined Results:"));
    tablesLayout->addWidget(m_refinedResultsTable);

    tablesTab->setLayout(tablesLayout);

    // Вкладки для графиков
    QWidget* plotsTab = new QWidget();
    QVBoxLayout* plotsLayout = new QVBoxLayout(plotsTab);

    // Создание под-вкладок для графиков
    QTabWidget* plotsSubTabWidget = new QTabWidget(this);

    // Solutions Plot
    QWidget* solutionsPlotTab = new QWidget();
    QVBoxLayout* solutionsPlotLayout = new QVBoxLayout(solutionsPlotTab);
    m_plot = new QChartView(new QChart(), this);
    solutionsPlotLayout->addWidget(m_plot);
    solutionsPlotTab->setLayout(solutionsPlotLayout);

    // Error Plot
    QWidget* errorPlotTab = new QWidget();
    QVBoxLayout* errorPlotLayout = new QVBoxLayout(errorPlotTab);
    m_errorPlot = new QChartView(new QChart(), this);
    errorPlotLayout->addWidget(m_errorPlot);
    errorPlotTab->setLayout(errorPlotLayout);

    // Log Error Plot
    QWidget* logErrorPlotTab = new QWidget();
    QVBoxLayout* logErrorPlotLayout = new QVBoxLayout(logErrorPlotTab);
    m_logErrorPlot = new QChartView(new QChart(), this);
    logErrorPlotLayout->addWidget(m_logErrorPlot);
    logErrorPlotTab->setLayout(logErrorPlotLayout);

    // Добавление под-вкладок
    plotsSubTabWidget->addTab(solutionsPlotTab, "Solutions Plot");
    plotsSubTabWidget->addTab(errorPlotTab, "Error Plot");
    plotsSubTabWidget->addTab(logErrorPlotTab, "Log Error Plot");

    plotsLayout->addWidget(plotsSubTabWidget);
    plotsTab->setLayout(plotsLayout);

    // Добавление вкладок в основной TabWidget
    m_resultsTabWidget->addTab(tablesTab, "Tables");
    m_resultsTabWidget->addTab(plotsTab, "Plots");

    // Организация основного макета
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* inputLayout = new QHBoxLayout();

    inputLayout->addWidget(new QLabel("Number of divisions (n):"));
    inputLayout->addWidget(m_spinBoxN);
    inputLayout->addWidget(new QLabel("Accuracy (epsilon):"));
    inputLayout->addWidget(m_spinBoxEpsilon);
    inputLayout->addWidget(m_solveButton);
    inputLayout->addWidget(m_helpButton);

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_infoText);
    mainLayout->addWidget(m_resultsTabWidget);

    setLayout(mainLayout);

    // Подключение сигналов и слотов
    connect(m_solveButton, &QPushButton::clicked, this, &MainTaskWidget::onSolveButtonClicked);
    connect(m_helpButton, &QPushButton::clicked, this, &MainTaskWidget::onHelpButtonClicked);
}

void MainTaskWidget::onSolveButtonClicked() {
    if (!m_model) return;

    // Установка параметров модели
    SolverModel::Params params;
    params.mu1 = 0.0; // Граничное условие
    params.mu2 = 0.0;
    params.xi = 0.5; // Точка разрыва для основной задачи
    params.n = m_spinBoxN->value();
    params.epsilon = m_spinBoxEpsilon->value();

    try {
        m_model->setParams(params);

        // Выполнение решения с уточнением
        SolverModel::Result result;
        SolverModel::Result refinedResult;

        result = m_model->solveWithAccuracy(params.epsilon);

        // Правильное присвоение уточнённых результатов
        refinedResult.x = result.x;
        refinedResult.u = result.uRefined;
        refinedResult.analytical = result.analytical;
        refinedResult.maxError = result.maxErrorRefined;

        // Отображение результатов
        displayResults(result, refinedResult);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
    }
}

void MainTaskWidget::displayResults(const SolverModel::Result& result, const SolverModel::Result& refinedResult) {
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

    // Таблица для уточнённых результатов
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
    QChart* chart = new QChart();
    QLineSeries* numericalSeries = new QLineSeries();
    QLineSeries* analyticalSeries = new QLineSeries();

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
    QChart* errorChart = new QChart();
    QLineSeries* errorSeries = new QLineSeries();

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
        // Отладочные сообщения
        qDebug() << "Convergence Data Size:" << result.convergenceData.size();
        for (const auto& data : result.convergenceData) {
            qDebug() << "n:" << data.n << ", error:" << data.error;
        }

        QChart* logErrorChart = new QChart();
        QLineSeries* logErrorSeries = new QLineSeries();

        for (const auto& data : result.convergenceData) {
            logErrorSeries->append(data.n, data.error);
        }

        logErrorChart->addSeries(logErrorSeries);
        logErrorChart->setTitle("Ошибка vs Количество разбиений (n)");

        // Создаём логарифмические оси
        QLogValueAxis* axisX = new QLogValueAxis;
        axisX->setTitleText("Количество разбиений (n)");
        axisX->setBase(10);
        axisX->setMinorTickCount(4);
        // axisX->setTickCount(result.convergenceData.size()); // Удалено

        QLogValueAxis* axisY = new QLogValueAxis;
        axisY->setTitleText("Ошибка");
        axisY->setBase(10);
        axisY->setMinorTickCount(4);
        // axisY->setTickCount(result.convergenceData.size()); // Удалено

        // Установка диапазона осей (опционально)
        // axisX->setRange(1, m_model->getMaxN()); // Если есть метод getMaxN()
        // axisY->setRange(1e-12, 1);

        // Добавляем оси к графику
        logErrorChart->addAxis(axisX, Qt::AlignBottom);
        logErrorChart->addAxis(axisY, Qt::AlignLeft);

        // Привязываем серию к осям
        logErrorSeries->attachAxis(axisX);
        logErrorSeries->attachAxis(axisY);

        logErrorSeries->setName("Ошибка");

        logErrorChart->legend()->hide();

        m_logErrorPlot->setChart(logErrorChart);
    }
}

void MainTaskWidget::onHelpButtonClicked() {
    QString helpText;
    helpText += "Основная задача:\n";
    helpText += "1. Установите количество разбиений (n) и точность (epsilon).\n";
    helpText += "2. Нажмите кнопку 'Solve' для выполнения основной задачи с уточнением сетки.\n";
    helpText += "3. Результаты решения будут отображены в таблицах и на графиках во вкладках 'Tables' и 'Plots'.\n";
    helpText += "   - Вкладка 'Tables' содержит основные и уточнённые результаты.\n";
    helpText += "   - Вкладка 'Plots' содержит графики решений, ошибок и логарифмического графика ошибки.\n";

    QMessageBox::information(this, "Справка", helpText);
}
