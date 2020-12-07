//
// Created by Raffaele Montella on 12/5/20.
//

#include "Wacomm.hpp"

using namespace netCDF;

Wacomm::Wacomm() {
    log4cplus::BasicConfigurator config;
    config.configure();
    logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("WaComM"));
}

Wacomm::Wacomm(string fileName, Particles& particles) {
    log4cplus::BasicConfigurator config;
    config.configure();
    logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("WaComM"));

    //this->particles=particles;

    Array2<double> mask_u, mask_v;
    Array4<double> u, v, w, akt;
    load(fileName,mask_u, mask_v, u,v,w,akt);

    size_t ocean_time=u.Nx();
    size_t s_rho=u.Ny();
    size_t s_w=w.Ny();
    size_t eta_rho=mask_rho.Nx();
    size_t xi_rho=mask_rho.Ny();
    size_t eta_u=mask_u.Nx();
    size_t xi_u=mask_u.Ny();
    size_t eta_v=mask_v.Nx();
    size_t xi_v=mask_v.Ny();

    LOG4CPLUS_INFO(logger,"ocean_time:" + std::to_string(ocean_time));
    LOG4CPLUS_INFO(logger,"s_rho:" + std::to_string(s_rho));
    LOG4CPLUS_INFO(logger,"s_w:" + std::to_string(s_w));
    LOG4CPLUS_INFO(logger,"eta_rho:" + std::to_string(eta_rho) + " xi_rho:" + std::to_string(xi_rho));
    LOG4CPLUS_INFO(logger,"eta_u:" + std::to_string(eta_u) + " xi_u:" + std::to_string(xi_u));
    LOG4CPLUS_INFO(logger,"eta_v:" + std::to_string(eta_v) + " xi_v:" + std::to_string(xi_v));

    //ucomp.Allocate(ocean_time,s_rho,eta_rho,xi_rho);
    //vcomp.Allocate(ocean_time,s_rho,eta_rho,xi_rho);
    //wcomp.Allocate(ocean_time,s_w,eta_rho,xi_rho);
    //aktcomp.Allocate(ocean_time,s_w,eta_rho,xi_rho);

    //uv2rho(mask_u, mask_v, u,v);
    //wakt2rho(mask_u, mask_v, w, akt);
}

Wacomm::~Wacomm()
{
}

void Wacomm::load(string fileName,
                  Array2<double>& mask_u, Array2<double>& mask_v,
                  Array4<double>& u, Array4<double>& v, Array4<double>& w,
                  Array4<double>& akt)
{
    LOG4CPLUS_INFO(logger,"Loading:"+fileName);

    // Open the file for read access
    netCDF::NcFile dataFile(fileName, NcFile::read);

    // Retrieve the variable named "mask_rho"
    NcVar varMaskRho=dataFile.getVar("mask_rho");

    size_t eta_rho = varMaskRho.getDim(0).getSize();
    size_t xi_rho = varMaskRho.getDim(1).getSize();

    cout << eta_rho << "," << xi_rho << endl;
    mask_rho=Array2<double>(eta_rho,xi_rho);
    cout << "aaaa" << endl;
    varMaskRho.getVar(mask_rho.ptr());
    cout << "bbbb" << endl;

    // Retrieve the variable named "mask_u"
    NcVar varMaskU=dataFile.getVar("mask_u");

    size_t eta_u = varMaskU.getDim(0).getSize();
    size_t xi_u = varMaskU.getDim(1).getSize();

    mask_u=Array2<double>(eta_u,xi_u);
    varMaskU.getVar(mask_u.ptr());

    // Retrieve the variable named "mask_v"
    NcVar varMaskV=dataFile.getVar("mask_v");

    size_t eta_v = varMaskV.getDim(0).getSize();
    size_t xi_v = varMaskV.getDim(1).getSize();

    mask_v=Array2<double>(eta_v,xi_v);
    varMaskU.getVar(mask_v.ptr());

    // Retrieve the variable named "u"
    NcVar varU=dataFile.getVar("u");

    size_t ocean_time = varU.getDim(0).getSize();
    size_t s_rho = varU.getDim(1).getSize();

    u=Array4<double>(ocean_time, s_rho, eta_u, xi_u);
    varU.getVar(u.ptr());

    // Retrieve the variable named "v"
    NcVar varV=dataFile.getVar("v");

    v=Array4<double>(ocean_time, s_rho, eta_v, xi_v);
    varV.getVar(v.ptr());

    // Retrieve the variable named "w"
    NcVar varW=dataFile.getVar("w");

    size_t s_w = varW.getDim(1).getSize();

    w=Array4<double>(ocean_time,s_w,eta_rho,xi_rho);
    varV.getVar(w.ptr());

    // Retrieve the variable named "AKt"
    NcVar varAKt=dataFile.getVar("AKt");

    akt=Array4<double>(ocean_time,s_w,eta_rho,xi_rho);
    varAKt.getVar(akt.ptr());
}

void Wacomm::uv2rho(Array2<double>& mask_u, Array2<double>& mask_v, Array4<double>& u, Array4<double>& v) {
    double uw1, uw2, vw1, vw2;

    LOG4CPLUS_INFO(logger,"uv2rho");

    size_t ocean_time = u.Nx();
    size_t s_rho = u.Ny();
    size_t eta_v = mask_v.Nx();
    size_t xi_v = mask_v.Ny();
    size_t eta_u = mask_v.Nx();
    size_t xi_u = mask_u.Ny();
    size_t eta_rho = mask_rho.Nx();
    size_t xi_rho = mask_rho.Ny();

    for (int t=0; t < ocean_time; t++)
    {
        for (int k=0; k < s_rho; k++)
        {
            for (int j=0; j < eta_v; j++)
            {
                for (int i=0; i< xi_u; i++)
                {

                    if ( j>=0 && i>=0 && j < eta_rho && i < xi_rho && mask_rho.at(j,i) > 0.0 )
                    {
                        if ( j>=0 && i>=0 && j<eta_u && i <xi_u && mask_u.at(j,i) > 0.0 )
                        {
                            uw1=u.at(t,k,j,i);
                        } else {
                            uw1=0.0;
                        }
                        if ( j>0 && i>=0 && j<eta_u && i <xi_u && mask_u.at(j-1,i) > 0.0 )
                        {
                            uw2=u.at(t,k,j-1,i);
                        } else
                        {
                            uw2=0.0;
                        }
                        if ( j>=0 && i>=0 && j<eta_v && i <xi_v && mask_v.at(j,i) > 0.0 )
                        {
                            vw1=v.at(t,k,j,i);
                        } else
                        {
                            vw1=0.0;
                        }
                        if ( j>=0 && i>0 && j<eta_v && i <xi_v && mask_v.at(j,i-1) > 0.0 )
                        {
                            vw2=v.at(t,k,j,i-1);
                        } else
                        {
                            vw2=0.0;
                        }
                        /*
                        ucomp(t,k,j,i)=0.5*(uw1+uw2);
                        vcomp(t,k,j,i)=0.5*(vw1+vw2);
                         */
                        ucomp.at(t,k,j,i,0.5*(uw1+uw2));
                        vcomp.at(t,k,j,i,0.5*(vw1+vw2));
                    } else {
                        /*
                        ucomp(t,k,j,i)=0.0;
                        vcomp(t,k,j,i)=0.0;
                         */
                        ucomp.at(t,k,j,i,0.0);
                        vcomp.at(t,k,j,i,0.0);
                    }
                }
            }
        }
    }
}

void Wacomm::wakt2rho(Array2<double>& mask_u, Array2<double>& mask_v, Array4<double>& w, Array4<double>& akt ) {
    double ww1, ww2, ww3, aktw1, aktw2, aktw3;

    LOG4CPLUS_INFO(logger,"wakt2rho");

    size_t ocean_time = w.Nx();
    size_t s_w = w.Ny();
    size_t eta_v = mask_v.Nx();
    size_t xi_v = mask_v.Ny();
    size_t eta_u = mask_v.Nx();
    size_t xi_u = mask_u.Ny();
    size_t eta_rho = mask_rho.Nx();
    size_t xi_rho = mask_rho.Ny();


    for (int t=0; t < ocean_time; t++) {
        for (int k=0; k < s_w; k++) {
            for (int j=0; j < eta_v; j++) {
                for (int i=0; i< xi_u; i++) {

                    if ( j>=0 && i>=0 && j<eta_rho && i <xi_rho && mask_rho.at(j,i) > 0.0 )
                    {
                        if ( j>=0 && i>0 && j<eta_rho && i <xi_rho && mask_rho.at(j,i-1) > 0.0 )
                        {
                            ww1=w.at(t, k,j,i-1);
                            aktw1=akt.at(t, k,j,i-1);
                        } else {
                            ww1=0.0;
                            aktw1=0.0;
                        }

                        if ( j>0 && i>=0 && j<eta_rho && i <xi_rho && mask_rho.at(j-1,i) > 0.0 ) {
                            ww2=w.at(t, k,j-1,i);
                            aktw2=akt.at(t, k,j-1,i);
                        } else {
                            ww2=0.0;
                            aktw2=0.0;
                        }

                        if ( j>0 && i>0 && j<eta_rho && i <xi_rho && mask_rho.at(j-1,i-1) > 0.0 ) {
                            ww3=w.at(t, k, j-1,i-1);
                            aktw3=akt.at(t, k, j-1,i-1);
                        } else {
                            ww3=0.0;
                            aktw3=0.0;
                        }
                        /*
                        wcomp(t,k,j,i)=0.25*(ww1+ww2+ww3+w.at(t,k,j,i));
                        aktcomp(t,k,j,i)=0.25*(aktw1+aktw2+aktw3+akt.at(t,k,j,i));
                         */
                        wcomp.at(t,k,j,i,0.25*(ww1+ww2+ww3+w.at(t,k,j,i)));
                        aktcomp.at(t,k,j,i,0.25*(aktw1+aktw2+aktw3+akt.at(t,k,j,i)));
                    } else {
                        /*
                        wcomp(t,k,j,i)=0.0;
                        aktcomp(t,k,j,i)=0.0;
                         */
                        wcomp.at(t,k,j,i,0.0);
                        aktcomp.at(t,k,j,i,0.0);
                    }
                }
            }
        }
    }
}



