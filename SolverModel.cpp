#include "SolverModel.hpp"
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <QDebug>

SolverModel::SolverModel() {
    // Установка параметров по умолчанию
    m_params = {0.0, 0.0, 0.5, 10, 1e-6}; // Добавлен epsilon
}

void SolverModel::setParams(const Params& params) {
    if (params.n < 2) {
        throw std::invalid_argument("Number of divisions must be at least 2");
    }
    m_params = params;
}

SolverModel::Result SolverModel::solve() {
    // Генерация узлов сетки
    double h = 1.0 / m_params.n;
    std::vector<double> x(m_params.n + 1);
    for (int i = 0; i <= m_params.n; ++i) {
        x[i] = i * h;
    }

    // Создание массивов коэффициентов для метода прогонки
    std::vector<double> a(m_params.n + 1, 0.0);
    std::vector<double> b(m_params.n + 1, 0.0);
    std::vector<double> c(m_params.n + 1, 0.0);
    std::vector<double> d(m_params.n + 1, 0.0);

    for (int i = 1; i < m_params.n; ++i) {
        double xi = x[i];
        auto coeffs = computeCoefficients(xi);
        double k = coeffs[0];
        double q = coeffs[1];
        double f = coeffs[2];

        a[i] = k / (h * h);                       // Нижняя диагональ
        b[i] = -2.0 * k / (h * h) - q;           // Центральная диагональ
        c[i] = k / (h * h);                      // Верхняя диагональ
        d[i] = -f;                               // Правая часть
    }

    // Учет граничных условий
    b[0] = b[m_params.n] = 1.0;
    d[0] = m_params.mu1;
    d[m_params.n] = m_params.mu2;

    // Решение методом прогонки
    std::vector<double> u = thomasAlgorithm(a, b, c, d);

    // Вычисление аналитического решения
    std::vector<double> analytical(m_params.n + 1);
    for (int i = 0; i <= m_params.n; ++i) {
        analytical[i] = analyticalSolution(x[i]);
    }

    // Вычисление максимальной ошибки
    double maxError = calculateError(u, analytical);

    return {x, u, analytical, maxError};
}

SolverModel::Result SolverModel::solveWithAccuracy(double targetError) {
    SolverModel::Params originalParams = m_params;
    SolverModel::Result result;

    double previousError = std::numeric_limits<double>::max(); // Предыдущая ошибка
    double relativeImprovement = 0.0; // Относительное улучшение ошибки

    do {
        result = solve();

        // Отладочная информация
        qDebug() << "n =" << m_params.n << ", maxError =" << result.maxError;

        // Проверяем достижение целевой точности
        if (result.maxError <= targetError) {
            break;
        }

        // Вычисляем относительное улучшение ошибки
        relativeImprovement = std::abs(previousError - result.maxError) / previousError;

        // Завершаем цикл, если ошибка перестала уменьшаться
        if (relativeImprovement < 1e-6) { // Порог сходимости (например, 1e-6)
            qDebug() << "Convergence reached: relative improvement =" << relativeImprovement;
            break;
        }

        previousError = result.maxError;

        // Проверка предельного размера сетки
        if (m_params.n >= 1e6) {
            qDebug() << "Grid size too large. Cannot refine further.";
            break;
        }

        // Удвоение количества разбиений сетки
        m_params.n *= 2;

    } while (true);

    m_params = originalParams; // Возврат к исходным параметрам
    return result;
}


std::vector<double> SolverModel::computeCoefficients(double x) {
    // Определение коэффициентов k(x), q(x), f(x)
    if (x < m_params.xi) {
        return {pow(x + 1, 2), std::exp(x), std::cos(M_PI * x)}; // k1, q1, f1
    } else {
        return {pow(x, 2), std::exp(x), 1.0};                   // k2, q2, f2
    }
}

std::vector<double> SolverModel::thomasAlgorithm(const std::vector<double>& a,
                                                 const std::vector<double>& b,
                                                 const std::vector<double>& c,
                                                 const std::vector<double>& d) {
    int n = b.size();
    std::vector<double> p(n), q(n), u(n);

    // Прямой ход
    p[0] = -c[0] / b[0];
    q[0] = d[0] / b[0];
    for (int i = 1; i < n; ++i) {
        double denom = b[i] + a[i] * p[i - 1];
        p[i] = -c[i] / denom;
        q[i] = (d[i] - a[i] * q[i - 1]) / denom;
    }

    // Обратный ход
    u[n - 1] = q[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        u[i] = p[i] * u[i + 1] + q[i];
    }

    return u;
}

double SolverModel::analyticalSolution(double x) {
    // Пример аналитического решения
    if (x < m_params.xi) {
        return x * (1 - x); // Для демонстрации
    } else {
        return x * (1 - x) * std::exp(x); // Для демонстрации
    }
}

double SolverModel::calculateError(const std::vector<double>& numerical,
                                   const std::vector<double>& analytical) {
    double maxError = 0.0;
    for (size_t i = 0; i < numerical.size(); ++i) {
        maxError = std::max(maxError, std::abs(numerical[i] - analytical[i]));
    }
    return maxError;
}

double SolverModel::calculateGridError(const Result& coarse, const Result& fine) {
    double maxError = 0.0;
    for (size_t i = 0; i < coarse.x.size(); ++i) {
        maxError = std::max(maxError, std::abs(coarse.u[i] - fine.u[2 * i]));
    }
    return maxError;
}
