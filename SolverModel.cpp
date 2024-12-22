#include "SolverModel.hpp"
#include <QDebug>
#include <cmath>

SolverModel::SolverModel() {
    // Установка параметров по умолчанию
    m_params = {0.0, 0.0, 0.5, 10, 1e-6};
}

void SolverModel::setParams(const Params& params) {
    if (params.n < 2) {
        throw std::invalid_argument("Количество разбиений должно быть не менее 2");
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
        c[i] = k / (h * h);                       // Верхняя диагональ
        d[i] = -f;                                // Правая часть
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

    Result result;
    result.x = x;
    result.u = u;
    result.analytical = analytical;
    result.maxError = maxError;

    return result;
}

SolverModel::Result SolverModel::solveWithAccuracy(double targetError) {
    Params originalParams = m_params;
    Result result;
    Result refinedResult;

    double previousError = std::numeric_limits<double>::max();
    double relativeImprovement = 0.0;

    const int maxIterations = 1000; // Максимальное количество итераций
    int iteration = 0;

    result.convergenceData.clear();

    while (iteration < maxIterations) {
        result = solve();

        // Сохранение данных для графика сходимости
        result.convergenceData.push_back({m_params.n, result.maxError});

        qDebug() << "Итерация" << iteration << ": n =" << m_params.n << ", maxError =" << result.maxError;

        // Проверяем достижение целевой точности
        if (result.maxError <= targetError) {
            qDebug() << "Целевая точность достигнута.";
            break;
        }

        // Вычисляем относительное улучшение ошибки
        relativeImprovement = std::abs(previousError - result.maxError) / previousError;

        // Завершаем цикл, если ошибка перестала уменьшаться
        if (relativeImprovement < 1e-6) {
            qDebug() << "Сходимость достигнута: относительное улучшение =" << relativeImprovement;
            break;
        }

        previousError = result.maxError;

        // Проверка предельного размера сетки
        if (m_params.n >= 1e6) {
            qDebug() << "Размер сетки слишком большой. Невозможно уточнить далее.";
            break;
        }

        // Удвоение количества разбиений сетки
        m_params.n *= 2;
        iteration++;
    }

    if (iteration >= maxIterations) {
        qDebug() << "Достигнуто максимальное количество итераций.";
    }

    // Решение на уточнённой сетке
    refinedResult = solve();
    result.uRefined = refinedResult.u;
    result.maxErrorRefined = refinedResult.maxError;

    m_params = originalParams; // Возврат к исходным параметрам
    return result;
}

std::vector<double> SolverModel::computeCoefficients(double x) {
    // Для данного примера используем постоянные коэффициенты
    return {1.0, 0.0, M_PI * M_PI * std::sin(M_PI * x)}; // k, q, f
}

std::vector<double> SolverModel::thomasAlgorithm(const std::vector<double>& a,
                                                 const std::vector<double>& b,
                                                 const std::vector<double>& c,
                                                 const std::vector<double>& d) {
    int n = b.size();
    std::vector<double> p(n, 0.0), q(n, 0.0), u(n, 0.0);

    // Прямой ход
    p[0] = -c[0] / b[0];
    q[0] = d[0] / b[0];
    for (int i = 1; i < n; ++i) {
        double denom = b[i] + a[i] * p[i - 1];
        if (fabs(denom) < 1e-12) { // Проверка на деление на ноль
            throw std::runtime_error("Нулевой знаменатель в методе прогонки");
        }
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
    return std::sin(M_PI * x); // Корректное аналитическое решение для данного примера
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
        if (2 * i < fine.u.size()) {
            maxError = std::max(maxError, std::abs(coarse.u[i] - fine.u[2 * i]));
        }
    }
    return maxError;
}
