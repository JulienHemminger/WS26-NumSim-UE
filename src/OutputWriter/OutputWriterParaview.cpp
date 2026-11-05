#include "OutputWriter/OutputWriterParaview.h"

#include <cstdlib>
#include <iostream>

OutputWriterParaview::OutputWriterParaview(std::shared_ptr<Discretization> discretization,
                                           std::string uID)
    : OutputWriter(discretization, uID) {
    vtkWriter_ = vtkSmartPointer<vtkXMLImageDataWriter>::New();
}

void OutputWriterParaview::writeFile(double currentTime) {
    std::stringstream fileName;
    fileName << outputDirectory_ << "/output_" << std::setw(4) << setfill('0') << fileNo_ << "."
             << vtkWriter_->GetDefaultFileExtension();

    fileNo_++;

    vtkWriter_->SetFileName(fileName.str().c_str());

    vtkSmartPointer<vtkImageData> dataSet = vtkSmartPointer<vtkImageData>::New();
    dataSet->SetOrigin(0, 0, 0);

    const double dx = discretization_->meshWidth()[0];
    const double dy = discretization_->meshWidth()[1];
    const double dz = 1;
    dataSet->SetSpacing(dx, dy, dz);

    std::array<int, 2> nCells = discretization_->nCells();
    dataSet->SetDimensions(nCells[0] + 1, nCells[1] + 1, 1);

    vtkSmartPointer<vtkDoubleArray> arrayPressure = vtkDoubleArray::New();

    arrayPressure->SetNumberOfComponents(1);

    arrayPressure->SetNumberOfTuples(dataSet->GetNumberOfPoints());

    arrayPressure->SetName("pressure");

    int index = 0;
    for (int j = 0; j < nCells[1] + 1; j++) {
        for (int i = 0; i < nCells[0] + 1; i++, index++) {
            if (discretization_->isObstacleCell(i, j) == 1.) {
                arrayPressure->SetValue(index, NAN);
            } else {
                const double x = i * dx;
                const double y = j * dy;

                arrayPressure->SetValue(index, discretization_->p().interpolateAt(x, y));
            }
        }
    }

    assert(index == dataSet->GetNumberOfPoints());

    dataSet->GetPointData()->AddArray(arrayPressure);

    vtkSmartPointer<vtkDoubleArray> arrayVelocity = vtkDoubleArray::New();

    arrayVelocity->SetNumberOfComponents(3);

    arrayVelocity->SetNumberOfTuples(dataSet->GetNumberOfPoints());

    arrayVelocity->SetName("velocity");

    index = 0;
    for (int j = 0; j < nCells[1] + 1; j++) {
        const double y = j * dy;

        for (int i = 0; i < nCells[0] + 1; i++, index++) {
            std::array<double, 3> velocityVector;
            if (discretization_->isObstacleCell(i, j) == 1.) {
                velocityVector[0] = NAN;
                velocityVector[1] = NAN;
                velocityVector[2] = 0.0;
            } else {
                const double x = i * dx;

                velocityVector[0] = discretization_->u().interpolateAt(x, y);
                velocityVector[1] = discretization_->v().interpolateAt(x, y);
                velocityVector[2] = 0.0;
            }

            arrayVelocity->SetTuple(index, velocityVector.data());
        }
    }

    assert(index == dataSet->GetNumberOfPoints());

    dataSet->GetPointData()->AddArray(arrayVelocity);

    vtkSmartPointer<vtkDoubleArray> arrayTime = vtkDoubleArray::New();
    arrayTime->SetName("TIME");
    arrayTime->SetNumberOfTuples(1);
    arrayTime->SetTuple1(0, currentTime);
    dataSet->GetFieldData()->AddArray(arrayTime);

    dataSet->Squeeze();

    vtkWriter_->SetInputData(dataSet);

    vtkWriter_->SetDataModeToBinary();

    vtkWriter_->Write();
}

void writeParaviewOutput(int fileNo, std::string uID) {
    const std::string outputDirectory = uID.empty() ? "out" : "out_" + uID;

    std::string command = "mkdir -p ";
    std::string folder_name = outputDirectory;
    int returnValue = system((command + folder_name).c_str());
    if (returnValue != 0)
        std::cout << "Could not create subdirectory \"" << outputDirectory << "\"." << std::endl;

    vtkSmartPointer<vtkXMLImageDataWriter> vtkWriter =
        vtkSmartPointer<vtkXMLImageDataWriter>::New();

    std::stringstream fileName;
    fileName << outputDirectory << "/output_" << std::setw(4) << setfill('0') << fileNo << "."
             << vtkWriter->GetDefaultFileExtension();

    std::cout << "Write file \"" << fileName.str() << "\"." << std::endl;

    vtkWriter->SetFileName(fileName.str().c_str());

    vtkSmartPointer<vtkImageData> dataSet = vtkSmartPointer<vtkImageData>::New();
    dataSet->SetOrigin(0, 0, 0);

    const double dx = 1;
    const double dy = 1;
    const double dz = 1;
    dataSet->SetSpacing(dx, dy, dz);

    dataSet->SetDimensions(10, 10, 1);

    vtkSmartPointer<vtkDoubleArray> arrayPressure = vtkDoubleArray::New();

    arrayPressure->SetNumberOfComponents(1);

    arrayPressure->SetNumberOfTuples(dataSet->GetNumberOfPoints());

    arrayPressure->SetName("pressure");

    for (int j = 0; j < 10; j++) {
        for (int i = 0; i < 10; i++) {
            int index = j * 10 + i;
            arrayPressure->SetValue(index, i + j - fileNo * (i - j));
        }
    }

    dataSet->GetPointData()->AddArray(arrayPressure);

    dataSet->Squeeze();

    vtkWriter->SetInputData(dataSet);

    vtkWriter->SetDataModeToBinary();

    vtkWriter->Write();
}