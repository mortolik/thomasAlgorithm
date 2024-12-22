#pragma once

#include <QMainWindow>
#include "SolverModel.hpp"
#include "SolverWidget.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    SolverModel* m_model;      // Модель для выполнения расчётов
    SolverWidget* m_widget;    // Виджет для отображения данных
};

