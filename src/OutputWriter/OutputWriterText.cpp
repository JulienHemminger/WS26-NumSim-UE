#include "OutputWriter/OutputWriterText.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "StaggeredGrid/Discretization.h"

void OutputWriterText::writeFile(double currentTime) {
    std::stringstream fileName;
    fileName << outputDirectory_ << "/output_" << std::setw(4) << std::setfill('0') << fileNo_
             << ".txt";

    fileNo_++;

    std::ofstream file(fileName.str().c_str());

    if (!file.is_open()) {
        std::cout << "Could not write to file \"" << fileName.str() << "\".";
        return;
    }

    file << "t: " << currentTime << std::endl;

    file << "nCells: " << discretization_->nCells()[0] << "x" << discretization_->nCells()[1]
         << ", dx: " << discretization_->dx() << ", dy: " << discretization_->dy() << std::endl
         << std::endl;

    const int fieldWidth = 9;

    file << "u (" << discretization_->u().size()[0] << "x" << discretization_->u().size()[1]
         << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->u().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->uJEnd() - 1; j >= discretization_->uJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            if (discretization_->isObstacleCell(i, j) == 1.) {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6) << NAN;
            } else {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                     << discretization_->u(i, j);
            }
        }
        file << std::endl;
    }
    file << std::endl;

    file << "v (" << discretization_->v().size()[0] << "x" << discretization_->v().size()[1]
         << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->v().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->vJEnd() - 1; j >= discretization_->vJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            if (discretization_->isObstacleCell(i, j) == 1.) {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6) << NAN;
            } else {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                     << discretization_->v(i, j);
            }
        }
        file << std::endl;
    }
    file << std::endl;

    file << "p (" << discretization_->p().size()[0] << "x" << discretization_->p().size()[1]
         << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->p().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            if (discretization_->isObstacleCell(i, j) == 1.) {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6) << NAN;
            } else {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                     << discretization_->p(i, j);
            }
        }
        file << std::endl;
    }
    file << std::endl;

    file << "F (" << discretization_->u().size()[0] << "x" << discretization_->u().size()[1]
         << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->u().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->uJEnd() - 1; j >= discretization_->uJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            if (discretization_->isObstacleCell(i, j) == 1.) {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6) << NAN;
            } else {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                     << discretization_->f(i, j);
            }
        }
        file << std::endl;
    }
    file << std::endl;

    file << "G (" << discretization_->v().size()[0] << "x" << discretization_->v().size()[1]
         << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->v().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->vJEnd() - 1; j >= discretization_->vJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            if (discretization_->isObstacleCell(i, j) == 1.) {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6) << NAN;
            } else {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                     << discretization_->g(i, j);
            }
        }
        file << std::endl;
    }
    file << std::endl;

    file << "rhs (" << discretization_->p().size()[0] << "x" << discretization_->p().size()[1]
         << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->p().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            if (discretization_->isObstacleCell(i, j) == 1.) {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6) << NAN;
            } else {
                file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                     << discretization_->rhs(i, j);
            }
        }
        file << std::endl;
    }
    file << std::endl;

    file << "isObstacleCell (" << discretization_->p().size()[0] << "x"
         << discretization_->p().size()[1] << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->p().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                 << discretization_->isObstacleCell(i, j) * 10;
        }
        file << std::endl;
    }
    file << std::endl;

    file << "hasFluidNeighbourLeft (" << discretization_->p().size()[0] << "x"
         << discretization_->p().size()[1] << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->p().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                 << discretization_->hasFluidNeighbourLeft(i, j) * 10;
        }
        file << std::endl;
    }
    file << std::endl;

    file << "hasFluidNeighbourRight (" << discretization_->p().size()[0] << "x"
         << discretization_->p().size()[1] << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->p().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                 << discretization_->hasFluidNeighbourRight(i, j) * 10;
        }
        file << std::endl;
    }
    file << std::endl;

    file << "hasFluidNeighbourTop (" << discretization_->p().size()[0] << "x"
         << discretization_->p().size()[1] << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->p().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                 << discretization_->hasFluidNeighbourTop(i, j) * 10;
        }
        file << std::endl;
    }
    file << std::endl;

    file << "hasFluidNeighbourBottom (" << discretization_->p().size()[0] << "x"
         << discretization_->p().size()[1] << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->p().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                 << discretization_->hasFluidNeighbourBottom(i, j) * 10;
        }
        file << std::endl;
    }
    file << std::endl;
}

void OutputWriterText::writePressureFile() {
    static int pressurefileNo = 0;

    std::stringstream fileName;
    fileName << outputDirectory_ << "/pressure_" << std::setw(4) << std::setfill('0')
             << pressurefileNo++ << ".txt";

    std::ofstream file(fileName.str().c_str());

    if (!file.is_open()) {
        std::cout << "Could not write to file \"" << fileName.str() << "\".";
        return;
    }

    file << "nCells: " << discretization_->nCells()[0] << "x" << discretization_->nCells()[1]
         << ", dx: " << discretization_->dx() << ", dy: " << discretization_->dy() << std::endl
         << std::endl;

    const int fieldWidth = 9;

    file << "p (" << discretization_->p().size()[0] << "x" << discretization_->p().size()[1]
         << "): " << std::endl
         << std::string(fieldWidth, ' ') << "|";
    for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
        file << std::setw(fieldWidth) << i;
    }
    file << std::endl
         << std::string(fieldWidth * (discretization_->p().size()[0] + 2) + 1, '-') << std::endl;

    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        file << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            file << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                 << discretization_->p(i, j);
        }
        file << std::endl;
    }
    file << std::endl;
}
