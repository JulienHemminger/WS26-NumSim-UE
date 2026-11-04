#include "typedef.hpp"
#include "parameter.hpp"

/// constructor
Parameter::Parameter(){
    Parameter::Load("default.param");
}

/// Load params
void Parameter::Load(const char *file){
    std::ifstream cFile;
    cFile.open(file);
    if (cFile.is_open()){
        std::string line;
        while(std::getline(cFile,line)){
            auto delimiterPos = line.find("=");
            std::string name = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            if(cFile.eof()) break;
            // set params in workspace
            Parameter::setParam(name,value);
            std::cout << name << " " << value << " end \n";
        }
    }
    else {
        std::cerr << "Couldn't open default parameter file for reading.\n";
    }
    cFile.close();
}

/// set params from file in workspace as numbers, atof converts string to double
void Parameter::setParam(std::string name, std::string value){
    if (name == "re") {
        _re = std::atof(value.c_str());
    } 
    else if (name == "omg"){
        _omega = std::atof(value.c_str());
    }
    else if (name == "alpha"){
         _alpha = std::atof(value.c_str());
    }
    else if (name == "dt"){
        _dt = std::atof(value.c_str());
    }
    else if (name == "tend"){
         _tend = std::atof(value.c_str());
    }
    else if (name == "itermax"){
        _itermax = std::atof(value.c_str());
    }
    else if (name == "eps"){
        _eps = std::atof(value.c_str());
    }
    else if (name == "tau"){
        _tau = std::atof(value.c_str());
    }
}

/// Getter functions
const real_t &Parameter::Re() const{
    return _re;
}
const real_t &Parameter::Omega() const{
    return _omega;
}
const real_t &Parameter::Alpha() const{
    return _alpha;
}
const real_t &Parameter::Dt() const{
    return _dt;
}
const real_t &Parameter::Tend() const{
    return _tend;
}
const index_t &Parameter::IterMax() const{
    return _itermax;
}
const real_t &Parameter::Eps() const{
    return _eps;
}
const real_t &Parameter::Tau() const{
    return _tau;
}