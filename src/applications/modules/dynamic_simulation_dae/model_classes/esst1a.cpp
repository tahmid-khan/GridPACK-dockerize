/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
// -------------------------------------------------------------
/**
 * @file   esst1a.cpp
 * @author Shuangshuang Jin 
 * @author Shrirang Abhyankar
 * @Last modified - 04/22/20 
 *  
 * @brief ESST1A exciter model implementation 
 *
 *
 */

#include <esst1a.hpp>
#include <gridpack/include/gridpack.hpp>
#include <constants.hpp>

Esst1aExc::Esst1aExc(void)
{
  Vmeas = 0.0; 
  dVmeas = 0.0;
  xLL1 = 0.0; 
  dxLL1 = 0.0;
  xLL2  = 0.0;
  dxLL2 = 0.0;
  Va    = 0.0;
  dVa   = 0.0;
  xf    = 0.0;
  dxf   = 0.0;
  UEL = 0; 
  VOS = 0; 
  Tr = 0.0; 
  Vimax = 0.0; 
  Vimin = 0.0; 
  Tc = 0.0; 
  Tb = 0.0;
  Tc1 = 0.0;
  Tb1 = 0.0; 
  Ka = 0.0;
  Ta = 0.0;
  Vamax = 0.0;
  Vamin = 0.0;
  Vrmax = 0.0;
  Vrmin = 0.0;
  Kc = 0.0;
  Kf = 0.0;
  Tf = 0.0;
  Klr = 0.0;
  Ilr = 0.0;    

  nxexc = 5;
}

Esst1aExc::~Esst1aExc(void)
{
}

/**
 * Load parameters from DataCollection object into exciter model
 * @param data collection of exciter parameters from input files
 * @param index of exciter on bus
 * TODO: might want to move this functionality to BaseExciterModel
 */
void Esst1aExc::load(const boost::shared_ptr<gridpack::component::DataCollection> data, int idx)
{
  BaseExcModel::load(data,idx); // load parameters in base exciter model
  
  // load parameters for the model type
  data->getValue(EXCITER_UEL, &UEL, idx);
  data->getValue(EXCITER_VOS, &VOS, idx);
  data->getValue(EXCITER_TR, &Tr, idx);
  data->getValue(EXCITER_VIMAX, &Vimax, idx);
  data->getValue(EXCITER_VIMIN, &Vimin, idx);
  data->getValue(EXCITER_TC, &Tc, idx);
  data->getValue(EXCITER_TB, &Tb, idx);
  data->getValue(EXCITER_TC1, &Tc1, idx);
  data->getValue(EXCITER_TB1, &Tb1, idx);
  data->getValue(EXCITER_KA, &Ka, idx);
  data->getValue(EXCITER_TA, &Ta, idx);
  data->getValue(EXCITER_VAMAX, &Vamax, idx);
  data->getValue(EXCITER_VAMIN, &Vamin, idx);
  data->getValue(EXCITER_VRMAX, &Vrmax, idx);
  data->getValue(EXCITER_VRMIN, &Vrmin, idx);
  data->getValue(EXCITER_KC, &Kc, idx);
  data->getValue(EXCITER_KF, &Kf, idx);
  data->getValue(EXCITER_TF, &Tf, idx);
  data->getValue(EXCITER_KLR, &Klr, idx);
  data->getValue(EXCITER_ILR, &Ilr, idx);

  if(fabs(Klr) > 1e-6) {
    printf("ESST1A model does not support non-zero Klr yet\n");
    exit(1);
  }

  // Set flags for differential or algebraic equations
  iseq_diff[0] = (Tr == 0)?0:1;
  iseq_diff[1] = (Tb == 0 || Tc == 0)?0:1;
  iseq_diff[2] = (Tb1 == 0 || Tc1 == 0)?0:1;
  iseq_diff[3] = (Ta == 0)?0:1;
  iseq_diff[4] = 1; // Tf is always > 0
}


/**
 * Initialize exciter model before calculation
 * @param [output] values - array where initialized exciter variables should be set
 */
void Esst1aExc::init(gridpack::ComplexType* values) 
{
  double Ec = sqrt(VD*VD + VQ*VQ);
  double yLL2,yLL1;
  double Vf=0.0,Vfd;
  BaseGenModel *gen=getGenerator();
  double LadIfd = gen->getFieldCurrent();

  // Field voltage (Efd) and bus voltage (VD,VQ) are already set 
  // Need to set the initial values for all the state variables

  Ec = sqrt(VD*VD + VQ*VQ);
  Vfd = Klr*(LadIfd - Ilr); 
  Vmeas    = Ec;
  xf       = -Kf/Tf*Efd;
  Va       = Efd + Vfd;
  yLL2     = Va/Ka;
  yLL1     = yLL2;
  if(iseq_diff[2]) xLL2    = (1 - Tc1/Tb1)*yLL2;
  else xLL2 = yLL2;
  Vref     = yLL1 + Vmeas + Vf;
  if(iseq_diff[1]) xLL1    = (1 - Tc/Tb)*(Vref - Vmeas - Vf);
  else xLL1 = Vref - Vmeas - Vf;

  values[0] = Vmeas;
  values[1] = xLL1;
  values[2] = xLL2;
  values[3] = Va;
  values[4] = xf;
}

/**
 * Write output from exciters to a string.
 * @param string (output) string with information to be printed out
 * @param bufsize size of string buffer in bytes
 * @param signal an optional character string to signal to this
 * routine what about kind of information to write
 * @return true if bus is contributing string to output, false otherwise
 */
bool Esst1aExc::serialWrite(char *string, const int bufsize,const char *signal)
{
  return false;
}

/**
 * Set event
 */
void Esst1aExc::setEvent(gridpack::math::DAESolver::EventManagerPtr eman)
{

}

/**
 * Write out exciter state
 * @param signal character string used to determine behavior
 * @param string buffer that contains output
 */
void Esst1aExc::write(const char* signal, char* string)
{
}

/**
 *  Set the number of variables for this exciter model
 *  @param [output] number of variables for this model
 */
bool Esst1aExc::vectorSize(int *nvar) const
{
  *nvar = nxexc;
  return true;
}

/**
 * Set the internal values of the voltage magnitude and phase angle. Need this
 * function to push values from vectors back onto exciters
 * @param values array containing exciter state variables
*/
void Esst1aExc::setValues(gridpack::ComplexType *values)
{  
  if(p_mode == XVECTOBUS) {
    Vmeas = real(values[0]);
    xLL1 = real(values[1]);
    xLL2 = real(values[2]);
    Va   = real(values[3]);
    xf   = real(values[4]);
  } else if(p_mode == XDOTVECTOBUS) {
    dVmeas = real(values[0]);
    dxLL1 = real(values[1]);
    dxLL2 = real(values[2]);
    dVa   = real(values[3]);
    dxf   = real(values[4]);
  } else if(p_mode == XVECPRETOBUS) {
    Vmeasprev = real(values[0]);
    xLL1prev = real(values[1]);
    xLL2prev = real(values[2]);
    Vaprev   = real(values[3]);
    xfprev   = real(values[4]);
  }
}

/**
 * Return the values of the exciter vector block
 * @param values: pointer to vector values
 * @return: false if exciter does not contribute
 *        vector element
 */
bool Esst1aExc::vectorValues(gridpack::ComplexType *values)
{
  int x1_idx = 0;
  int x2_idx = 1;
  int x3_idx = 2;
  int x4_idx = 3;
  int x5_idx = 4;
  double Ec,yLL1,yLL2,Vf;
  Ec = sqrt(VD*VD + VQ*VQ);
  
  Efd = Esst1aExc::getFieldVoltage();
  // On fault (p_mode == FAULT_EVAL flag), the exciter variables are held constant. This is done by setting the vector values of residual function to 0.0.
  if(p_mode == FAULT_EVAL) {
    // Vmeas equation
    if(iseq_diff[0]) values[0] = Vmeas - Vmeasprev;
    else values[0] = -Vmeas + Ec;

    // xLL1 equation
    Vf = xf + Kf/Tf*Efd;
    if(iseq_diff[1]) {
      values[1] = xLL1 - xLL1prev;
      yLL1 = xLL1 + Tc/Tb*(Vref - Vmeas - Vf);
    } else {
      values[1] = -xLL1 + Vref - Vmeas - Vf;
      yLL1 = xLL1;
    }

    // xLL2 equation
    if(iseq_diff[2]) {
      values[2] = xLL2 - xLL2prev;
      yLL2 = xLL2 + Tc1/Tb1*yLL1;
    } else {
      values[2] = -xLL2 + yLL1;
      yLL2 = xLL2;
    }

    // Va equation
    if(iseq_diff[3]) values[3] = Va - Vaprev;
    else values[3] = -Va + Ka*yLL2;

    // xf equation
    values[4] = xf - xfprev;

  } else if(p_mode == RESIDUAL_EVAL) {

    // Vmeas equation
    if(iseq_diff[0]) values[0] = (-Vmeas + Ec)/Tr - dVmeas;
    else values[0] = -Vmeas + Ec;

    // xLL1 equation
    Vf = xf + Kf/Tf*Efd;
    if(iseq_diff[1]) {
      values[1] = (-xLL1 + (1 - Tc/Tb)*(Vref - Vmeas - Vf))/Tb - dxLL1;
      yLL1 = xLL1 + Tc/Tb*(Vref - Vmeas - Vf);
    } else {
      values[1] = -xLL1 + Vref - Vmeas - Vf;
      yLL1 = xLL1;
    }

    // xLL2 equation
    if(iseq_diff[2]) {
      values[2] = (-xLL2 + (1 - Tc1/Tb1)*yLL1)/Tb1 - dxLL2;
      yLL2 = xLL2 + Tc1/Tb1*yLL1;
    } else {
      values[2] = -xLL2 + yLL1;
      yLL2 = xLL2;
    }

    // Va equation
    if(iseq_diff[3]) values[3] = (-Va + Ka*yLL2)/Ta - dVa;
    else values[3] = -Va + Ka*yLL2;

    // xf equation
    values[4] = (-xf - Kf/Tf*Efd)/Tf - dxf;
  }
  
  return true;
}

/**
 * Set Jacobian block
 * @param values a 2-d array of Jacobian block for the bus
 */
bool Esst1aExc::setJacobian(gridpack::ComplexType **values)
{
  int Vmeas_idx = offsetb;
  int xLL1_idx  = offsetb+1;
  int xLL2_idx  = offsetb+2;
  int Va_idx    = offsetb+3;
  int xf_idx    = offsetb+4;
  int VD_idx    = 0;
  int VQ_idx    = 1;
  double Ec,yLL1,yLL2,Vf;
  double dyLL2_dVmeas=0.0,dyLL2_dxLL1=0.0;
  double dyLL2_dxLL2=0.0,dyLL2_dVa=0.0;
  double dyLL2_dxf=0.0;
  double dVf_dxf = 1.0;
  double dVf_dEfd = Kf/Tf;
  double dyLL1_dxLL1=0.0,dyLL1_dVmeas=0.0;
  double dyLL1_dxLL2=0.0,dyLL1_dVa=0.0;
  double dyLL1_dxf=0.0, dyLL1_dEfd=0.0;

  Ec = sqrt(VD*VD + VQ*VQ);

  double dEc_dVD = VD/Ec;
  double dEc_dVQ = VQ/Ec;

  if(p_mode == FAULT_EVAL) {
    // Partial derivatives of Vmeas equation
    if(iseq_diff[0]) {
      values[Vmeas_idx][Vmeas_idx] = 1.0;
    } else {
      values[Vmeas_idx][Vmeas_idx] = -1.0;
      values[VD_idx][Vmeas_idx] = dEc_dVD;
      values[VQ_idx][Vmeas_idx]  = dEc_dVQ;
    }
      
    // Partial derivatives of xLL1 equation
    if(iseq_diff[1]) {
      values[xLL1_idx][xLL1_idx] = 1.0;
      yLL1 = xLL1 + Tc/Tb*(Vref - Vmeas - Vf);
      dyLL1_dxLL1  = 1.0;
      dyLL1_dVmeas = -Tc/Tb;
      dyLL1_dxf    = -Tc/Tb*dVf_dxf;
    } else {
      values[Vmeas_idx][xLL1_idx] = -1.0;
      values[xLL1_idx][xLL1_idx]  = -1.0;
      values[xf_idx][xLL1_idx]    = -dVf_dxf;
      yLL1 = xLL1;
      dyLL1_dxLL1 = 1.0;
    }

    // Partial derivatives of xLL2 equation
    if(iseq_diff[2]) {
      values[xLL2_idx][xLL2_idx] = 1.0;
      yLL2 = xLL2 + Tc1/Tb1*yLL1;

      dyLL2_dVmeas = Tc1/Tb1*dyLL1_dVmeas;
      dyLL2_dxLL1  = Tc1/Tb1*dyLL1_dxLL1;
      dyLL2_dxLL2  = 1.0 + Tc1/Tb1*dyLL1_dxLL2;
      dyLL2_dVa    = Tc1/Tb1*dyLL1_dVa;
      dyLL2_dxf    = Tc1/Tb1*dyLL1_dxf;
    } else {
      values[Vmeas_idx][xLL2_idx] = dyLL1_dVmeas;
      values[xLL1_idx][xLL2_idx]  = dyLL1_dxLL1;
      values[xLL2_idx][xLL2_idx]  = -1.0  + dyLL1_dxLL2;
      values[Va_idx][xLL2_idx]    = dyLL1_dVa;
      values[xf_idx][xLL2_idx]    = dyLL1_dxf;

      dyLL2_dxLL2 = 1.0;
    }

    // Partial derivatives of Va equation
    if(iseq_diff[3]) {
      values[Va_idx][Va_idx] = 1.0;
    } else {
      values[Vmeas_idx][Va_idx] = Ka*dyLL2_dVmeas;
      values[xLL1_idx][Va_idx]  = Ka*dyLL2_dxLL1;
      values[xLL2_idx][Va_idx]  = Ka*dyLL2_dxLL2;
      values[Va_idx][Va_idx]    = -1.0 + Ka*dyLL2_dVa;
      values[xf_idx][Va_idx]    = Ka*dyLL2_dxf;
    }

    // Partial derivatives of xf equation
    values[xf_idx][xf_idx] = 1.0;
  } else {

    // Partial derivatives of Vmeas equation
    if(iseq_diff[0]) {
      values[Vmeas_idx][Vmeas_idx] = -1.0/Tr - shift;
      values[VD_idx][Vmeas_idx]    = dEc_dVD/Tr;
      values[VQ_idx][Vmeas_idx]    = dEc_dVQ/Tr;
    } else {
      values[Vmeas_idx][Vmeas_idx] = -1.0;
      values[VD_idx][Vmeas_idx] = dEc_dVD;
      values[VQ_idx][Vmeas_idx]  = dEc_dVQ;
    }

    // Partial derivatives of xLL1 equation
    if(iseq_diff[1]) {
      values[Vmeas_idx][xLL1_idx] = (1 - Tc/Tb)*-1.0/Tb;
      values[xLL1_idx][xLL1_idx]  = -1.0/Tb - shift;
      values[xf_idx][xLL1_idx]    = (1 - Tc/Tb)*-dVf_dxf/Tb;

      yLL1 = xLL1 + Tc/Tb*(Vref - Vmeas - Vf);
      dyLL1_dxLL1  = 1.0;
      dyLL1_dVmeas = -Tc/Tb;
      dyLL1_dxf    = -Tc/Tb*dVf_dxf;
    } else {
      values[Vmeas_idx][xLL1_idx] = -1.0;
      values[xLL1_idx][xLL1_idx]  = -1.0;
      values[xf_idx][xLL1_idx]    = -dVf_dxf;
      yLL1 = xLL1;
      dyLL1_dxLL1 = 1.0;
    }

    // Partial derivatives of xLL2 equation
    if(iseq_diff[2]) {
      values[Vmeas_idx][xLL2_idx] =  (1 - Tc1/Tb1)*dyLL1_dVmeas/Tb1;
      values[xLL1_idx][xLL2_idx]  =  (1 - Tc1/Tb1)*dyLL1_dxLL1/Tb1;
      values[xLL2_idx][xLL2_idx]  =  -1/Tb1 + (1 - Tc1/Tb1)*dyLL1_dxLL1/Tb1 - shift;
      values[Va_idx][xLL2_idx]    =  (1 - Tc1/Tb1)*dyLL1_dVa/Tb1;
      values[xf_idx][xLL2_idx]    =  (1 - Tc1/Tb1)*dyLL1_dxf/Tb1;

      yLL2 = xLL2 + Tc1/Tb1*yLL1;

      dyLL2_dVmeas = Tc1/Tb1*dyLL1_dVmeas;
      dyLL2_dxLL1  = Tc1/Tb1*dyLL1_dxLL1;
      dyLL2_dxLL2  = 1.0 + Tc1/Tb1*dyLL1_dxLL2;
      dyLL2_dVa    = Tc1/Tb1*dyLL1_dVa;
      dyLL2_dxf    = Tc1/Tb1*dyLL1_dxf;
    } else {
      values[Vmeas_idx][xLL2_idx] = dyLL1_dVmeas;
      values[xLL1_idx][xLL2_idx]  = dyLL1_dxLL1;
      values[xLL2_idx][xLL2_idx]  = -1.0  + dyLL1_dxLL2;
      values[Va_idx][xLL2_idx]    = dyLL1_dVa;
      values[xf_idx][xLL2_idx]    = dyLL1_dxf;

      dyLL2_dxLL2 = 1.0;
    }

    // Partial derivatives of Va equation
    if(iseq_diff[3]) {
      values[Vmeas_idx][Va_idx] = Ka*dyLL2_dVmeas/Ta;
      values[xLL1_idx][Va_idx]  = Ka*dyLL2_dxLL1/Ta;
      values[xLL2_idx][Va_idx]  = Ka*dyLL2_dxLL2/Ta;
      values[Va_idx][Va_idx]    = (-1.0 + Ka*dyLL2_dVa)/Ta - shift;
      values[xf_idx][Va_idx]    = Ka*dyLL2_dxf/Ta;
    } else {
      values[Vmeas_idx][Va_idx] = Ka*dyLL2_dVmeas;
      values[xLL1_idx][Va_idx]  = Ka*dyLL2_dxLL1;
      values[xLL2_idx][Va_idx]  = Ka*dyLL2_dxLL2;
      values[Va_idx][Va_idx]    = -1.0 + Ka*dyLL2_dVa;
      values[xf_idx][Va_idx]    = Ka*dyLL2_dxf;
    }

    // Partial derivatives of xf equation
    values[xf_idx][xf_idx] = -1.0/Tf - shift;
  }

  return true;
}


/**
 * Set the initial field voltage (at t = tstart) for the exciter
 * @param fldv value of the field voltage
 */
void Esst1aExc::setInitialFieldVoltage(double fldv)
{
  Efd = fldv;
}

bool Esst1aExc::getFieldVoltagePartialDerivatives(int *xexc_loc,double *dEfd_dxexc,double *dEfd_dxgen)
{
  int nxgen,i;

  xexc_loc[0] = offsetb;
  xexc_loc[1] = offsetb+1;
  xexc_loc[2] = offsetb+2;
  xexc_loc[3] = offsetb+3;
  xexc_loc[4] = offsetb+4;

  dEfd_dxexc[0] = 0.0;
  dEfd_dxexc[1] = 0.0;
  dEfd_dxexc[2] = 0.0;
  dEfd_dxexc[3] = 1.0;
  dEfd_dxexc[4] = 0.0;

  // Note: dEfd_dxgen is all zeros since Klr is assumed to be zero. This
  // should be updated when Klr is non-zero
  getGenerator()->vectorSize(&nxgen);

  for(i=0; i < nxgen; i++) dEfd_dxgen[i] = 0.0;

  return true;
}

/** 
 * Get the value of the field voltage parameter
 * @return value of field voltage
 */
double Esst1aExc::getFieldVoltage()
{
  double VT = sqrt(VD*VD + VQ*VQ);
  double Vmin = VT*Vrmin;
  double Vmax;
  double fdv;
  BaseGenModel* gen = getGenerator();
  LadIfd = gen->getFieldCurrent();
  Vmax = VT*Vrmax - Kc*LadIfd;
  
  Efd = Va - fmax(0,Klr*(LadIfd - Ilr)); // should be actually max(0,Klr*(LadIfd - Ilr));
  fdv = fmin(fmax(Efd,Vmin),Vmax);

  return fdv;
}
