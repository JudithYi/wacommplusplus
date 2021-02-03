//
// Created by Raffaele Montella on 14/12/20.
//

#include "WacommPlusPlus.hpp"
#include "JulianDate.hpp"
#include "OceanModelAdapters/WacommAdapter.hpp"
#include "OceanModelAdapters/ROMSAdapter.hpp"

#ifdef USE_MPI
#define OMPI_SKIP_MPICXX
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
    LOG4CPLUS_DEBUG(logger, "External loop...");

    int world_size = 1, world_rank = 0;

#ifdef USE_MPI
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
#endif

    int idx = 0;
    for (auto &ncInput : config->NcInputs()) {

        if (world_rank == 0) {
            LOG4CPLUS_INFO(logger, world_rank << ": Input from Ocean Model: " << ncInput);
        }

        shared_ptr<OceanModelAdapter> oceanModelAdapter;
        if (config->OceanModel() == "ROMS") {
            oceanModelAdapter = make_shared<ROMSAdapter>(ncInput);

        } else {
            oceanModelAdapter = make_shared<WacommAdapter>(ncInput);
        }
        oceanModelAdapter->process();

        Calendar cal;

        // Time in "seconds since 1968-05-23 00:00:00"
        double modJulian=oceanModelAdapter->OceanTime()(0);

        // Convert time in days based
        modJulian=modJulian/86400;

        JulianDate::fromModJulian(modJulian, cal);

        // Check if it is needed to load the sources
        if (config->UseSources() && sources->empty()) {
            string fileName = config->SourcesFile();
            if (fileName.empty()) {
                sources->loadFromNamelist(config->ConfigFile());
            } else {
                if (fileName.substr(fileName.find_last_of('.') + 1) == "json") {
                    // The configuration is a json
                    sources->loadFromJson(fileName, oceanModelAdapter);
                } else {
                    // the configuration is a fortran style namelist
                    sources->loadFromNamelist(fileName);
                }
            }
        }

        // Check if the rank is 0
        if (world_rank == 0) {

            // Check if the processed input must be saved
            if (config->SaveInput()) {

                // Create the filename
                string inputFilename = config->NcInputRoot() + cal.asNCEPdate() + ".nc";

                // Show a information message
                LOG4CPLUS_INFO(logger,  "Saving processed output: " << inputFilename);

                // Save the processed input
                oceanModelAdapter->saveAsNetCDF(inputFilename);
            }
        }

        // Check if it is a dry run
        if (!config->Dry()) {
            // Create a new Wacomm object
            Wacomm wacomm(config, oceanModelAdapter, sources, particles);

            // Run the model
            wacomm.run();
        }
        // Go to the next input file
        idx++;
    }
}
