//
// Created by Raffaele Montella on 12/5/20.
//

#include <cfloat>
#include "Particle.hpp"
#include "Config.hpp"

Particle::Particle(double j, double k, double i,
                   double health, double tpart):
                   j(j), k(k), i(i),
                   health(health),tpart(tpart)
{
}

Particle::~Particle() {
}

bool Particle::isAlive() {
    return health>0;
}



void Particle::move(std::shared_ptr<Config> config,
                    int ocean_time_idx,
                    std::shared_ptr<OceanModelAdapter> oceanModelAdapter) {

    double dti=config->Dti();
    double deltat=config->Deltat();
    double tau0=config->Tau0();
    double survprob=config->Survprob();

    double idet=0,jdet=0,kdet=0;
    double crid=1;
    double vs=0;

    double iint=deltat/dti;
    double t=0;

    size_t s_w = oceanModelAdapter->W()->Nx();
    size_t eta_rho = oceanModelAdapter->Mask()->Nx();
    size_t xi_rho = oceanModelAdapter->Mask()->Ny();

    for (unsigned t=0;t<iint;t++) {
        if (health<survprob) {
            pstatus=0;
            return;
        }
        unsigned int iI=(unsigned int)i; double iF=i-iI;
        unsigned int jI=(unsigned int)j; double jF=j-jI;
        unsigned int kI=(unsigned int)k; double kF=k-kI;

        if (oceanModelAdapter->Mask()->operator ()(jI,iI)<=0.0 || jI>=eta_rho|| iI>=xi_rho) {
            health=-1;
            pstatus=0;
            return;
        }

        if (k>=s_w) {
            health=-13;
            pstatus=0;
            return;
        }

        //printf("Alive:%u kI:%u jI:%u iI:%u kF:%f jF:%f iF:%f\n",id,kI,jI,iI,kF,jF,iF);

        double u1=oceanModelAdapter->U()->operator()(ocean_time_idx,kF,jF,iF)*(1-iF)*(1-jF);
        double u2=oceanModelAdapter->U()->operator()(ocean_time_idx,kF,jF+1,iF)*(1-iF)*jF;
        double u3=oceanModelAdapter->U()->operator()(ocean_time_idx,kF,jF+1,iF+1)*iF*jF;
        double u4=oceanModelAdapter->U()->operator()(ocean_time_idx,kF,jF,iF+1)*iF*(1-jF);
        double uu=u1+u2+u3+u4;

        //printf("uu:%f\n",uu);

        double v1=oceanModelAdapter->V()->operator()(ocean_time_idx,kF,jF,iF)*(1-iF)*(1-jF);
        double v2=oceanModelAdapter->V()->operator()(ocean_time_idx,kF,jF+1,iF)*(1-iF)*jF;
        double v3=oceanModelAdapter->V()->operator()(ocean_time_idx,kF,jF+1,iF+1)*iF*jF;
        double v4=oceanModelAdapter->V()->operator()(ocean_time_idx,kF,jF,iF+1)*iF*(1-jF);
        double vv=v1+v2+v3+v4;

        //printf("vv:%f\n",vv);

        double w1=oceanModelAdapter->W()->operator()(ocean_time_idx,kF,jF,iF)*(1-iF)*(1-jF)*(1-kF);
        double w2=oceanModelAdapter->W()->operator()(ocean_time_idx,kF,jF+1,iF)*(1-iF)*jF*(1-kF);
        double w3=oceanModelAdapter->W()->operator()(ocean_time_idx,kF,jF+1,iF+1)*iF*jF*(1-kF);
        double w4=oceanModelAdapter->W()->operator()(ocean_time_idx,kF,jF,iF+1)*iF*(1-jF)*(1-kF);
        double w5=oceanModelAdapter->W()->operator()(ocean_time_idx,kF+1,jF,iF)*(1-iF)*(1-jF)*kF;
        double w6=oceanModelAdapter->W()->operator()(ocean_time_idx,kF+1,jF+1,iF)*(1-iF)*jF*kF;
        double w7=oceanModelAdapter->W()->operator()(ocean_time_idx,kF+1,jF+1,iF+1)*iF*jF*kF;
        double w8=oceanModelAdapter->W()->operator()(ocean_time_idx,kF+1,jF,iF+1)*iF*(1-jF)*kF;
        double ww=w1+w2+w3+w4+w5+w6+w7+w8;

        //printf("ww:%f\n",ww);

        double ileap=uu*dti;
        double jleap=vv*dti;
        double kleap=(vs+ww)*dti;

        //printf ("kleap:%f jleap:%f ileap:%f\n",kleap,jleap,ileap); 

        double sigmaprof=3.46*(1+kdet/s_w);
        double gi=0,gj=0,gk=0;
        for (int a=0;a<12;a++) {
            gi=gi+gen()-0.5;
            gj=gj+gen()-0.5;
            gk=gk+gen()-0.5;
        }

        double a1=oceanModelAdapter->AKT()->operator()(ocean_time_idx,kF,jF,iF)*(1-iF)*(1-jF)*(1-kF);
        double a2=oceanModelAdapter->AKT()->operator()(ocean_time_idx,kF,jF+1,iF)*(1-iF)*jF*(1-kF);
        double a3=oceanModelAdapter->AKT()->operator()(ocean_time_idx,kF,jF+1,iF+1)*iF*jF*(1-kF);
        double a4=oceanModelAdapter->AKT()->operator()(ocean_time_idx,kF,jF,iF+1)*iF*(1-jF)*(1-kF);
        double a5=oceanModelAdapter->AKT()->operator()(ocean_time_idx,kF+1,jF,iF)*(1-iF)*(1-jF)*kF;
        double a6=oceanModelAdapter->AKT()->operator()(ocean_time_idx,kF+1,jF+1,iF)*(1-iF)*jF*kF;
        double a7=oceanModelAdapter->AKT()->operator()(ocean_time_idx,kF+1,jF+1,iF+1)*iF*jF*kF;
        double a8=oceanModelAdapter->AKT()->operator()(ocean_time_idx,kF+1,jF,iF+1)*iF*(1-jF)*kF;
        double aa=a1+a2+a3+a4+a5+a6+a7+a8;
        //printf("aa:%f\n",aa);

        double rileap=gi*sigmaprof;
        double rjleap=gj*sigmaprof;
        double rkleap=gk*aa*crid;

        //printf ("rkleap:%f rjleap:%f rileap:%f\n",rkleap,rjleap,rileap);

        ileap=ileap+rileap;
        jleap=jleap+rjleap;
        kleap=kleap+rkleap;

        double d1,d2,dist;

        d1=(oceanModelAdapter->Lat()->operator()(jI+1,iI)-oceanModelAdapter->Lat()->operator()(jI,iI));
        d2=(oceanModelAdapter->Lon()->operator()(jI,iI+1)-oceanModelAdapter->Lon()->operator()(jI,iI));
        d1=pow(sin(0.5*d1),2) + pow(sin(0.5*d2),2)* cos(oceanModelAdapter->Lat()->operator()(jI+1,iI))*cos(oceanModelAdapter->Lat()->operator()(jI,iI));
        dist=2.0*atan2(pow(d1,.5),pow(1.0-d1,.5))*6371.0;
        idet=i+0.001*ileap/dist;

        d1=(oceanModelAdapter->Lat()->operator()(jI+1,iI)-oceanModelAdapter->Lat()->operator()(jI,iI));
        d2=(oceanModelAdapter->Lon()->operator()(jI,iI+1)-oceanModelAdapter->Lon()->operator()(jI,iI));
        d1=pow(sin(0.5*d1),2) + pow(sin(0.5*d2),2)* cos(oceanModelAdapter->Lat()->operator()(jI+1,iI))*cos(oceanModelAdapter->Lat()->operator()(jI,iI));
        dist=2.0*atan2(sqrt(d1),pow(1.0-d1,.5))*6371.0;
        jdet=j+0.001*jleap/dist;

        //printf("depth:%f, zeta:%f\n",depth[kI],zeta[jI][iI]);
        dist=oceanModelAdapter->Depth()->operator()(kI)*oceanModelAdapter->Zeta()->operator()(jI,iI);
        if (dist<=0) dist=1e-4;
        //printf("dist:%f, kleap:%f\n",dist,kleap);
        if ( abs(kleap) > abs(dist) ) {
            kleap=sign(dist,kleap);
        }
        kdet=k+kleap/dist;

        //printf("kdet:%f jdet:%f idet:%f\n",kdet,jdet,idet);

        if ( kdet >= s_w ) {
            kdet=s_w-1;
        }
        if ( kdet < 0. ) {
            kdet=0;
        }

        //printf("kdet:%f jdet:%f idet:%f\n",kdet,jdet,idet);

        unsigned int jdetI=(unsigned int)jdet;
        unsigned int idetI=(unsigned int)idet;
        if ( oceanModelAdapter->Mask()->operator()(jdetI,idetI) <= 0.0 ) {
            if ( idetI < iI ) {
                idet=(double)iI + abs(i-idet);
            } else if ( idetI > iI ) {
                idet=(double)idetI- mod(idet,1.0);
            }
            if ( jdetI < jdet ) {
                jdet=(double)jdetI+ abs(j-jdet);
            } else if ( jdetI > jI ) {
                jdet=(double)jdetI - mod(jdet,1.0);
            }
        }

        i=idet;
        j=jdet;
        k=kdet;

        time=time+dti;
        health=health0*exp(-time/tau0);
    }
}

double Particle::K()  { return k; }
double Particle::J()  { return j; }
double Particle::I()  { return i; }

int Particle::iK()  { return (int)(round(k));}
int Particle::iJ()  { return (int)(round(j));}
int Particle::iI()  { return (int)(round(i));}

/* The random generator should be initialized only one time */
double Particle::gen()
{
    std::random_device randomDevice;
    std::mt19937 randomGenerator=std::mt19937 (randomDevice());
    std::uniform_real_distribution<double> randomDistribution=std::uniform_real_distribution<double>(0, std::nextafter(1, DBL_MAX));
    return randomDistribution(randomGenerator);
}

double Particle::sgn(double a) { return (a > 0) - (a < 0); }
double Particle::mod(double a, double p) { return a-p*(int)(a/p); }
double Particle::sign(double a, double b) { return abs(a)*sgn(b); }




