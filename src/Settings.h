#pragma once

#include <array>
#include <iostream>

/** All settings that parametrize a simulation run.
 */
struct Settings {
    std::array<int, 2> nCells;
    std::array<double, 2> physicalSize;
    std::string complexGeometryPath;

    double re = 1000;
    double endTime = 10.0;
    double tau = 0.5;
    double maximumDt = 0.1;

    std::array<double, 2> g{0., 0.};

    bool useDonorCell = false;
    double alpha = 0.5;

    double underrelaxationVelocity = 0;

    std::array<double, 2> dirichletBcBottom;
    std::array<double, 2> dirichletBcTop;
    std::array<double, 2> dirichletBcLeft;
    std::array<double, 2> dirichletBcRight;

    bool outflowBottom;
    bool outflowTop;
    bool outflowLeft;
    bool outflowRight;

    std::string pressureSolver = "SOR";
    double omega = 1.0;
    double epsilon = 1e-5;
    int maximumNumberOfIterations = 1e5;

    std::string uID;

    //! parse a text file with settings, each line contains "<parameterName> = <value>"
    void loadFromFile(std::string filename);

    //! output all settings to console
    void printSettings();
};
