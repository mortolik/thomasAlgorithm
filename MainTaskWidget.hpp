#ifndef MAINTASKWIDGET_HPP
#define MAINTASKWIDGET_HPP

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QTableWidget>
#include <QtCharts/QChartView>
#include <QTabWidget>
#include "SolverModel.hpp"

class MainTaskWidget : public QWidget {
    Q_OBJECT

public:
    explicit MainTaskWidget(QWidget* parent = nullptr);
    void setModel(SolverModel* model);

private slots:
    void onSolveButtonClicked();
    void onHelpButtonClicked();

private:
    void setupUI();
    void displayResults(const SolverModel::Result& result, const SolverModel::Result& refinedResult);

    SolverModel* m_model;

    // Элементы управления
    QSpinBox* m_spinBoxN;
    QDoubleSpinBox* m_spinBoxEpsilon;
    QTextEdit* m_infoText;
    QPushButton* m_solveButton;
    QPushButton* m_helpButton;

    // Вкладки для отображения результатов
    QTabWidget* m_resultsTabWidget;

    // Таблицы
    QTableWidget* m_resultsTable;
    QTableWidget* m_refinedResultsTable;

    // Графики
    QtCharts::QChartView* m_plot;
    QtCharts::QChartView* m_errorPlot;
    QtCharts::QChartView* m_logErrorPlot;
};

#endif // MAINTASKWIDGET_HPP
