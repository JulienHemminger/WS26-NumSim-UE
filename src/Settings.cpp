#include "Settings.h"

#include <fstream>
#include <iostream>

void Settings::printSettings() {
    std::cout << "Settings: " << std::endl
              << "  physicalSize: " << physicalSize[0] << " x " << physicalSize[1]
              << ", nCells: " << nCells[0] << " x " << nCells[1] << std::endl
              << " path to file with geometry layout: " << complexGeometryPath << std::endl
              << "  endTime: " << endTime << " s, re: " << re << ", g: (" << g[0] << "," << g[1]
              << "), tau: " << tau << ", maximum dt: " << maximumDt << std::endl
              << "  dirichletBC: bottom: (" << dirichletBcBottom[0] << "," << dirichletBcBottom[1]
              << ")"
              << ", top: (" << dirichletBcTop[0] << "," << dirichletBcTop[1] << ")"
              << ", left: (" << dirichletBcLeft[0] << "," << dirichletBcLeft[1] << ")"
              << ", right: (" << dirichletBcRight[0] << "," << dirichletBcRight[1] << ")"
              << std::endl
              << "  outflow: bottom: " << outflowBottom << ", top: " << outflowTop
              << ", left: " << outflowLeft << ", right: " << outflowRight << std::endl
              << "  useDonorCell: " << std::boolalpha << useDonorCell << ", alpha: " << alpha
              << std::endl
              << " underrelaxation constant: " << underrelaxationVelocity << std::endl
              << "  pressureSolver: " << pressureSolver << ", omega: " << omega
              << ", epsilon: " << epsilon
              << ", maximumNumberOfIterations: " << maximumNumberOfIterations << std::endl
              << " unique folder name: " << uID << std::endl;
}

void Settings::loadFromFile(std::string filename) {
    complexGeometryPath = "None";
    underrelaxationVelocity = 1.;
    uID = "";

    std::ifstream file(filename.c_str(), std::ios::in);

    if (!file.is_open()) {
        std::cout << "Could not open parameter file \"" << filename << "\"." << std::endl;
        return;
    }

    for (int lineNo = 0;; lineNo++) {
        std::string line;
        getline(file, line);

        if (file.eof()) break;

        int first_character_idx = line.find_first_not_of(" \t\n\r");

        if (line[0] == '#') {
            continue;
        } else if (!(first_character_idx != std::string::npos)) {
            continue;
        }

        int eq_idx = line.find('=');
        if (!(eq_idx != std::string::npos)) {
            std::cout << "There is a badly defined line\"" << lineNo << " of " << filename << "\"."
                      << std::endl;
            std::cout << line << std::endl;
            return;
        }

        int last_character_idx = line.find_first_of(" \t", first_character_idx);

        int end_idx = 0;
        if (last_character_idx >= eq_idx) {
            end_idx = eq_idx - first_character_idx;
        } else {
            end_idx = last_character_idx - first_character_idx;
        }
        std::string parameterName = line.substr(first_character_idx, end_idx);

        int first_value_idx = line.find_first_not_of(" \t", eq_idx + 1);
        int last_value_idx = line.find_first_of(" \t\n\r#", first_value_idx);
        std::string valueString = line.substr(first_value_idx, (last_value_idx - first_value_idx));

        if (parameterName == "nCellsX") {
            nCells[0] = int(atof(valueString.c_str()));
        } else if (parameterName == "nCellsY") {
            nCells[1] = int(atof(valueString.c_str()));
        } else if (parameterName == "physicalSizeX") {
            physicalSize[0] = atof(valueString.c_str());
        } else if (parameterName == "physicalSizeY") {
            physicalSize[1] = atof(valueString.c_str());
        } else if (parameterName == "complexGeometryPath") {
            complexGeometryPath = valueString;
        } else if (parameterName == "re") {
            re = atof(valueString.c_str());
        } else if (parameterName == "endTime") {
            endTime = atof(valueString.c_str());
        } else if (parameterName == "tau") {
            tau = atof(valueString.c_str());
        } else if (parameterName == "maximumDt") {
            maximumDt = atof(valueString.c_str());
        } else if (parameterName == "gX") {
            g[0] = atof(valueString.c_str());
        } else if (parameterName == "gY") {
            g[1] = atof(valueString.c_str());
        } else if (parameterName == "useDonorCell") {
            if (valueString == "true") {
                useDonorCell = true;
            } else if (valueString == "false") {
                useDonorCell = false;
            } else {
                std::cout << "The donor cell parameter is not understood: " << parameterName
                          << std::endl;
                return;
            }
        } else if (parameterName == "underrelaxationVelocity") {
            underrelaxationVelocity = atof(valueString.c_str());
        } else if (parameterName == "alpha") {
            alpha = atof(valueString.c_str());
        } else if (parameterName == "dirichletBottomX") {
            dirichletBcBottom[0] = atof(valueString.c_str());
        } else if (parameterName == "dirichletBottomY") {
            dirichletBcBottom[1] = atof(valueString.c_str());
        } else if (parameterName == "dirichletTopX") {
            dirichletBcTop[0] = atof(valueString.c_str());
        } else if (parameterName == "dirichletTopY") {
            dirichletBcTop[1] = atof(valueString.c_str());
        } else if (parameterName == "dirichletLeftX") {
            dirichletBcLeft[0] = atof(valueString.c_str());
        } else if (parameterName == "dirichletLeftY") {
            dirichletBcLeft[1] = atof(valueString.c_str());
        } else if (parameterName == "dirichletRightX") {
            dirichletBcRight[0] = atof(valueString.c_str());
        } else if (parameterName == "dirichletRightY") {
            dirichletBcRight[1] = atof(valueString.c_str());
        } else if (parameterName == "outflowBottom") {
            outflowBottom = (valueString == "true");
        } else if (parameterName == "outflowTop") {
            outflowTop = (valueString == "true");
        } else if (parameterName == "outflowLeft") {
            outflowLeft = (valueString == "true");
        } else if (parameterName == "outflowRight") {
            outflowRight = (valueString == "true");
        } else if (parameterName == "pressureSolver") {
            pressureSolver = valueString;
        } else if (parameterName == "omega") {
            omega = atof(valueString.c_str());
        } else if (parameterName == "epsilon") {
            epsilon = atof(valueString.c_str());
        } else if (parameterName == "maximumNumberOfIterations") {
            maximumNumberOfIterations = int(atof(valueString.c_str()));
        } else if (parameterName == "uID") {
            uID = valueString;
        } else {
            std::cout << "The parameter " << parameterName << " in this line " << lineNo
                      << " could not be found in " << filename << "\"." << std::endl;
            std::cout << line << std::endl;
            return;
        }
    }
    std::cout << "Reading is complete." << std::endl;
}
