//
// Created by Raffaele Montella on 14/12/20.
//

#include "WacommPlusPlus.hpp"

#ifdef USE_MPI
#include <mpi.h>
#endif

WacommPlusPlus::~WacommPlusPlus() = default;

WacommPlusPlus::WacommPlusPlus(std::shared_ptr<Config> config): config(config) {
    logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("WaComM"));

    if (config->UseRestart()) {
        particles = std::make_shared<Particles>(config->RestartFile());
    } else {
        particles = std::make_shared<Particles>();
    }
    sources =  std::make_shared<Sources>();
}



void WacommPlusPlus::run() {
    LOG4CPLUS_DEBUG(logger,"External loop...");

    int world_size=1, world_rank=0;

#ifdef USE_MPI
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
#endif

    int idx=0;
    for (auto & ncInput : config->NcInputs()) {
        //if (idx>=config->StartTimeIndex() && idx<config->NumberOfInputs()) {
        if (true) {
            LOG4CPLUS_INFO(logger, "Input from Ocean Model: " << ncInput);
            ROMSAdapter romsAdapter(ncInput);
            romsAdapter.process();
            shared_ptr<OceanModelAdapter> oceanModelAdapter;
            oceanModelAdapter = make_shared<OceanModelAdapter>(romsAdapter);

            // Check if it is needed to load the sources
            if ( config->UseSources() && sources->size()==0) {
                string fileName=config->SourcesFile();
                if (fileName.empty()) {
                    sources->loadFromNamelist(config->ConfigFile());
                } else {
                    if (fileName.substr(fileName.find_last_of(".") + 1) == "json") {
                        // The configuration is a json
                        sources->loadFromJson(fileName, oceanModelAdapter);
                    } else {
                        // the configuration is a fortran style namelist
                        sources->loadFromNamelist(fileName);
                    }
                }
            }

            if (world_rank==0) {
                string inputFilename = "input.nc";
                oceanModelAdapter->saveAsNetCDF(inputFilename);
            }
            Wacomm wacomm(config, oceanModelAdapter, sources, particles);
            wacomm.run();
        }
        idx++;
    }
}
