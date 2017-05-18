#if !defined(_WIN32)
#define dgemm dgemm_
#endif

#include "mex.h"
#include "blas.h"
#include <math.h>

/*
% EVmergefuncm specialized indexed multiplication operation for use with EV functions
% USAGE
%   z=EVmergefuncm(x,xind,y,yind);
% INPUTS
%   x    : mx x nx matrix
%   xind : p-vector of uint32 values on [1,...,nx] (if empty nx must equal p)
%   y    : my x ny matrix
%   yind : p-vector of uint32 values on [1,...,ny] (if empty ny must equal p)
% OUTPUT
%   z    : mx/my x p matrix
%
% mx/my must be an integer
*/

bool indexcheck8(mwSize *indout, uint8_T *indexin, mwSize p, mwSize n){
bool okay=true;
mwSize i;
  for (i=0; i<p; i++){
    if (indexin[i]>n) {okay=false; break; }
    indout[i] = (mwSize) indexin[i]; 
  }
  return(okay);
}

bool indexcheck16(mwSize *indout, uint16_T *indexin, mwSize p, mwSize n){
bool okay=true;
mwSize i;
  for (i=0; i<p; i++){
    if (indexin[i]>n) {okay=false; break; }
    indout[i] = (mwSize) indexin[i]; 
  }
  return(okay);
}

bool indexcheck32(mwSize *indout, uint32_T *indexin, mwSize p, mwSize n){
bool okay=true;
mwSize i;
  for (i=0; i<p; i++){
    if (indexin[i]>n) {okay=false; break; }
    indout[i] = (mwSize) indexin[i]; 
  }
  return(okay);
}

bool indexcheck64(mwSize *indout, uint64_T *indexin, mwSize p, mwSize n){
bool okay=true;
mwSize i;
  for (i=0; i<p; i++){
    if (indexin[i]>n) {okay=false; break; }
    indout[i] = (mwSize) indexin[i]; 
  }
  return(okay);
}

bool indexcheckD(mwSize *indout, double *indexin, mwSize p, mwSize n){
bool okay=true;
mwSize i;
  for (i=0; i<p; i++){
    if (indexin[i]>n) {okay=false; break; }
    indout[i] = (mwSize) indexin[i]; 
  }
  return(okay);
}

void merge00(double *z, double *x, double *y, mwSize p, mwSize mx, mwSize my){
  size_t rx,cx, cy;
  mwSize i;  
  char *chn = "N";
    /* scalar values to use in dgemm */
  double one = 1.0, zero = 0.0;
  rx = mx/my;
  cy = 1;
  cx = my;
  for (i=0; i<p; i++){
     /* Pass arguments to Fortran by reference */
    dgemm(chn, chn, &rx, &cy, &cx, &one, x, &rx, y, &cx, &zero, z, &rx);
    x += mx;
    y += my;
    z += rx;
  }
}


void merge01(double *z, double *x, double *y, mwSize *yind,   
             mwSize p, mwSize mx, mwSize my){
  size_t rx,cx, cy;
  mwSize i; 
  char *chn = "N";
    /* scalar values to use in dgemm */
  double one = 1.0, zero = 0.0; 
  rx = mx/my;
  cy = 1;
  cx = my;
  y -= my;    // convert to 1-base indexing
  for (i=0; i<p; i++){
     /* Pass arguments to Fortran by reference */
    dgemm(chn, chn, &rx, &cy, &cx, &one, x, &rx, y+my*(mwSize)yind[i], &cx, &zero, z, &rx);
    x += mx;
    z += rx;
  }
}

void merge10(double *z, double *x, double *y, mwSize *xind,    
             mwSize p, mwSize mx, mwSize my){
  size_t rx,cx, cy;
  mwSize i;  
  char *chn = "N";
    /* scalar values to use in dgemm */
  double one = 1.0, zero = 0.0;
  rx = mx/my;
  cy = 1;
  cx = my;
  x -= mx;        // convert to 1-base indexing
  for (i=0; i<p; i++){
     /* Pass arguments to Fortran by reference */
    dgemm(chn, chn, &rx, &cy, &cx, &one, x+mx*(mwSize)xind[i], &rx, y, &cx, &zero, z, &rx);
    y += my;
    z += rx;
  }
}

void merge11(double *z, double *x, double *y, mwSize *xind, mwSize *yind, 
             mwSize p, mwSize mx, mwSize my){
  size_t rx, cx, cy;
  mwSize i;  
  char *chn = "N";
    /* scalar values to use in dgemm */
  double one = 1.0, zero = 0.0;
  rx = mx/my;
  cy = 1;
  cx = my;
  x -= mx; y -= my;    // convert to 1-base indexing
  for (i=0; i<p; i++){
     /* Pass arguments to Fortran by reference */
    dgemm(chn, chn, &rx, &cy, &cx, &one, x+mx*(mwSize)xind[i], &rx, y+my*(mwSize)yind[i], &cx, &zero, z, &rx);
    z += rx;
  }
}

// sparse versions
void merge00s(double *z, double *x, double *y, 
             mwSize p, mwSize mx, mwSize my, mwSize *Iry, mwSize *Jcy){
  double *xj, *xji, *yj, yji;
  mwSize i, j, k, ij, nc, rx, *Iryj, Jcyij;  
  rx = mx/my;
  for (j=0; j<p; ){
    xj = x + mx*j;
    Jcyij=Jcy[j++];
    nc=Jcy[j]-Jcyij;
    yj = y + Jcyij;
    Iryj = Iry + Jcyij;
    for (i=0; i<nc; i++) {
      yji=yj[i];
      ij = Iryj[i];
      xji = xj + rx*ij;
      for (k=0; k<rx; k++) z[k] += xji[k] * yji;
    }
    z += rx;
  }
}

void merge10s(double *z, double *x, double *y, mwSize *xind,  
             mwSize p, mwSize mx, mwSize my, mwSize *Iry, mwSize *Jcy){
  double *xj, *xji, *yj, yji;
  mwSize i, j, k, ij, nc, rx, *Iryj, Jcyij;  
  rx = mx/my;
  x -= mx;  // convert to 1-base indexing
  for (j=0; j<p; ){
    xj = x + mx*xind[j];
    Jcyij=Jcy[j++];
    nc=Jcy[j]-Jcyij;
    yj = y + Jcyij;
    Iryj = Iry + Jcyij;
    for (i=0; i<nc; i++) {
      yji=yj[i];
      ij = Iryj[i];
      xji = xj + rx*ij;
      for (k=0; k<rx; k++) z[k] += xji[k] * yji;
    }
    z += rx;
  }
}

void merge01s(double *z, double *x, double *y, mwSize *yind, 
             mwSize p, mwSize mx, mwSize my, mwSize *Iry, mwSize *Jcy){
  double *xj, *xji, *yj, yji;
  mwSize i, j, k, ij, nc, rx, *Iryj, Jcyij;  
  rx = mx/my;
  Jcy -= 1; // convert to 1-base indexing
  for (j=0; j<p; j++){
    xj = x + mx*j;
    ij = yind[j];
    Jcyij=Jcy[ij++];
    nc=Jcy[ij]-Jcyij;
    yj = y + Jcyij;
    Iryj = Iry + Jcyij;
    for (i=0; i<nc; i++) {
      yji=yj[i];
      ij = Iryj[i];
      xji = xj + rx*ij;
      for (k=0; k<rx; k++) z[k] += xji[k] * yji;
    }
    z += rx;
  }
}

void merge11s(double *z, double *x, double *y, mwSize *xind, mwSize *yind, 
             mwSize p, mwSize mx, mwSize my, mwSize *Iry, mwSize *Jcy){
  double *xj, *xji, *yj, yji;
  mwSize i, j, k, ij, nc, rx, *Iryj, Jcyij;  
  rx = mx/my;
  x -= mx; Jcy -= 1; // convert to 1-base indexing
  for (j=0; j<p; j++){
    xj = x + mx*xind[j];
    ij = yind[j];
    Jcyij=Jcy[ij++];
    nc=Jcy[ij]-Jcyij;
    yj = y + Jcyij;
    Iryj = Iry + Jcyij;
    for (i=0; i<nc; i++) {
      yji=yj[i];
      ij = Iryj[i];
      xji = xj + rx*ij;
      for (k=0; k<rx; k++) z[k] += xji[k] * yji;
    }
    z += rx;
  }
}


void mexFunction(
   int nlhs, mxArray *plhs[],
   int nrhs, const mxArray *prhs[])
{
  double  *x, *y, *z;
  void *xy;
  mwSize *xind, *yind, *Iry, *Jcy;
  mwSize p, px, py, mx, my, nx, ny, i;
  bool okay=true, ysparse;
  
  /* Error checking on inputs */  
  if (nrhs>4 || nrhs<4) mexErrMsgTxt("Invalid number of input arguments");
  if (!mxIsDouble(prhs[0]))
      mexErrMsgTxt("Input 1 must be double");  
  if (!mxIsDouble(prhs[2]))
      mexErrMsgTxt("Input 3 must be double");  
  for (i=0; i<nrhs; i++){
    if (mxIsSparse(prhs[i]) && i!=2)
      mexErrMsgTxt("Inputs cannot be sparse");
    if (mxIsComplex(prhs[i]))
      mexErrMsgTxt("Inputs cannot be complex");
  }
  if (mxIsSparse(prhs[2])) ysparse=true; else ysparse=false;
 
  mx=mxGetM(prhs[0]);
  my=mxGetM(prhs[2]);
  
  p=mx/my;
  if (p*my != mx)
    mexErrMsgTxt("rows numbers of x and y are incompatible");
  
  nx=mxGetN(prhs[0]);
  ny=mxGetN(prhs[2]);

  px=mxGetNumberOfElements(prhs[1]);
  py=mxGetNumberOfElements(prhs[3]);
  if (px>0 && py>0 && px!=py)
      mexErrMsgTxt("Index vectors must have the same number of elements");
  if (px==0) p=nx; else p=px;
  if (py==0 && ny!=p)
      mexErrMsgTxt("y has an incorrect # of columns");
  if (py>0 && py!=p)
      mexErrMsgTxt("yind has an incorrect # of elements");


  if (px>0){  
    xind = mxMalloc(p*sizeof(mwSize));
    xy   = mxGetData(prhs[1]);
    switch (mxGetClassID(prhs[1])) {
      case mxUINT8_CLASS:  okay=indexcheck8(xind, xy, p, nx);  break; 
      case mxUINT16_CLASS: okay=indexcheck16(xind, xy, p, nx);  break; 
      case mxUINT32_CLASS: okay=indexcheck32(xind, xy, p, nx);  break; 
      case mxUINT64_CLASS: okay=indexcheck64(xind, xy, p, nx);  break; 
      case mxDOUBLE_CLASS: okay=indexcheckD(xind, xy, p, nx);  break; 
      default:
        mexErrMsgTxt("xind is an improper data type - it must be an unsigned integer type");
    }
  }
  if (!okay)
     mexErrMsgTxt("xind has invalid values");

  if (py>0){  
    yind = mxMalloc(p*sizeof(mwSize));
    xy   = mxGetData(prhs[3]);
    switch (mxGetClassID(prhs[3])) {
      case mxUINT8_CLASS:  okay=indexcheck8(yind, xy, p, ny);  break; 
      case mxUINT16_CLASS: okay=indexcheck16(yind, xy, p, ny); break; 
      case mxUINT32_CLASS: okay=indexcheck32(yind, xy, p, ny); break; 
      case mxUINT64_CLASS: okay=indexcheck64(yind, xy, p, ny); break; 
      case mxDOUBLE_CLASS: okay=indexcheckD(yind, xy, p, ny);  break; 
      default:
        mexErrMsgTxt("yind is an improper data type - it must be an unsigned integer type");
    }
  }
  if (!okay)
     mexErrMsgTxt("yind has invalid values");


  x    = mxGetPr(prhs[0]);
  y    = mxGetPr(prhs[2]);

  
  plhs[0]=mxCreateDoubleMatrix(mx/my,p, mxREAL);
  z=mxGetPr(plhs[0]);

  //plhs[0]=mxCreateUninitNumericMatrix(mx/my,p,mxDOUBLE_CLASS, mxREAL);
  //z=mxGetData(plhs[0]);
  
  
  if (ysparse) {
    Iry = mxGetIr(prhs[2]);
    Jcy = mxGetJc(prhs[2]);
    if (px>0)
      if (py>0) merge11s(z,x,y,xind,yind,p,mx,my,Iry,Jcy);
      else      merge10s(z,x,y,xind,p,mx,my,Iry,Jcy);  
    else
      if (py>0) merge01s(z,x,y,yind,p,mx,my,Iry,Jcy); 
      else      merge00s(z,x,y,p,mx,my,Iry,Jcy);
  }
  else {
    if (px>0)
      if (py>0) merge11(z,x,y,xind,yind,p,mx,my);
      else      merge10(z,x,y,xind,p,mx,my);  
    else
      if (py>0) merge01(z,x,y,yind,p,mx,my); 
      else      merge00(z,x,y,p,mx,my);
  }
  if (px>0) mxFree(xind);
  if (py>0) mxFree(yind);
}

