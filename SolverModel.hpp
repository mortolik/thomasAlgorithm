#pragma once

#include <vector>

class SolverModel {
public:
    struct Params {
        double mu1;     // Граничное условие на левом краю
        double mu2;     // Граничное условие на правом краю
        double xi;      // Точка разрыва
        int n;          // Количество разбиений
        double epsilon;  // Заданная точность
    };

    struct Result {
        std::vector<double> x;         // Узлы сетки
        std::vector<double> u;         // Численное решение
        std::vector<double> analytical; // Аналитическое решение
        double maxError;               // Максимальная ошибка
        std::vector<double> uRefined;
        double maxErrorRefined;
    };

    SolverModel();
    void setParams(const Params& params);
    Result solve();
    Result solveWithAccuracy(double targetError);
    double calculateGridError(const Result& coarse, const Result& fine);

private:
    Params m_params;
    std::vector<double> computeCoefficients(double x);
    std::vector<double> thomasAlgorithm(const std::vector<double>& a,
                                        const std::vector<double>& b,
                                        const std::vector<double>& c,
                                        const std::vector<double>& d);

    double analyticalSolution(double x); // Аналитическое решение
    double calculateError(const std::vector<double>& numerical,
                          const std::vector<double>& analytical);
};

