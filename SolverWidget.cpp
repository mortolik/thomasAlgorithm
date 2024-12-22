#include "SolverWidget.hpp"
#include <QVBoxLayout>

SolverWidget::SolverWidget(QWidget* parent)
    : QWidget(parent), m_model(nullptr) {
    setupUI();
}

void SolverWidget::setModel(SolverModel* model) {
    m_model = model;
    m_testTaskWidget->setModel(model);
    m_mainTaskWidget->setModel(model);
}

void SolverWidget::setupUI() {
    m_tabWidget = new QTabWidget(this);

    m_testTaskWidget = new TestTaskWidget(this);
    m_mainTaskWidget = new MainTaskWidget(this);

    m_tabWidget->addTab(m_testTaskWidget, "Test Task");
    m_tabWidget->addTab(m_mainTaskWidget, "Main Task");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_tabWidget);

    setLayout(mainLayout);
}
