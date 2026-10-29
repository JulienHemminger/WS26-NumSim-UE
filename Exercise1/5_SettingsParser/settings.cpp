#include "settings.h"
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

void Settings::loadFromFile(std::string filename)
{
  std::ifstream file(filename);
  if (!file.is_open())
  {
    std::cerr << "Could not open settings file: " << filename << std::endl;
    return;
  }

  std::string line;
  while (std::getline(file, line))
  {
    auto commentPos = line.find('#');
    if (commentPos != std::string::npos)
    {
      line = line.substr(0, commentPos);
    }

    auto equalPos = line.find('=');
    if (equalPos == std::string::npos)
    {
      continue;
    }

    std::string parameterName = line.substr(0, equalPos);
    std::string value = line.substr(equalPos + 1);

    parameterName.erase(0, parameterName.find_first_not_of(" \t\r\n"));
    parameterName.erase(parameterName.find_last_not_of(" \t\r\n") + 1);
    value.erase(0, value.find_first_not_of(" \t\r\n"));
    value.erase(value.find_last_not_of(" \t\r\n") + 1);

    if (parameterName.empty())
    {
      continue;
    }

    if (parameterName == "physicalSizeX")
    {
      physicalSize[0] = std::stod(value);
    }
    else if (parameterName == "physicalSizeY")
    {
      physicalSize[1] = std::stod(value);
    }
    else if (parameterName == "endTime")
    {
      endTime = std::stod(value);
    }
    else if (parameterName == "re")
    {
      re = std::stod(value);
    }
    else if (parameterName == "gX")
    {
      g[0] = std::stod(value);
    }
    else if (parameterName == "gY")
    {
      g[1] = std::stod(value);
    }
    else if (parameterName == "dirichletBottomX")
    {
      dirichletBcBottom[0] = std::stod(value);
    }
    else if (parameterName == "dirichletBottomY")
    {
      dirichletBcBottom[1] = std::stod(value);
    }
    else if (parameterName == "dirichletTopX")
    {
      dirichletBcTop[0] = std::stod(value);
    }
    else if (parameterName == "dirichletTopY")
    {
      dirichletBcTop[1] = std::stod(value);
    }
    else if (parameterName == "dirichletLeftX")
    {
      dirichletBcLeft[0] = std::stod(value);
    }
    else if (parameterName == "dirichletLeftY")
    {
      dirichletBcLeft[1] = std::stod(value);
    }
    else if (parameterName == "dirichletRightX")
    {
      dirichletBcRight[0] = std::stod(value);
    }
    else if (parameterName == "dirichletRightY")
    {
      dirichletBcRight[1] = std::stod(value);
    }
    else if (parameterName == "nCellsX")
    {
      nCells[0] = std::stoi(value);
    }
    else if (parameterName == "nCellsY")
    {
      nCells[1] = std::stoi(value);
    }
    else if (parameterName == "useDonorCell")
    {
      useDonorCell = (value == "true" || value == "1");
    }
    else if (parameterName == "alpha")
    {
      alpha = std::stod(value);
    }
    else if (parameterName == "tau")
    {
      tau = std::stod(value);
    }
    else if (parameterName == "maximumDt")
    {
      maximumDt = std::stod(value);
    }
    else if (parameterName == "pressureSolver")
    {
      pressureSolver = value;
    }
    else if (parameterName == "omega")
    {
      omega = std::stod(value);
    }
    else if (parameterName == "epsilon")
    {
      epsilon = std::stod(value);
    }
    else if (parameterName == "maximumNumberOfIterations")
    {
      maximumNumberOfIterations = std::stoi(value);
    }
  }
}
void Settings::printSettings()
{
  std::cout << "Settings: " << std::endl
    << "  physicalSize: " << physicalSize[0] << " x " << physicalSize[1] << ", nCells: " << nCells[0] << " x " << nCells[1] << std::endl
    << "  endTime: " << endTime << " s, re: " << re << ", g: (" << g[0] << "," << g[1] << "), tau: " << tau << ", maximum dt: " << maximumDt << std::endl
    << "  dirichletBC: bottom: (" << dirichletBcBottom[0] << "," << dirichletBcBottom[1]  << ")"
    << ", top: ("  << dirichletBcTop[0] << "," << dirichletBcTop[1]  << ")"
    << ", left: ("  << dirichletBcLeft[0] << "," << dirichletBcLeft[1] << ")"
    << ", right: ("  << dirichletBcRight[0] << "," << dirichletBcRight[1] << ")" << std::endl
    << "  useDonorCell: " << std::boolalpha << useDonorCell << ", alpha: " << alpha << std::endl
    << "  pressureSolver: " << pressureSolver << ", omega: " << omega << ", epsilon: " << epsilon << ", maximumNumberOfIterations: " << maximumNumberOfIterations << std::endl;
}


/*
void loadFromFile(std::string filename)
{
    std::ifstream file(filename.c_str(), std::ios::in);

    if (!file.is_open())
    {
        std::cout << "Could not open parameter file \"" << filename << "\"." << std::endl;
        return;
    }

    for (int lineNo = 0;; lineNo++)
    {
        std::string line;
        getline(file, line);

        if (file.eof())
            break;

        size_t lineStart = line.find_first_not_of(" \t");
        if (lineStart == std::string::npos)
            continue;

        line.erase(0, lineStart);

        if (line.empty() || line[0] == '#')
            continue;

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos)
            continue;

        std::string parameterName = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);

        size_t paramEnd = parameterName.find_last_not_of(" \t");
        if (paramEnd != std::string::npos)
        {
            parameterName.erase(paramEnd + 1);
        }
        else
        {
            parameterName.clear();
        }

        if (parameterName.find_first_of(" \t") != std::string::npos)
        {
            parameterName.erase(parameterName.find_first_of(" \t"));
        }

        size_t valStart = value.find_first_not_of(" \t");
        if (valStart != std::string::npos)
        {
            value.erase(0, valStart);
        }
        else
        {
            value.clear();
        }

        size_t commentPos = value.find('#');
        if (commentPos != std::string::npos)
        {
            value.erase(commentPos);
        }

        size_t valEnd = value.find_last_not_of(" \t");
        if (valEnd != std::string::npos)
        {
            value.erase(valEnd + 1);
        }
        else
        {
            value.clear();
        }

        if (parameterName == "endTime")
        {
            double endTime = std::atof(value.c_str());
            std::cout << "endTime: " << endTime << std::endl;
        }
        else if (parameterName == "re")
        {
            int re = std::atoi(value.c_str());
            std::cout << "re: " << re << std::endl;
        }
    }
}
*/
