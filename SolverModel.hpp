#pragma once

#include <vector>

class SolverModel {
public:
    struct Params {
        double mu1;
        double mu2;
        double xi;
        int n;
        double epsilon;
    };

    struct ConvergenceData {
        int n;
        double error;
    };

    struct Result {
        std::vector<double> x;
        std::vector<double> u;
        std::vector<double> analytical;
        double maxError;

        // Для основной задачи
        std::vector<double> xRefined;
        std::vector<double> uRefined;
        std::vector<double> analyticalRefined;
        double maxErrorRefined;

        // Данные для графика сходимости
        std::vector<ConvergenceData> convergenceData;
    };

    SolverModel();
    void setParams(const Params& params);
    Result solve();
    Result solveWithAccuracy(double targetError);

    double analyticalSolution(double x);
    double calculateError(const std::vector<double>& numerical, const std::vector<double>& analytical);
    double calculateGridError(const Result& coarse, const Result& fine);

private:
    Params m_params;

    std::vector<double> computeCoefficients(double x);
    std::vector<double> thomasAlgorithm(const std::vector<double>& a,
                                        const std::vector<double>& b,
                                        const std::vector<double>& c,
                                        const std::vector<double>& d);
};

