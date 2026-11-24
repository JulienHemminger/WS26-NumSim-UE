#include "OutputWriter/OutputWriter.h"

#include <iostream>

#include "Partitioning/Partitioning.h"

OutputWriter::OutputWriter(std::shared_ptr<Discretization> discretization)
    : discretization_(discretization), fileNo_(0), partitioning_(nullptr) {
    // create "out" subdirectory if it does not yet exist
    int returnValue = system("mkdir -p out");
    if (returnValue != 0) std::cout << "Could not create subdirectory \"out\"." << std::endl;
}

OutputWriter::OutputWriter(std::shared_ptr<Discretization> discretization,
                           std::shared_ptr<Partitioning> partitioning)
    : discretization_(discretization),
      fileNo_(0),
      partitioning_(partitioning)

{
    int returnValue = system("mkdir -p out");
    if (returnValue != 0) std::cout << "Could not create subdirectory \"out\"." << std::endl;
}
