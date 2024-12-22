#include "TestTaskWidget.hpp"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QDebug>

using namespace QtCharts;

TestTaskWidget::TestTaskWidget(QWidget* parent)
    : QWidget(parent), m_model(nullptr) {
    setupUI();
}

void TestTaskWidget::setModel(SolverModel* model) {
    m_model = model;
}

void TestTaskWidget::setupUI() {
    // Создание элементов управления
    QLabel* labelN = new QLabel("Количество разбиений (n):", this);
    m_spinBoxN = new QSpinBox(this);
    m_spinBoxN->setRange(2, 1000000);
    m_spinBoxN->setValue(10);

    QLabel* labelEpsilon = new QLabel("Точность (epsilon):", this);
    m_spinBoxEpsilon = new QDoubleSpinBox(this);
    m_spinBoxEpsilon->setRange(1e-12, 1.0);
    m_spinBoxEpsilon->setDecimals(12);
    m_spinBoxEpsilon->setSingleStep(1e-6);
    m_spinBoxEpsilon->setValue(1e-6);

    m_solveButton = new QPushButton("Решить", this);

    // Размещение элементов управления
    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(labelN);
    inputLayout->addWidget(m_spinBoxN);
    inputLayout->addWidget(labelEpsilon);
    inputLayout->addWidget(m_spinBoxEpsilon);
    inputLayout->addWidget(m_solveButton);

    // Создание области для информации и таблицы результатов
    m_infoText = new QTextEdit(this);
    m_infoText->setReadOnly(true);

    m_resultsTable = new QTableWidget(this);
    m_resultsTable->setColumnCount(4);
    m_resultsTable->setHorizontalHeaderLabels({"x_i", "u(x_i)", "v(x_i)", "u(x_i) - v(x_i)"});
    m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Создание графика решений
    m_plot = new QChartView(new QChart(), this);
    m_plot->setRenderHint(QPainter::Antialiasing);

    // Основной макет
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_infoText);
    mainLayout->addWidget(m_resultsTable);
    mainLayout->addWidget(m_plot);

    setLayout(mainLayout);

    // Подключение сигнала
    connect(m_solveButton, &QPushButton::clicked, this, &TestTaskWidget::onSolveButtonClicked);
}

void TestTaskWidget::onSolveButtonClicked() {
    if (!m_model) {
        QMessageBox::warning(this, "Предупреждение", "Модель не установлена.");
        return;
    }

    // Установка параметров модели
    SolverModel::Params params;
    params.mu1 = 0.0; // Граничное условие
    params.mu2 = 0.0;
    params.xi = 0.5; // Точка разрыва для тестовой задачи (может быть не используется)
    params.n = m_spinBoxN->value();
    params.epsilon = m_spinBoxEpsilon->value();

    try {
        m_model->setParams(params);

        // Выполнение решения тестовой задачи
        SolverModel::Result result = m_model->solve();

        // Отображение результатов
        displayResults(result);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
    }
}

void TestTaskWidget::displayResults(const SolverModel::Result& result) {
    // Обновление информационного текста
    QString info;
    info += QString("Количество разбиений (n): %1\n").arg(result.x.size() - 1);
    info += QString("Максимальная ошибка (ε1): %1\n").arg(result.maxError);
    m_infoText->setText(info);

    // Обновление таблицы результатов
    m_resultsTable->setRowCount(result.x.size());
    for (size_t i = 0; i < result.x.size(); ++i) {
        m_resultsTable->setItem(i, 0, new QTableWidgetItem(QString::number(result.x[i])));
        m_resultsTable->setItem(i, 1, new QTableWidgetItem(QString::number(result.u[i])));
        m_resultsTable->setItem(i, 2, new QTableWidgetItem(QString::number(result.analytical[i])));
        m_resultsTable->setItem(i, 3, new QTableWidgetItem(QString::number(result.u[i] - result.analytical[i])));
    }
    m_resultsTable->resizeColumnsToContents();

    // Обновление графика решений
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
    chart->setTitle("Решения");
    chart->createDefaultAxes();

    numericalSeries->setName("Численное решение");
    analyticalSeries->setName("Аналитическое решение");

    chart->legend()->show();
}
