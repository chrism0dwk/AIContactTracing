/* ./src/utils/R2_calc/epiCovars.cpp
 *
 * Copyright 2012 Chris Jewell <chrism0dwk@gmail.com>
 *
 * This file is part of InFER.
 *
 * InFER is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * InFER is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InFER.  If not, see <http://www.gnu.org/licenses/>. 
 */

/* sinrEpi class methods for SINR Epidemic */

#include <string.h>

#include "epiCovars.h"



EpiCovars::EpiCovars() {
}

EpiCovars::~EpiCovars() {
}

int EpiCovars::freqInit(char *filename) {

  ifstream datafile;
  char line[200];
  char element[10];
  char *line_ptr;
  char *element_ptr;
  frequencies freqRow;

  cFreq.clear();

  datafile.open(filename,ios::in);
  if(!datafile.is_open()) {
    cerr << "Cannot open frequency file" << endl;
    return(-1);
  }

  while(1) {

    datafile.getline(line,200);
    if(datafile.eof()) break;
    line_ptr = line;
    element_ptr = strchr(line_ptr,' '); *element_ptr = '\0'; strcpy(element,line_ptr);
    freqRow.fm = (freq_t)atof(element);
    line_ptr = element_ptr+1;
    element_ptr = strchr(line_ptr,' '); *element_ptr = '\0'; strcpy(element,line_ptr);
    freqRow.fm_N = atof(element);
    line_ptr = element_ptr + 1;
    element_ptr = strchr(line_ptr,' '); *element_ptr = '\0'; strcpy(element,line_ptr);
    freqRow.sh = (freq_t)atof(element);
    line_ptr = element_ptr + 1;
    strcpy(element,line_ptr);
    freqRow.sh_N = atof(element);
    cFreq.push_back(freqRow);
  }

  datafile.close();

  return(0);
}


int EpiCovars::init(int myN_total, const char *contactPrefix, const char *distFile) {

  char contactFilename[200];

  N_total = myN_total;

  /* Set up the contact matrix */
  sprintf(contactFilename,"%s.fm",contactPrefix);
  rv = fm_Mat.init(contactFilename,N_total);
  if(rv != 0) {
    cerr << "Feed Mill Contact Matrix could not be initialised!" << endl;
    return(-1);
  }

  sprintf(contactFilename,"%s.sh",contactPrefix);
  rv = sh_Mat.init(contactFilename,N_total);
  if(rv != 0) {
    cerr << "SH Contact Matrix could not be initialised!" << endl;
    return(-1);
  }

  sprintf(contactFilename,"%s.cp",contactPrefix);
  rv = cp_Mat.init(contactFilename,N_total);
  if(rv != 0) {
    cerr << "Company Contact Matrix could not be initialised!" << endl;
    return(-1);
  }

  sprintf(contactFilename,"%s.freq",contactPrefix);
  rv = freqInit(contactFilename);
  if(rv != 0) {
    cerr << "Frequency table could not be loaded!" << endl;
    return(-1);
  }

  cerr << "Contact matrices initialised!" << endl;

  sprintf(contactFilename,"%s.sp",contactPrefix);
  rv = species.initialize(contactFilename,N_total,9);
  if(rv !=0) {
    cerr << "Species table could not be initialized!" << endl;
    return(-1);
  }



  /* Set up the distance matrix */

  rho = new (nothrow) float[N_total*N_total];
  if (rho == 0) {
    cerr << "Can't allocate rho!" << endl;
    return(-1);
  }
  rv = rhoInit(distFile,rho);
  if(rv != 0) {
    cerr << "Initialisation of rho failed!" << endl;
    return(-1);
  }

  return(0);

}

int EpiCovars::rhoInit(const char *filename,float *myRho) {

  ifstream datafile;
  int i,j;
  float dist;
  char line[200];
  char element[10];
  char *line_ptr;
  char *element_ptr;

  // Set all elements of rho to GSL_POS_INF:

  for(int h=0; h < N_total*N_total;++h) {
    *(myRho+h) = GSL_POSINF;
  }


  datafile.open(filename,ios::in);
  if(!datafile.is_open()) {
    cerr << "Cannot open distance file" << filename << endl;
    return(-1);
  }

  while(1) {

    datafile.getline(line,200);
    if(datafile.eof()) break;
    //line[strlen(line)-1] = '\0';
    line_ptr = line;
    element_ptr = strchr(line_ptr,' ');
    *element_ptr = '\0';
    strcpy(element,line_ptr);
    i = atoi(element);
    line_ptr = element_ptr+1;
    element_ptr = strchr(line_ptr,' ');
    *element_ptr = '\0';
    strcpy(element,line_ptr);
    j = atoi(element);
    line_ptr = element_ptr + 1;
    strcpy(element,line_ptr);
    dist = atof(element);
    *(myRho + i + N_total*j) = dist;
    //    cerr << i << " " << j << " " << dist << "\n";
  }

  datafile.close();

  return(0);
}



double EpiCovars::dist(const int& i, const int& j) {
  // Returns the distance between two farms
  return *(rho+i+N_total*j);
}
