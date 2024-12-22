#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QtCharts/QChartView>
#include "SolverModel.hpp"

using namespace QtCharts;

class SolverWidget : public QWidget {
    Q_OBJECT

public:
    explicit SolverWidget(QWidget* parent = nullptr);

    // Метод для подключения модели
    void setModel(SolverModel* model);

private slots:
    // Слот для обработки нажатия кнопки "Solve"
    void onSolveButtonClicked();

private:
    SolverModel* m_model; // Модель для выполнения расчётов

    QSpinBox* m_spinBoxN;       // Спинбокс для ввода количества разбиений
    QTextEdit* m_infoText;      // Поле для отображения текстовой информации
    QTableWidget* m_resultsTable; // Таблица для отображения результатов
    QChartView* m_plot;         // График на основе Qt Charts
    QPushButton* m_solveButton; // Кнопка для запуска расчётов

    // Метод для отображения результатов
    void displayResults(const SolverModel::Result& result);

    // Метод для настройки интерфейса
    void setupUI();
};

