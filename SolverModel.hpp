#pragma once

#include <vector>

class SolverModel {
public:
    struct Params {
        double mu1;     // Граничное условие на левом краю
        double mu2;     // Граничное условие на правом краю
        double xi;      // Точка разрыва
        int n;          // Количество разбиений
    };

    struct Result {
        std::vector<double> x;         // Узлы сетки
        std::vector<double> u;         // Численное решение
        std::vector<double> analytical; // Аналитическое решение
        double maxError;               // Максимальная ошибка
    };


    SolverModel();
    void setParams(const Params& params);
    Result solve();

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
    double calculateGridError(const Result& coarse, const Result& fine);
};

