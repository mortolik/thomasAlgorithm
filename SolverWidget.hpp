#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QtCharts/QChartView>
#include "SolverModel.hpp"

using namespace QtCharts;

class SolverWidget : public QWidget {
    Q_OBJECT

public:
    explicit SolverWidget(QWidget* parent = nullptr);
    void setModel(SolverModel* model);

private slots:
    void onSolveButtonClicked();
    void onHelpButtonClicked();

private:
    void setupUI();
    void displayResults(const SolverModel::Result& result, const SolverModel::Result& refinedResult);

    SolverModel* m_model;

    QSpinBox* m_spinBoxN;
    QDoubleSpinBox* m_spinBoxEpsilon;
    QComboBox* m_taskSelector;
    QTextEdit* m_infoText;
    QTableWidget* m_resultsTable;
    QTableWidget* m_refinedResultsTable; // Новая таблица для уточнённых результатов
    QtCharts::QChartView* m_plot;
    QtCharts::QChartView* m_errorPlot;
    QtCharts::QChartView* m_logErrorPlot; // Новый график для логарифмической зависимости
    QPushButton* m_solveButton;
    QPushButton* m_helpButton; // Кнопка справки
};

