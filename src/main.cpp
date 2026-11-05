#include <cstdlib>
#include <iostream>

#include "Computation.h"
#include "Settings.h"

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cout << "usage: " << argv[0] << " <filename>" << std::endl;

        return EXIT_FAILURE;
    }
    std::cout << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "Parsing paramter file..." << std::endl;
    std::cout << std::endl;

    Computation computation = Computation();
    computation.initialize(argc, argv);

    std::cout << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "Starting simulation ..." << std::endl;
    std::cout << std::endl;

    computation.runSimulation();

    std::cout << "-------------------------------------------------" << std::endl;

    return EXIT_SUCCESS;
}
