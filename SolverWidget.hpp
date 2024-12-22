#pragma once

#include <QWidget>
#include <QTabWidget>
#include "SolverModel.hpp"
#include "TestTaskWidget.hpp"
#include "MainTaskWidget.hpp"

class SolverWidget : public QWidget {
    Q_OBJECT

public:
    explicit SolverWidget(QWidget* parent = nullptr);
    void setModel(SolverModel* model);

private:
    void setupUI();

    SolverModel* m_model;

    QTabWidget* m_tabWidget;
    TestTaskWidget* m_testTaskWidget;
    MainTaskWidget* m_mainTaskWidget;

};

