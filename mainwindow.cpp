#include "MainWindow.h"
#include "SolverModel.hpp"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_model(new SolverModel),
    m_widget(new SolverWidget(this)) {

    // Установка модели для виджета
    m_widget->setModel(m_model);

    // Установка виджета как центрального
    setCentralWidget(m_widget);

    // Настройка окна
    setWindowTitle("Solver Application");
    resize(800, 600); // Установка размера окна
}

MainWindow::~MainWindow() {
    delete m_model;
}
