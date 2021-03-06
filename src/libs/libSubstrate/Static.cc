/* Copyright (C) 2005-2011 M. T. Homer Reid
 *
 * This file is part of SCUFF-EM.
 *
 * SCUFF-EM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SCUFF-EM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * libSubstrate/Static.cc -- compute the electrostatic green's function
 *                        -- above a multi-layer dielectric substrate
 *
 * homer reid             -- 3/2017
 *
 */
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <fenv.h>

#include "libhrutil.h"
#include "libMDInterp.h"
#include "libSGJC.h"
#include "libSpherical.h"
#include "libSubstrate.h"

/*****************************************************************/
/* add the contributions of a charge of strength Q at (xs,ys,zs) */
/* to the potential and E-field of a charge of strength Q at XDest*/
/*****************************************************************/
void AddPhiE0(double XDest[3], double xs, double ys, double zs, double Q, double PhiE[4])
{
  double R[3];
  R[0]=XDest[0]-xs;
  R[1]=XDest[1]-ys;
  R[2]=XDest[2]-zs;
  double r2 = R[0]*R[0] + R[1]*R[1] + R[2]*R[2];
  double Term = Q/(4.0*M_PI*sqrt(r2));
  PhiE[0] += Term;
  Term/=r2;
  PhiE[1] += R[0]*Term;
  PhiE[2] += R[1]*Term;
  PhiE[3] += R[2]*Term;
}

void AddPhiE0(double XDest[3], double XSource[3], double Q, double PhiE[4])
{ AddPhiE0(XDest, XSource[0], XSource[1], XSource[2], Q, PhiE); }

/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
double LayeredSubstrate::GetStaticG0Correction(double z)
{
  UpdateCachedEpsMu(0.0);

  for(int n=0; n<NumInterfaces; n++)
   if ( EqualFloat(z,zInterface[n]) )
    { 
      // break ties by assuming source and observation points
      // lie in whichever region has the lower permittivity
      double EpsA = real(EpsLayer[n]);
      double EpsB = real(EpsLayer[n+1]);
      return 2.0*fmin(EpsA, EpsB)/(EpsA + EpsB);
    };

  return 1.0;
}

double LayeredSubstrate::GetStaticG0Correction(double zD, double zS)
{ 
  if (!EqualFloat(zD, zS))
   return 1.0;
  return GetStaticG0Correction(zD);
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void GetZetaXi(double q, double zD, double zS, double zGP,
               double ZetaXi[2], double Sign=0.0)
{
  bool HaveGP = (zGP!=-1.0*HUGE_VAL);

  double Term1 = exp(-q*fabs(zD-zS));
  double Term2 = HaveGP ? exp(-q*(zD + zS - 2.0*zGP)) : 0.0;
  if (Sign==0.0)
   Sign  = (zD >= zS) ? 1.0 : -1.0;

  ZetaXi[0] =      Term1 - Term2;
  ZetaXi[1] = Sign*Term1 - Term2;
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void LayeredSubstrate::GetSigmaTwiddle(double zS, double q, double *SigmaTwiddle)
{
  UpdateCachedEpsMu(0.0);

  /*--------------------------------------------------------------*/
  /* assemble RHS vector                                          */
  /*--------------------------------------------------------------*/
  double *RHS = new double[NumInterfaces];
  for(int m=0; m<NumInterfaces; m++)
   { 
     double Sign=0.0;
     if (EqualFloat(zInterface[m], zS))
      { double EpsA = real( EpsLayer[m]);
        double EpsB = real( EpsLayer[m+1]);
        Sign = (EpsA < EpsB) ? -1.0 : 1.0;
      };

     double ZetaXi[2];
     GetZetaXi(q, zInterface[m], zS, zGP, ZetaXi, Sign);
     RHS[m] = -1.0*ZetaXi[1];
   };

  /*--------------------------------------------------------------*/
  /* assemble M matrix                                           -*/
  /*--------------------------------------------------------------*/
  double *M = new double[NumInterfaces*NumInterfaces];
  bool Degenerate=false;
  for(int m=0; m<NumInterfaces; m++)
   for(int n=0; n<NumInterfaces; n++)
    { 
      if (m==n)
       { double EpsA     = real(EpsLayer[m]);
         double EpsB     = real(EpsLayer[m+1]);
         double DeltaEps = EpsA   - EpsB;
         if (DeltaEps==0.0)
          { Degenerate=true;
            continue; // this is handled below
          };
         double ZetaXiP[2], ZetaXiM[2];
         GetZetaXi(q, zInterface[m], zInterface[m], zGP, ZetaXiP, +1.0);
         GetZetaXi(q, zInterface[m], zInterface[m], zGP, ZetaXiM, -1.0);
         M[m+m*NumInterfaces] = (EpsA*ZetaXiP[1] - EpsB*ZetaXiM[1])/DeltaEps;
       }
      else
       { double ZetaXi[2];
         GetZetaXi(q, zInterface[m], zInterface[n], zGP, ZetaXi);
         M[m+n*NumInterfaces]=ZetaXi[1];
       };
    };

  /*--------------------------------------------------------------*/
  /*- handle degenerate cases in which Eps_m = Eps_{m-1} by       */
  /*- replacing equation #m with 1*Sigma_m = 0                    */
  /*--------------------------------------------------------------*/
  if (Degenerate)
   for(int m=0; m<NumInterfaces; m++)
    { double EpsA   = real(EpsLayer[m]);
      double EpsB   = real(EpsLayer[m+1]);
      if (EqualFloat(EpsA, EpsB))
       { RHS[m]=0.0;
         for(int n=0; n<NumInterfaces; n++)
          M[m + n*NumInterfaces]=(m==n) ? 1.0 : 0.0;
      };
   };

  /*--------------------------------------------------------------*/
  /*- solve the system--------------------------------------------*/
  /*--------------------------------------------------------------*/
  if (NumInterfaces==1)
   { 
     SigmaTwiddle[0] = RHS[0] / M[0];
   }
  else if (NumInterfaces==2)
   { double M11=M[0], M21=M[1], M12=M[2], M22=M[3];
     double R1=RHS[0], R2=RHS[1];
     double Denom = M11*M22 - M21*M12;
     SigmaTwiddle[0] = ( M22*R1 - M12*R2 )/Denom;
     SigmaTwiddle[1] = (-M21*R1 + M11*R2 )/Denom;
   }
  else
   { HMatrix MMatrix(NumInterfaces, NumInterfaces, LHM_REAL, M);
     HVector RVector(NumInterfaces, LHM_REAL, RHS);
     MMatrix.LUFactorize();
     MMatrix.LUSolve(&RVector);
     memcpy(SigmaTwiddle, RHS, NumInterfaces*sizeof(double));
   };

  delete[] RHS;
  delete[] M;
  
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
typedef struct qIntegrandData
 { double RhoMag;   // transverse distance dest-source
   double zD;       // evaluation point z coordinate 
   double zS;       // source point z coordinate 
   LayeredSubstrate *S;
   FILE *LogFile;
   int NCalls;
 } qIntegrandData;

static int qIntegrand(unsigned ndim, const double *u, 
                      void *UserData, unsigned fdim, 
                      double *qIntegral)
{
  (void) fdim; // unused 
  (void) ndim; // unused 

  qIntegral[0]=qIntegral[1]=qIntegral[2]=0.0;

  /*--------------------------------------------------------------*/
  /* integrand vanishes at q->infinity                            */
  /*--------------------------------------------------------------*/
  if ( EqualFloat(u[0],1.0) )
   return 0;

  double Denom   = 1.0-u[0];
  double q       = u[0] / Denom;
  double Jac     = 1.0/(Denom*Denom);

  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  qIntegrandData *qID= (qIntegrandData *)UserData;
  qID->NCalls++;

  double RhoMag        = qID->RhoMag;
  double zD            = qID->zD;
  double zS            = qID->zS;
  LayeredSubstrate *S  = qID->S;
  int NumInterfaces    = S->NumInterfaces;
  double *zInterface   = S->zInterface;
  double zGP           = S->zGP;
  cdouble *EpsLayer    = S->EpsLayer;

  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  double *SigmaTwiddle = new double[NumInterfaces];
  S->GetSigmaTwiddle(zS, q, SigmaTwiddle);

  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  double qRho = q*RhoMag;
  double J0, J1;
  if (qRho<1.0e-4) // Bessel-function series expansions for small arguments
   { double qRho2=qRho*qRho;
     J0 = 1.0 - qRho2/4.0;
     J1 = 0.5*qRho*(1.0 - qRho2/8.0);
   }
  else if (qRho>1.0e2) // asymptotic forms for large argument
   {
     double Factor = sqrt(2.0/(M_PI*qRho));
     J0 = Factor * cos(qRho - 0.25*M_PI);
     J1 = Factor * cos(qRho - 0.75*M_PI);
   }
  else
  { cdouble J0J1[2];
    double Workspace[16];
    AmosBessel('J', qRho, 0.0, 2, false, J0J1, Workspace);
    J0=real(J0J1[0]);
    J1=real(J0J1[1]);
  };
  J0*=Jac/(4.0*M_PI);
  J1*=Jac/(4.0*M_PI);

  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  for(int n=0; n<NumInterfaces; n++)
   { 
     double ZetaXi[2];
     GetZetaXi(q, zD, zInterface[n], zGP, ZetaXi);

     qIntegral[0] +=     J0 * ZetaXi[0] * SigmaTwiddle[n];
     qIntegral[1] += q * J1 * ZetaXi[0] * SigmaTwiddle[n];
     qIntegral[2] += q * J0 * ZetaXi[1] * SigmaTwiddle[n];

     if ( EqualFloat(zD,zInterface[n]) && EqualFloat(zS,zInterface[n]) )
      { double EpsA   = real(EpsLayer[n]);
        double EpsB   = real(EpsLayer[n+1]);
        double Sign   = (EpsA < EpsB) ? 1.0 : -1.0;
        double Factor = Sign*(EpsA- EpsB) / (EpsA + EpsB);
        qIntegral[0] -= J0*Factor;
        qIntegral[1] -= q*J1*Factor;
        qIntegral[2] -= q*J0*Sign*Factor;
      };
   };

  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  if (qID->LogFile)
   { FILE *LogFile = qID->LogFile;
     static bool init=true;
     if (init)
      { init=false;
        fprintf(LogFile,"\n\n");
        fprintf(LogFile,"#1 2 3 4    q  Rho zD zS\n");
        fprintf(LogFile,"#6 7 8 9 10 J0 J1 qI[0] qI[1] qI[2]\n");
        fprintf(LogFile,"#11 12 13  Sigma,Zeta,Xi (layer 0) \n");
        fprintf(LogFile,"#14 15 16  Sigma,Zeta,Xi (layer 1) \n");
      };
     fprintf(LogFile,"%e %e %e %e ", q, RhoMag, zD, zS);
     fprintf(LogFile,"%e %e ", J0, J1);
     fprintf(LogFile,"%e %e %e ",qIntegral[0],qIntegral[1],qIntegral[2]);
     for(int n=0; n<NumInterfaces; n++)
      { double ZetaXi[2];
        GetZetaXi(q, zD, zInterface[n], zGP, ZetaXi);
        fprintf(LogFile,"%e %e %e ",SigmaTwiddle[n],ZetaXi[0],ZetaXi[1]);  
      };
     fprintf(LogFile,"\n");
   };

  delete[] SigmaTwiddle;

  return 0;
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void LayeredSubstrate::GetqIntegral(double RhoMag, double zD, double zS, 
                                    double qIntegral[3])
{ 
  char *LogFileName = getenv("SSGF_LOGFILE");
  FILE *LogFile     = LogFileName ? fopen(LogFileName, "a") : 0;
  qIntegrandData MyqID, *qID=&MyqID;
  qID->RhoMag  = RhoMag;
  qID->zD      = zD;
  qID->zS      = zS;
  qID->S       = this;
  qID->LogFile = LogFile;
  qID->NCalls  = 0;
  double uMin=0.0, uMax=1.0;
  int FDim=3;
  int NDim=1;
  double Error[3];
  int MaxEval   = qMaxEval;
  double AbsTol = qAbsTol;
  double RelTol = qRelTol;
  hcubature(FDim, qIntegrand, (void *)qID, NDim, &uMin, &uMax,
	    MaxEval, AbsTol, RelTol, ERROR_INDIVIDUAL, qIntegral, Error);
  if (LogFile)
   fclose(LogFile);

}

/***************************************************************/
/* Compute the extra contributions to the potential and E-field*/
/* at XDest due to a point charge at XSource in the presence of*/
/* a substrate.                                                */
/*                                                             */
/* 'Extra contributions' include everything but the direct     */
/* contributions of the point charge in vacuum.                */
/***************************************************************/
void LayeredSubstrate::GetDeltaPhiE(double XD[3], double XS[3],
                                    double PhiE[4], 
                                    double *pG0Correction)
{  
  double Rho[2], ZD=XD[2], ZS=XS[2];
  Rho[0] = XD[0]-XS[0];
  Rho[1] = XD[1]-XS[1];
  double RhoMag = sqrt(Rho[0]*Rho[0] + Rho[1]*Rho[1]);
  double CosTheta = (RhoMag==0.0) ? 1.0 : Rho[0]/RhoMag;
  double SinTheta = (RhoMag==0.0) ? 0.0 : Rho[1]/RhoMag;

  /***************************************************************/
  /* try to get values of q integral using lookup table          */
  /***************************************************************/
  double qIntegral[3];
  bool GotqIntegral=false;
  if (     I1D
        && I1DOmega==0.0
        && EqualFloat(ZD, ZS) && EqualFloat(ZD, I1DZ)
        && I1DRhoMin<=RhoMag && RhoMag<=I1DRhoMax
     )
   {
     GotqIntegral = I1D->Evaluate(RhoMag, qIntegral);
   };
 
  /***************************************************************/
  /*- evaluate q integral to get contributions of surface        */
  /*- charges at dielectric interface                            */
  /***************************************************************/
  if (!GotqIntegral) 
   GetqIntegral(RhoMag, ZD, ZS, qIntegral);

  /***************************************************************/
  /***************************************************************/
  /***************************************************************/
  PhiE[0] = qIntegral[0];
  PhiE[1] = CosTheta*qIntegral[1];
  PhiE[2] = SinTheta*qIntegral[1];
  PhiE[3] = qIntegral[2];
  if (pG0Correction)
   *pG0Correction=GetStaticG0Correction(ZD,ZS);

  // contribution of image charge if present
  if (zGP != -1.0*HUGE_VAL)
   AddPhiE0(XD, XS[0], XS[1], 2.0*zGP-XS[2], -1.0, PhiE);
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void LayeredSubstrate::GetTotalPhiE(double XD[3], double XS[3],
                                    double PhiE[4])
{
  double G0Correction;
  GetDeltaPhiE(XD, XS, PhiE, &G0Correction);
  AddPhiE0(XD, XS, G0Correction, PhiE);
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void LayeredSubstrate::GetPhiE(double XD[3], double XS[3],
                               double PhiE[4], bool Total)
{ Total ?  GetTotalPhiE(XD, XS, PhiE) : GetDeltaPhiE(XD, XS, PhiE); }
