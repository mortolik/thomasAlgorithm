#ifndef TESTTASKWIDGET_HPP
#define TESTTASKWIDGET_HPP

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QTableWidget>
#include <QtCharts/QChartView>
#include "SolverModel.hpp"

class TestTaskWidget : public QWidget {
    Q_OBJECT

public:
    explicit TestTaskWidget(QWidget* parent = nullptr);
    void setModel(SolverModel* model);

private slots:
    void onSolveButtonClicked();

private:
    void setupUI();
    void displayResults(const SolverModel::Result& result);

    SolverModel* m_model;

    QSpinBox* m_spinBoxN;
    QDoubleSpinBox* m_spinBoxEpsilon;
    QTextEdit* m_infoText;
    QTableWidget* m_resultsTable;
    QtCharts::QChartView* m_plot;
    QPushButton* m_solveButton;
};

#endif // TESTTASKWIDGET_HPP
