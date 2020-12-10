//
// Created by Raffaele Montella on 12/5/20.
//

#include "Particles.hpp"

Particles::Particles() {

    // Logger configuration
    log4cplus::BasicConfigurator basicConfig;
    basicConfig.configure();
    logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("WaComM"));

    LOG4CPLUS_INFO(logger, "Empty Particles");
}

Particles::Particles(string fileName) {

    log4cplus::BasicConfigurator basicConfig;
    basicConfig.configure();
    logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("WaComM"));

    LOG4CPLUS_INFO(logger, "Reading restart file:"+fileName);

    std::ifstream infile(fileName);

    int count=0;
    std::string line;
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        if (count > 0)
        {
            double k, j, i, health, tpart;
            if (!(iss >> i >> j >> k >> health >> tpart )) { break; } // error

            if (k<0) k=0;
            if (j<0) j=0;
            if (i<0) i=0;
            Particle particle(k, j, i, health, tpart);
            push_back(particle);
        }
        count++;
    }

}

void Particles::save(string fileName)
{

}

Particles::~Particles() {
}
