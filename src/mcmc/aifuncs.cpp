/* ./src/mcmc/aifuncs.cpp
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

/* This is a function to compute A_1 */
#include <cassert>
#include <iostream>
#include <omp.h>
#include "aifuncs.h"
#include <vector>


// vector of vectors to hold non-zero connections

void initConnections(epiParms &parms, sinrEpi &epidata) {
  // Initialises a two-dimensional data structure
  // representing connected premises.
  
  int sparseCounter = 0;
  int i,j;
#pragma omp parallel for default(shared) private(i,j) reduction(+:sparseCounter)
  for(i=0; i<epidata.N_total; ++i) {
    for(j=0; j<epidata.N_total; ++j) {
      if(i!=j) {
	if(beta(parms,epidata,i,j) > 0.0) {
	  sparseCounter++;
	  epidata.individuals.at(i).connections.push_back(j);
	}
      }
    }
  }
}

      
    

/* Functions to compute A1 and A2 */

inline double spatialRate(epiParms &parms, sinrEpi &epidata, int i, int j)
{
  // Computes company network and spatial component of the model

  double beta(0.0);

  // Company
  beta += parms.beta[3] * epidata.cp_Mat.isConn(i,j);

  //Spatial
  beta += parms.beta[4] * exp(-parms.beta[6] * (*(epidata.rho+i+epidata.N_total*j) - 5));

  // Species susceptibility
  beta *= species(parms,epidata,i,j);

  return beta;
}



inline double networkRate(epiParms &parms, sinrEpi &epidata, int i, int j)
{
  // Computes the network component of the model

  double beta(0.0);
  ///////////NB: Multiplier is 10 for contact tracing egs! /////

  // Feed Mills
  beta = parms.beta[1] * 10 * epidata.fm_Mat.isConn(i,j) * ( 0.5 * epidata.cFreq[j].fm * ( 3 / (epidata.cFreq[j].fm_N) ) );

  // Slaughterhouse
  beta += parms.beta[2] * 10 * epidata.sh_Mat.isConn(i,j) * ( 0.5 * epidata.cFreq[j].sh * ( 3 / (epidata.cFreq[j].sh_N) ) );

  // Species susceptibility
  beta *= species(parms,epidata,i,j);

  return beta;
}


double fmRate(epiParms &parms, sinrEpi &epidata, int i, int j)
{
  double beta = parms.beta[1] * 10 * epidata.fm_Mat.isConn(i,j) * ( 0.5 * epidata.cFreq[j].fm * ( 3 / (epidata.cFreq[j].fm_N) ) );
  return beta *= species(parms,epidata,i,j);
}



double species(epiParms &parms, sinrEpi &epidata, int i, int j)
{
  double beta(1.0);

  // Species susceptibility
  for(size_t k=7;k<parms.p;++k) {
    if(epidata.species.at(j,k-7) == 1) {
      beta = parms.beta[k];
      break;
    }
  }

  return beta;
}



double beta(epiParms &parms, sinrEpi &epidata, int i, int j) 
{

  // Here i and j are LABELS, NOT indices!

  return spatialRate(parms,epidata,i,j) + networkRate(parms,epidata,i,j);
}



double betastar(epiParms &parms, sinrEpi &epidata, int i, int j) 
{
  // Returns the value of \beta^\star.  Notice the beta[5] cf speciesRate!

  double beta(0.0);

  beta = parms.beta[5] * exp(-parms.beta[6] * (*(epidata.rho+i+epidata.N_total*j) - 5));

  // Species susceptibility
  beta *= species(parms,epidata,i,j);

  return beta;
}



inline double binomComponent(epiParms &parms, infection* s, double t)
{
  // Returns the binomial component of the contact for s up to
  // time t (the infection time)

  infection* myIndiv = 0;
  vector<const Contact*> nonInfecContacts;
  vector<const Contact*>::iterator niIter;
  double answer = 1.0;
  bool isInfecByContact = false;

#ifdef CONTACT_DEBUG
  cout << "Label: " << s->label << endl;
#endif


  // First for Feedmill

  if(s->isInfecByWhoAt(FEEDMILL,t,myIndiv)) {
    isInfecByContact = true;
    answer *= parms.beta[1] * hFunc(parms,t - myIndiv->I);
#ifdef CONTACT_DEBUG
    cout << "FM infec: 1" << endl;
#endif
  }
  else {
#ifdef CONTACT_DEBUG
    cout << "FM infec: 0" << endl;
#endif
  }

  nonInfecContacts = s->getContactsByUntil(FEEDMILL,t); // Get list of non-infec contacts
#ifdef CONTACT_DEBUG
  cout << "FM non infec: " << nonInfecContacts.size() << endl;
#endif
  niIter = nonInfecContacts.begin();

  while( niIter != nonInfecContacts.end() ) {
    double myBeta = parms.beta[1] * hFunc(parms,(*niIter)->time - (*niIter)->source->I);
    answer *= 1 - myBeta;
    niIter++;
  }



  // Now for Slaughterhouse

  if(s->isInfecByWhoAt(SHOUSE,t,myIndiv)) {
    isInfecByContact = true;
    answer *= parms.beta[2] * hFunc(parms,t - myIndiv->I);
#ifdef CONTACT_DEBUG
    cout << "SH infec: 1" << endl;
#endif
  }
  else {
#ifdef CONTACT_DEBUG
    cout << "SH infec: 0" << endl;
#endif
  }
 

  nonInfecContacts = s->getContactsByUntil(SHOUSE,t);
#ifdef CONTACT_DEBUG
  cout << "SH non infec: " << nonInfecContacts.size() << endl;
#endif
  niIter = nonInfecContacts.begin();
  
  while( niIter != nonInfecContacts.end() ) {
    double myBeta = parms.beta[2] * hFunc(parms,(*niIter)->time - (*niIter)->source->I);
    answer *= 1 - myBeta;
    niIter++;
  }
  

  return answer;
}



void drawBetaCan(const int& nParms, const double mu[], gsl_matrix* const sdMat, double mvNormRV[], const int addOffset)
{
  // Takes an input vector of parameter, mu, and 
  // draws proposals for either a multiplicative, 
  // or part additive multi-site random walk
    
  double* betaTemp = new double[nParms];

  for(int k=0;k<addOffset;++k) {
    betaTemp[k] = log(mu[k]);
  }
  for(int k=addOffset;k<nParms;++k) {
    betaTemp[k] = mu[k];
  }

  rmnorm(nParms,betaTemp,sdMat,mvNormRV);
  
  for(int k=0;k<addOffset;++k) {
    mvNormRV[k] = exp(mvNormRV[k]);
  }
}



double occultProposal_pdf(const double time, const double& a, const double& b)
{
  return truncNorm_pdf(time, -1 / b, 1/(a*b*b) );
}



double occultProposal(const double& a, const double& b)
{
  return truncNorm( -1/b, 1/(a*b*b) );
}



double hFunc(epiParms &parms, double t)
{

  assert(t>=0);
  double exponent = exp(parms.g*t);
  return exponent / (parms.f + exponent);
}



inline double infecInteg(epiParms &parms, double t) 
{
  //assert(t >= 0);
  if ( t < 0.0 ) {
    cout << "WARNING: t < 0.0 in " << __PRETTY_FUNCTION__ << endl;
  }

  return 1/parms.g * log( (parms.f+exp(parms.g*t)) / (parms.f+1) );
}




/////////////////////////////////////////////////////////////////////////
// Compute functions
/////////////////////////////////////////////////////////////////////////

double compute_bgPress(epiParms &parms, sinrEpi &epidata)
{
  double result = 0;
  /* \beta_0 * ( \sum_i (I_i) +  n_sT - (N-1)I1 ) */
  
  result += parms.beta[0] * (epidata.sumI() + epidata.susceptible.size() * (ObsTime) - (epidata.N_total-1) * epidata.infected[epidata.I1]->I);

  return result;
}



double compute_A1(epiParms &parms, sinrEpi &epidata) {

  double result = 0.0;
  int i,j;
  size_t iLabel,jLabel;
  int num_infectives = epidata.infected.size();
  int num_susceptibles = epidata.susceptible.size();
  vector<size_t>::iterator jIter;

#pragma omp parallel for default(shared) private(i,j,iLabel,jLabel,jIter) schedule(static)  reduction(+:result)

  for(i=0; i<num_infectives; ++i) {

    iLabel = epidata.infected.at(i)->label;
    jIter = epidata.infected.at(i)->connections.begin();

    while(jIter != epidata.infected.at(i)->connections.end()) {

      jLabel = *jIter;
      
      // Infectious pressure on infectives
      if(epidata.individuals.at(jLabel).status == INFECTED) {
	result += spatialRate(parms,epidata,iLabel,jLabel) * infecInteg(parms,epidata.exposureI(iLabel,jLabel));
	result += networkRate(parms,epidata,iLabel,jLabel) * infecInteg(parms,epidata.exposureIBeforeCT(iLabel,jLabel));
      }

      // Infectious pressure on susceptibles
      else {
	result += spatialRate(parms,epidata,iLabel,jLabel) *  infecInteg(parms,epidata.ITime(iLabel));
	result += networkRate(parms,epidata,iLabel,jLabel) * infecInteg(parms,epidata.ITimeBeforeCT(iLabel));
      }

      jIter++;
    }
  }

  return result;

} /* end of function : compute_A1 */



double compute_A2(epiParms &parms, sinrEpi &epidata) {

  double result = 0.0;
  int i,j;
  size_t iLabel,jLabel;
  int num_infectives = epidata.infected.size();
  int num_susceptibles = epidata.susceptible.size();
  vector<size_t>::iterator jIter;

#pragma omp parallel for default(shared) private(i,j,iLabel,jLabel,jIter) schedule(static) reduction(+:result)

  for (i=0; i<num_infectives; ++i) {

    iLabel = epidata.infected.at(i)->label;
    jIter = epidata.infected.at(i)->connections.begin();

    while(jIter != epidata.infected.at(i)->connections.end()) {


      jLabel = *jIter;

      if(epidata.individuals.at(jLabel).status == INFECTED) {
        /* this is the first part  */
        result = result + betastar(parms,epidata,iLabel,jLabel) * epidata.exposureN(iLabel,jLabel);
      }

      else {
        /* the second part */
        result = result + betastar(parms,epidata,iLabel,jLabel) * epidata.NTime(iLabel);
      }

      jIter++;

    }

  }

  return result;

}



/* this is a function to evaluate infectious pressure that individual i gets */

double compute_log_prod_pressure(epiParms &parms, sinrEpi &epidata,vector<double> *product_Curr) {

  // Calculate the instantaneous infectious pressure on all i's *from* all j's.

  int i,j;
  size_t iLabel,jLabel;
  double Ii,Ij,Ni,Ri;
  //bool iHasContacts;
  double sum_over_j = 0.0;
  double result = 0.0;
  int num_infectives = epidata.infected.size();

#pragma omp parallel for default(shared) private(i,j,iLabel,Ii,jLabel,Ij,Ni,Ri,sum_over_j) schedule(static) reduction(+:result)
  for (j=0; j<num_infectives; ++j) {
    
    if ( j != epidata.I1) {

      jLabel = epidata.infected.at(j)->label;
      Ij = epidata.infected.at(j)->I;

      sum_over_j = 0.0;

      if( epidata.infected[j]->isInfecByContact() ) {
	sum_over_j = 1.0;
      }
      else {

	for(i=0; i<num_infectives; ++i) {
	
	  if ( i!=j ) {

	    iLabel = epidata.infected[i]->label;
	    Ii = epidata.infected[i]->I;
	    Ni = epidata.infected[i]->N;
	    Ri = epidata.infected[i]->R;
	    
	    if (Ii < Ij && Ij <= Ni) {
	
              sum_over_j += spatialRate(parms,epidata,iLabel,jLabel) * hFunc(parms,Ij - Ii);

	      if(!epidata.infected[j]->infecInCTWindow() && !epidata.infected[i]->inCTWindowAt(Ij)) {
		sum_over_j += networkRate(parms,epidata,iLabel,jLabel) * hFunc(parms,Ij - Ii);
	      }

	    }
	    else if (Ni < Ij && Ij <= Ri) {
	      sum_over_j += betastar(parms,epidata,iLabel,jLabel);
	    }
	  }
	}
      
	sum_over_j += parms.beta[0];
      }


      product_Curr->at(j) = sum_over_j;
      result = result + log(sum_over_j);
    }
  }

  product_Curr->at(epidata.I1) = 1.0;  // Fill in for I1
  return result; 
}



double computeLogCT(epiParms& parms, sinrEpi& epidata)
{
  // Computes the binomial portion of the likelihood
  // for contact tracing data


  double prod(0.0);
  int j;
  infection* indiv;

#pragma omp parallel for default(shared) private(j,indiv) schedule(static) reduction(+:prod)
  for(j=0; j < epidata.individuals.size(); ++j) {
    indiv = &(epidata.individuals[j]);
    if(indiv->I == epidata.infected[epidata.I1]->I) continue;
    prod += log(binomComponent(parms,indiv,indiv->I));
  }

  return prod;
}



//////////////////////////////////////////////////////////////////////////////
// Update functions
//////////////////////////////////////////////////////////////////////////////
double update_log_prod(int &move_index, epiParms &parms, 
		       sinrEpi &epidata, double &log_prod, 
		       vector<double> *prodCurr_vec, 
		       vector<double> *prodCan_vec) 
{
  double log_prod_can = log_prod;
  double Icurr = epidata.infected[move_index]->I;
  double Ij;
  int j,i;
  int I1can = epidata.I1;  // Necessary to allow us to adjust I1 if needs be
  int num_infectives = epidata.infected.size();

 

  double row_sum = 0.0;  // Hold the new value of \sum^nI_j(beta_ij) for the proposal

  if( move_index == epidata.I1 ) {  // Tests for a new I1
    int I2 = epidata.I2();   // and if we have, we set our local I1 to I2.
    if ( parms.Ican > epidata.infected[I2]->I ) I1can = I2;
//    cout << "Moving I1" << endl;
  }
  else if( parms.Ican < epidata.infected[epidata.I1]->I ) {
    I1can = move_index;
  }

  // First recalculate the product row for our proposed individual
  bool iCanInCTWindow = epidata.infected[move_index]->inCTWindowAt(parms.Ican);
  bool iCurrInCTWindow = epidata.infected[move_index]->infecInCTWindow();

  if( move_index != I1can  && !epidata.infected[move_index]->isInfecContactAt(parms.Ican)) {

    #pragma omp parallel for default(shared) private(i) schedule(static) reduction(+:row_sum)
    for (i=0; i<num_infectives; ++i) {

      if ( i==move_index ) continue;
      
      if (epidata.infected[i]->I < parms.Ican && parms.Ican <= epidata.infected[i]->N) {
	  row_sum += spatialRate(parms,epidata,epidata.infected.at(i)->label,epidata.infected.at(move_index)->label)
                     * hFunc(parms,parms.Ican - epidata.infected[i]->I);

   	  if ( !iCanInCTWindow && !epidata.infected[i]->inCTWindowAt(parms.Ican) ) {
	    row_sum += networkRate(parms,epidata,epidata.infected.at(i)->label,epidata.infected.at(move_index)->label)
	               * hFunc(parms,parms.Ican - epidata.infected[i]->I);
	  }
      }
      
      else if (epidata.infected[i]->N < parms.Ican && parms.Ican <= epidata.infected[i]->R) {
	row_sum += betastar(parms,epidata,epidata.infected.at(i)->label,epidata.infected.at(move_index)->label);
      }

    }
    row_sum += parms.beta[0]; // Don't forget to add the omnipresent \beta_0
  }
  else row_sum = 1.0;

  prodCan_vec->at(move_index) = row_sum;

  log_prod_can = log_prod_can - log(prodCurr_vec->at(move_index)) + log(prodCan_vec->at(move_index));



  // Now update all other \sum^nI_j(beta_ij) based on this proposed move
  double log_prod_alter = 0.0;

#pragma omp parallel for default(shared) private(Ij,j) schedule(dynamic) reduction(+:log_prod_alter)
  for(j=0; j < num_infectives; ++j) {

    if(j==move_index) continue;  // Already done move_index
    else if(epidata.infected[j]->isInfecByContact()) { // Nothing changes if j is infected by a contact
      if( prodCurr_vec->at(j) != 1.0 ) {
	cerr << "prodCurr_vec->at(" << j << ") = " << prodCurr_vec->at(j) << endl;
	throw logic_error("Inconsistency in product vector!");
	}
      prodCan_vec->at(j) = 1.0;
      continue;
    }
    else {
	if (prodCurr_vec->at(j) == 1.0 && j != epidata.I1) {
		cout << "prodCurr_vec->at(" << j << ") = " << prodCurr_vec->at(j) << " and is not infected by contact" << endl;
	}
    }

    prodCan_vec->at(j) = prodCurr_vec->at(j); // Copy across before modifying.
    Ij = epidata.infected[j]->I;

    // First subtract pressure if needs be
    if ( j == I1can ) prodCan_vec->at(j) = 1.0;
    else if ( Icurr < Ij && Ij < epidata.infected[move_index]->N ) {
	prodCan_vec->at(j) -= spatialRate(parms,epidata,epidata.infected[move_index]->label,epidata.infected[j]->label) * hFunc(parms,Ij - Icurr);
	
	if(!epidata.infected[move_index]->inCTWindowAt(Ij) && !epidata.infected[j]->infecInCTWindow()) {
		prodCan_vec->at(j) -= networkRate(parms,epidata,epidata.infected[move_index]->label,epidata.infected[j]->label) * hFunc(parms,Ij - Icurr);
	}
    }

    // Now add pressure if needs be
    if(parms.Ican < Ij && Ij < epidata.infected[move_index]->N) {

        // If j is the old I1, meaning that we're proposing a new I1, we calculate pressure on j:
        if(j == epidata.I1) {
	   prodCan_vec->at(j) = parms.beta[0]; // Add beta_0
	}
	
	// Add non-network pressure
	prodCan_vec->at(j) += spatialRate(parms,epidata,epidata.infected[move_index]->label,epidata.infected[j]->label) * hFunc(parms,Ij - parms.Ican);

	// Add network pressure if Ij is not in a contact window
	if(!epidata.infected[move_index]->inCTWindowAt(Ij) && !epidata.infected[j]->infecInCTWindow()) {
		prodCan_vec->at(j) += networkRate(parms,epidata,epidata.infected[move_index]->label,epidata.infected[j]->label) * hFunc(parms,Ij - parms.Ican);
	}
    }


    // Update log_prod_alter
    if(prodCan_vec->at(j) != prodCurr_vec->at(j) ) {
	log_prod_alter += log(prodCan_vec->at(j)) - log(prodCurr_vec->at(j));
    }

  }

  log_prod_can += log_prod_alter;

  return(log_prod_can);
}

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////IN PROGRESS///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
double update_logCT(int& move_index,
		   epiParms& parms,
		   sinrEpi& epidata,
		   double& logCTcurr)
{
  // Updates the contact tracing binomial portion of the likelihood

  infection* s = epidata.infected.at(move_index);
  double Icurr = s->I;
  double logCTcan = 0.0;

  int j;
  int numInfectives = epidata.infected.size();
  infection* indiv;

  //#pragma omp parallel for default(shared) private(j,indiv) schedule(static) reduction(+:logCTcan)
  for(j=0; j < epidata.individuals.size(); ++j) {

    indiv = &(epidata.individuals[j]);

    // Epidemic integrity check - checks for infectious contacts that have been abandoned
    infection* mySource;
    if(indiv->isInfecByContact(mySource)) {
      if(mySource->label == s->label && parms.Ican > indiv->I) {
	return GSL_NEGINF;
      }
    }

    s->I = parms.Ican; // Swap in the proposal

    logCTcan += log(binomComponent(parms,indiv,indiv->I));

    s->I = Icurr; // Swap back the old time
  }

//   for(j = 0; j < epidata.susceptible.size(); ++j) {
// 	logCTcan += log(binomComponent(parms,epidata.susceptible[j],epidata.susceptible[j]->I));
//   }

  return logCTcan;
}



double update_bgPress(int &move_index,
		      epiParms &parms,
		      sinrEpi &epidata,
		      double &bg_press) 
{
  // update_bg_press returns a candidate partial liklihood for bg_press

  double result = bg_press;
  Ipos_t myI2;

  if(move_index != epidata.I1) // If our proposal is not I1
    
    if(parms.Ican > epidata.infected[epidata.I1]->I) {  // If proposal does not affect I1
      
      result -= parms.beta[0] * epidata.infected[move_index]->I;
      result += parms.beta[0] * parms.Ican;
    }
    else{  // The proposal becomes I1
      result += parms.beta[0] * (epidata.N_total - 1) * epidata.infected[epidata.I1]->I; // subtract -(N-1)I_{1}
      result -= parms.beta[0] * (epidata.N_total - 1) * parms.Ican; // Subtract (N-1)I_{move_index}^{can}
      result -= parms.beta[0] * epidata.infected[move_index]->I; // Subtract \beta0 * I_{move}^{cur} 
      result += parms.beta[0] * epidata.infected[epidata.I1]->I; // I1 now has a time within which it was susceptible
    }
  
  
  else {  // The moved infection time belongs to I1

    myI2 = epidata.I2();
    
    if(parms.Ican > epidata.infected[myI2]->I) { // If our propsal means that I1 is no longer index case
      //cout << "MOVING I1 in bgPress to after I2" << endl;
      //cout << "I2: " << epidata.infected[myI2].label << " (" << myI2 << "), time=" << epidata.infected[myI2].I << endl;
      result += parms.beta[0] * (epidata.N_total - 1) * epidata.infected[move_index]->I; // Add (N-1)I_1
      result -= parms.beta[0] * (epidata.N_total - 1) * epidata.infected[myI2]->I; // Subtract (N-1)I_{2}
      result -= parms.beta[0] * epidata.infected[myI2]->I;  // I2 is now I1 so was never susceptible
      result += parms.beta[0] * parms.Ican;  // I1 now has a susceptible period so add beta_0 pressure for that.
    }
    
    else {  // I1 moves, but stays as the index case
      //cout << "MOVING I1 in bgPress to before I2" << endl;
      //cout << "I2: " << epidata.infected[myI2].label << " (" << myI2 << "), time=" << epidata.infected[myI2].I << endl;
      result += parms.beta[0] * (epidata.N_total - 1) * epidata.infected[move_index]->I; // Subtract for the old I1
      result -= parms.beta[0] * (epidata.N_total - 1) * parms.Ican; // Add for the new I1
    }
  }

  return result;
}



double update_A1(int &move_index,
		 epiParms &parms,
		 sinrEpi &epidata,
		 double A1)
{
  // update_A1 returns a candidate partial likelihood for A1_can

  int num_infectives = epidata.infected.size();
  int num_susceptibles = epidata.susceptible.size();
  int i,j;
  size_t iLabel,jLabel;
  double part_integral = 0.0;
  double jStop = 0.0;
  double jStop_can = 0.0;
  vector<size_t>& connections = epidata.infected.at(move_index)->connections;

  /* First calculate the new pressure for our chosen individual on all others in the population */

  int num_connections = connections.size();
  iLabel = epidata.infected.at(move_index)->label;

  #pragma omp parallel for default(shared) private(j,jStop,jLabel) schedule(static) reduction(+:part_integral)
  for(j=0; j < num_connections; ++j) { //j's getting infected by move_index here.

    if(j==iLabel) continue;

    jLabel = connections.at(j);

    if(epidata.individuals.at(jLabel).status == INFECTED) {  // Pressure onto the infectives

      jStop = GSL_MIN(epidata.individuals.at(jLabel).I,epidata.individuals.at(jLabel).contactStart);
      jStop = GSL_MIN(jStop,epidata.individuals[iLabel].contactStart);
      
      part_integral += spatialRate(parms,epidata,iLabel,jLabel) * 
	(
	 infecInteg(parms,GSL_MIN(epidata.infected[move_index]->N,epidata.individuals[jLabel].I) - GSL_MIN(epidata.individuals[jLabel].I,parms.Ican)) - 
	 infecInteg(parms,GSL_MIN(epidata.infected[move_index]->N,epidata.individuals[jLabel].I) - GSL_MIN(epidata.individuals[jLabel].I,epidata.infected[move_index]->I))
	 );
      part_integral += networkRate(parms,epidata,iLabel,jLabel) *
	(
	 infecInteg(parms,GSL_MIN(epidata.infected[move_index]->N,jStop) - GSL_MIN(jStop,parms.Ican)) - 
	 infecInteg(parms,GSL_MIN(epidata.infected[move_index]->N,jStop) - GSL_MIN(jStop,epidata.infected[move_index]->I))
	 );
    }
    else {  // Pressure onto the susceptibles
      part_integral += beta(parms,epidata,iLabel,jLabel) *
	(
	 infecInteg(parms, epidata.infected.at(move_index)->N - parms.Ican) -  
	 infecInteg(parms, epidata.infected.at(move_index)->N - epidata.infected.at(move_index)->I)
	 );
    }
  }
  A1 += part_integral;



  /* Now add the integral for all others */

  part_integral = 0.0;

  double move_indexStop = GSL_MIN(epidata.infected.at(move_index)->I,epidata.infected.at(move_index)->contactStart);
  double move_indexStopCan = GSL_MIN(parms.Ican,epidata.infected.at(move_index)->contactStart);

  #pragma omp parallel for default(shared) private(i,jStop,jStop_can) schedule(static) reduction(+:part_integral)
  for(i=0; i < num_infectives; ++i) { // Now move_index is getting infected by i.

    if(i==move_index) continue;

    jStop = GSL_MIN(move_indexStop,epidata.infected[i]->contactStart);
    jStop_can = GSL_MIN(move_indexStopCan,epidata.infected[i]->contactStart);

    part_integral += spatialRate(parms,epidata,epidata.infected.at(i)->label,epidata.infected.at(move_index)->label) * 
      (
       infecInteg(parms, GSL_MIN(epidata.infected[i]->N, parms.Ican) - GSL_MIN(parms.Ican, epidata.infected[i]->I))  -  
       infecInteg(parms, GSL_MIN(epidata.infected[i]->N, epidata.infected[move_index]->I) - GSL_MIN(epidata.infected[move_index]->I, epidata.infected[i]->I))
       );
    part_integral += networkRate(parms,epidata,epidata.infected.at(i)->label,epidata.infected.at(move_index)->label) * 
      (
       infecInteg(parms, GSL_MIN(epidata.infected[i]->N, jStop_can) - GSL_MIN(jStop_can, epidata.infected[i]->I))  -  
       infecInteg(parms, GSL_MIN(epidata.infected[i]->N, jStop) - GSL_MIN(jStop, epidata.infected[i]->I))
       );

  }

  A1 += part_integral;



  return(A1);
}



/* update_A2 returns a candidate partial likelihood for A2_can */

double update_A2(int &move_index, epiParms &parms, sinrEpi &epidata, double &A2) {

  int num_infectives = epidata.infected.size();
  int i;
  double part_integral = 0.0;

  #pragma omp parallel for default(shared) private(i) schedule(static) reduction(+:part_integral)
  for(i=0; i < num_infectives; ++i) {
    if(i==move_index) continue;
    part_integral += betastar(parms,epidata,epidata.infected.at(i)->label,epidata.infected.at(move_index)->label) * 
      (
       GSL_MIN(epidata.infected[i]->R,parms.Ican) - GSL_MIN(parms.Ican,epidata.infected[i]->N) - 
       GSL_MIN(epidata.infected[i]->R,epidata.infected[move_index]->I) + GSL_MIN(epidata.infected[move_index]->I,epidata.infected[i]->N)
       );
  }

  return(A2 + part_integral);
}



/* FUNCTIONS FOR ADDING AN INFECTION */

double addInfec_log_prod(epiParms &parms, sinrEpi &epidata, double &log_prod, vector<double> *prodCurr_vec, vector<double> *prodCan_vec) 
{
  double log_prod_can = log_prod;
  double log_prod_alter = 0;
  double Ij;
  int myI1 = epidata.I1;  // Necessary to allow us to adjust I1 if needs be
  int add_index = epidata.infected.size()-1;  // Our index for the new infection is the last entry in the infected vector
  int loopSize;
  int j;
 

 /* First recalculate the product row for our proposed individual */
  double row_sum = 0.0;  // Hold the new value of \sum^nI_j(beta_ij) for the proposal

  if(epidata.infected[add_index]->I > epidata.infected[myI1]->I) {

    loopSize = epidata.infected.size()-1;
#pragma omp parallel for default(shared) private(j) schedule(static) reduction(+:row_sum)
    for (j=0; j<loopSize; ++j) {  // Note that our proposal is the last in the infected vector, hence the -1.

      if (epidata.infected[j]->I < epidata.infected[add_index]->I && epidata.infected[add_index]->I <= epidata.infected[j]->N) {
	
	  row_sum += spatialRate(parms,epidata,epidata.infected[j]->label,epidata.infected[add_index]->label)
                   * hFunc(parms,epidata.exposureI(epidata.infected.at(j)->label,epidata.infected.at(add_index)->label));

	  if(!epidata.infected[j]->inCTWindowAt(epidata.infected[add_index]->I)) {
	    row_sum += networkRate(parms,epidata,epidata.infected[j]->label,epidata.infected[add_index]->label)
	           * hFunc(parms,epidata.exposureI(epidata.infected[j]->label,epidata.infected[add_index]->label));
	  }
      }
      
      else if (epidata.infected[j]->N < epidata.infected[add_index]->I && epidata.infected[add_index]->I <= epidata.infected[j]->R) {	
	row_sum += betastar(parms,epidata,epidata.infected.at(j)->label,epidata.infected.at(add_index)->label);
      }
    }
  row_sum += parms.beta[0]; // Don't forget to add \beta_0 !
  }
  else {
    row_sum = 1;
    cerr << "Illegal addition!" << endl;
    throw logic_error("Illegal addition");
  }

  prodCan_vec->push_back(row_sum);

  log_prod_can = log_prod_can + log(prodCan_vec->back());  // Put the new value into the product




  /* Now update all other \sum^nI_j(beta_ij) based on this proposed move */
  loopSize = epidata.infected.size()-1;
#pragma omp parallel for default(shared) private(j,Ij) schedule(static) reduction(+:log_prod_alter)
  for(j=0; j < loopSize; ++j) {    // Note that our proposal is the last in the infected vector, hence the -1.

    if(epidata.infected[j]->isInfecByContact()) {
      assert(prodCurr_vec->at(j) == 1.0);
      prodCan_vec->at(j) = prodCurr_vec->at(j);
      continue;
    }

    Ij = epidata.infected[j]->I;

    if (epidata.infected[add_index]->I < Ij) {

	prodCan_vec->at(j) = prodCurr_vec->at(j) + spatialRate(parms,epidata,epidata.infected.at(add_index)->label,epidata.infected.at(j)->label)
	                                         * hFunc(parms,Ij - epidata.infected[add_index]->I);
      
      if(!epidata.infected[j]->infecInCTWindow()) {
	prodCan_vec->at(j) += networkRate(parms,epidata,epidata.infected.at(add_index)->label,epidata.infected.at(j)->label)
                                                 * hFunc(parms,Ij - epidata.infected[add_index]->I);
      }

      if(j==epidata.I1) {
	prodCan_vec->at(j) -= 1; // If our proposal is before current I1, we subtract 1 from the entry in the candidate vector
	prodCan_vec->at(j) += parms.beta[0];
	assert(prodCan_vec->at(j) != 0);
	cout << "ILLEGAL ADDITION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
      }
      log_prod_alter = log_prod_alter - log(prodCurr_vec->at(j)) + log(prodCan_vec->at(j));
    }
    else {
      prodCan_vec->at(j) = prodCurr_vec->at(j);
    }
  }

  log_prod_can += log_prod_alter;
  return(log_prod_can);
}



double addInfec_bgPress(epiParms &parms, sinrEpi &epidata, double bgPress) 
{
  int add_index = epidata.infected.size()-1;

  bgPress -= parms.beta[0] * ObsTime; // Subtract a $\beta_0 * T$
   
  if(epidata.infected[add_index]->I > epidata.infected[epidata.I1]->I) {
    bgPress += parms.beta[0] * epidata.infected[add_index]->I;  // Add the new infection time
  }
  else{
    cout << "Addition before I1 is illegal!" << endl;
    cout << "I1 = " << epidata.infected[epidata.I1]->I << endl;
    cout << "Add Time = " << epidata.infected[add_index]->I << endl;
    return(-1);
  }

  return bgPress;
}



double addInfec_logCT(epiParms& parms,
		   sinrEpi& epidata,
		   double logCTcurr)
{
  // Returns the binomial portion of the likelihood
  //  - easy, as the new infection is added before this
  //    function is called :-)
  double logCTcan;

  logCTcan = computeLogCT(parms,epidata);

  return logCTcan;
}



double addInfec_A1(epiParms &parms, sinrEpi &epidata, double A1) 
{
  // addInfec_A1 returns a candidate partial likelihood for A1_can if an infection time is added
  // THIS FUNCTION ASSUMES THAT WE HAVE ALREADY ALTERED THE EPIDEMIC

  /* This is a 4 part algorithm to update A1 if an infection time is added */
  int add_index = epidata.infected.size()-1;  // The position of our new infection is the last in the infected vector
  int loopSize;
  double partPressure;
  int i,j;
  size_t iLabel,jLabel;
  vector<size_t>& connections = epidata.infected.at(add_index)->connections;

  /* First calculate the added pressure from the new infection (i) on other infectives (j)*/

  partPressure=0.0;
  loopSize = connections.size();
  iLabel = epidata.infected.at(add_index)->label;

#pragma omp parallel for default(shared) private(j,jLabel) schedule(static) reduction(+:partPressure)
  for (j=0; j<loopSize; ++j) {

    jLabel = connections.at(j);

    if(iLabel == jLabel) continue; // Continue since we can't have pressure on ourselves

    if(epidata.individuals.at(jLabel).status == INFECTED) { // Add pressure on infectives
      partPressure += spatialRate(parms,epidata,iLabel,jLabel) * infecInteg(parms, epidata.exposureI(iLabel,jLabel));
      partPressure += networkRate(parms,epidata,iLabel,jLabel) * infecInteg(parms, epidata.exposureIBeforeCT(iLabel,jLabel));
    }
    else { // Add pressure on susceptibles - NB. Because i is the addition, it has no contact tracing window.
      partPressure += beta(parms,epidata,iLabel,jLabel) * infecInteg(parms, epidata.ITime(iLabel));
    }
  }
  A1 += partPressure;

  // Now the pressure from the other infectives (i) on our addition (j):

  partPressure = 0.0;
  loopSize = epidata.infected.size()-1;
#pragma omp parallel for default(shared) private(i) schedule(static) reduction(+:partPressure)
  for(i=0; i<loopSize; ++i) { // We ignore the last infective - this is our new one!

    //calculate the pressure from the other infectives on our new infective
    partPressure += spatialRate(parms,epidata,epidata.infected[i]->label,epidata.infected[add_index]->label) * infecInteg(parms,epidata.exposureI(epidata.infected[i]->label,epidata.infected[add_index]->label));
    partPressure += networkRate(parms,epidata,epidata.infected[i]->label,epidata.infected[add_index]->label) * infecInteg(parms,epidata.exposureIBeforeCT(epidata.infected[i]->label,epidata.infected[add_index]->label));

    //subtract the pressure from the other infectives on our new infective IF we were still susceptible
    partPressure -= spatialRate(parms,epidata,epidata.infected[i]->label,epidata.infected[add_index]->label) * infecInteg(parms,epidata.ITime(epidata.infected[i]->label));
    partPressure -= networkRate(parms,epidata,epidata.infected[i]->label,epidata.infected[add_index]->label) * infecInteg(parms,epidata.ITimeBeforeCT(epidata.infected[i]->label));

  }

  A1 += partPressure;

  return(A1);
}

double addInfec_A2(epiParms &parms, sinrEpi &epidata, double A2) {

  /* This is a 2 part algorithm to update A2 if an infection has been added */
  int add_index = epidata.infected.size()-1; // Position of our new infection is the last in the infected vector
  int loopSize;
  double partPressure;
  int i;

  partPressure = 0.0;
  loopSize = epidata.infected.size()-1;
#pragma omp parallel for default(shared) private(i) schedule(static) reduction(+:partPressure)
  for(i=0; i<loopSize; ++i) {

    //calculate the added pressure on the new infection from individuals in the notified period
    partPressure += betastar(parms,epidata,epidata.infected.at(i)->label,epidata.infected.at(add_index)->label) * epidata.exposureN(epidata.infected.at(i)->label,epidata.infected.at(add_index)->label);

    // Secondly subtract the pressure from the other infecteds from when our new infective was susceptible
    partPressure -= betastar(parms,epidata,epidata.infected.at(i)->label,epidata.infected.at(add_index)->label) * epidata.NTime(epidata.infected.at(i)->label);
  }

  A2 += partPressure;
  return(A2);
}


/* FUNCTIONS FOR DELETING AN INFECTION */
/* THESE FUNCTIONS ASSUME THAT THE REMOVAL PROPOSAL HAS NOT YET BEEN REMOVED FROM THE EPIDEMIC - THIS IS OPPOSITE TO THE ADDING FUNCTIONS */

double delInfec_log_prod(int &remove_index, epiParms &parms, sinrEpi &epidata, double &log_prod, vector<double> *prodCurr_vec, vector<double> *prodCan_vec) {

  double log_prod_can = log_prod;
  double Icurr = epidata.infected[remove_index]->I;
  int myI1 = epidata.I1;  // Necessary to allow us to adjust I1 if needs be

  if(remove_index == epidata.I1) {  // Tests if we've moved I1
    cout << "DELETING I1!" << endl;
    myI1 = epidata.I2();   // and if we have, we set our local I1 to I2.
    prodCan_vec->at(myI1) = 1; // Update the vector for I1
    log_prod_can = log_prod_can - log(prodCurr_vec->at(myI1)); // Update the log_prod for I1
  }


  /* First erase the product row for our proposed individual */

  prodCan_vec->erase(prodCan_vec->begin() + remove_index);  // Erase the entry from the candidate vector
  log_prod_can = log_prod_can - log(prodCurr_vec->at(remove_index)); // Subtract the the row for our removee from the log product (which is log(1) if we're removing I1, by the way)


  /* Now iterate through the current product vector and subtract elements where i is infected by the removee */
  unsigned int Can_count = 0;  // Iterates through out prodCan_vec.

  for(unsigned int i=0; i < epidata.infected.size(); ++i) {

    if(i==remove_index ) continue;  // Exclude remove_index and myI1 - we've already done these above.
    else if(i==myI1) {
      ++Can_count;
      continue;
    }
    else if(epidata.infected[i]->isInfecByContact()) { // Infected by contact, irrelevant to this bit of likelihood
      assert(prodCurr_vec->at(i) == 1.0);
      prodCan_vec->at(Can_count) == prodCurr_vec->at(i);
      ++Can_count;
      continue;
    }

    if(Icurr < epidata.infected[i]->I) {

       prodCan_vec->at(Can_count) = prodCurr_vec->at(i) - spatialRate(parms,epidata,epidata.infected.at(remove_index)->label,epidata.infected.at(i)->label)
	                                                 * hFunc(parms,epidata.infected[i]->I - Icurr);

      if ( !epidata.infected[i]->inCTWindowAt(Icurr) ) {
	prodCan_vec->at(Can_count) -= networkRate(parms,epidata,epidata.infected.at(remove_index)->label,epidata.infected.at(i)->label)
                                      * hFunc(parms,epidata.infected[i]->I - Icurr);
      }

      log_prod_can = log_prod_can - log(prodCurr_vec->at(i)) + log(prodCan_vec->at(Can_count));
    }
    else {
      prodCan_vec->at(Can_count) = prodCurr_vec->at(i);
    }
    // DEBUG
    if(prodCan_vec->at(Can_count) < 0.0) {
      double oldVal = prodCurr_vec->at(i);
      double newVal = prodCan_vec->at(Can_count);
      cout.precision(16);
      cout << fixed << "Warning!! diff = " << oldVal - newVal << "\n"
	   << "Old Val = " << oldVal << ", new val = " << newVal << endl;
    }
    assert(fabs(prodCan_vec->at(Can_count)) >= 0.0);
    ++Can_count;
  }
  return(log_prod_can);
}


/* delInfec_bgPress returns a candidate partial likelihood for bgPress if an infection time is delete */

double delInfec_bgPress(int &remove_index, epiParms &parms, sinrEpi &epidata, double bgPress) {

  if(remove_index != epidata.I1) {
    bgPress -= parms.beta[0] * epidata.infected[remove_index]->I;
    bgPress += parms.beta[0] * ObsTime;
  }
  else{
    Ipos_t myI2 = epidata.I2();
    bgPress += parms.beta[0] * (epidata.N_total - 1) * epidata.infected[epidata.I1]->I;
    bgPress -= parms.beta[0] * (epidata.N_total - 1) * epidata.infected[myI2]->I;
    bgPress += parms.beta[0] * ObsTime;
  }


  return bgPress;
}



double delInfec_logCT(int& del_index,
		      epiParms& parms,
		      sinrEpi& epidata,
		      double logCTcurr)
{
  // Computes the binomial portion of the likelihood
  // for deleted infection.

  double prod(0.0);
  int j;
  int numInfectives = epidata.infected.size();
  double Icurr;
  infection* mySource;

  // Look for invalid links first:
  for(j=0; j < numInfectives; ++j) {
    if(j == del_index) continue; // Skip the deletee
    else if(epidata.infected[j]->isInfecByContact(mySource)) {
      if(mySource->label == epidata.infected.at(del_index)->label) {
	return GSL_NEGINF;
      }
    }
  }

  // A bit hacky, but we set del_index's I = N to prevent it from infecting anyone by CT
  Icurr = epidata.infected[del_index]->I;
  epidata.infected[del_index]->I = epidata.infected[del_index]->N;


  prod = computeLogCT(parms,epidata);

// #pragma omp parallel for default(shared) private(j) schedule(static) reduction(+:prod)
//    for(j=0; j < numInfectives; ++j) {
//      if(j == del_index) continue; // Skip the deletee
// 
//      prod += log(binomComponent(parms,epidata.infected[j],epidata.infected[j]->I));    
// 
//    }

   // Set del_index's I back to normal
   epidata.infected[del_index]->I = Icurr;

  return prod;
}



/* delInfec_A1 returns a candidate partial likelihood for A1_can if an infection time is deleted */

double delInfec_A1(int &remove_index, epiParms &parms, sinrEpi &epidata, double A1) {

  double partPressure;
  int loopSize;
  int i,j;
  size_t iLabel,jLabel;
  vector<size_t>& connections = epidata.infected.at(remove_index)->connections;

  /* First subtract the pressure from the removee (i) on other infectives (j) */
  
  partPressure = 0.0;
  loopSize = connections.size();
  iLabel = epidata.infected.at(remove_index)->label;

#pragma omp parallel for default(shared) private(j,jLabel) schedule(static) reduction(+:partPressure)
  for (j=0; j<loopSize; ++j) {

    jLabel = connections.at(j);

    if(jLabel==iLabel) continue;

    if(epidata.individuals.at(jLabel).status == INFECTED) { // Subtract pressure on infectives
      partPressure -= spatialRate(parms,epidata,iLabel,jLabel) * infecInteg(parms, epidata.exposureI(iLabel,jLabel));
      partPressure -= networkRate(parms,epidata,iLabel,jLabel) * infecInteg(parms, epidata.exposureIBeforeCT(iLabel,jLabel));
    }
    else { // Subtract pressure on susceptibles. NB. Because i is occult, it has no contact tracing window.
      partPressure -= beta(parms,epidata,iLabel,jLabel) * infecInteg(parms, epidata.ITime(iLabel));
    }
  }
  A1 += partPressure;

  // Delete pressure from other infectives (i) on removee (j):

  partPressure = 0.0;
  loopSize = epidata.infected.size();
#pragma omp parallel for default(shared) private(i) schedule(static) reduction(+:partPressure)
  for(i=0; i<loopSize; ++i) {
    if(i==remove_index) continue;

    // subtract the pressure from the other infectives on the removee */
    partPressure -= spatialRate(parms,epidata,epidata.infected[i]->label,epidata.infected[remove_index]->label) * infecInteg(parms, epidata.exposureI(epidata.infected[i]->label,epidata.infected[remove_index]->label));
    partPressure -= networkRate(parms,epidata,epidata.infected[i]->label,epidata.infected[remove_index]->label) * infecInteg(parms, epidata.exposureIBeforeCT(epidata.infected[i]->label,epidata.infected[remove_index]->label));

    // add the pressure from the other infectives on the removee AS IF it were susceptible */
    partPressure += spatialRate(parms,epidata,epidata.infected[i]->label,epidata.infected[remove_index]->label) * infecInteg(parms, epidata.ITime(epidata.infected[i]->label));
    partPressure += networkRate(parms,epidata,epidata.infected[i]->label,epidata.infected[remove_index]->label) * infecInteg(parms, epidata.ITimeBeforeCT(epidata.infected[i]->label));
  }
  A1 += partPressure;

  return(A1);
}




/* delInfec_A2 returns a candidate partial likelihood for A2_can if an infections time is deleted */

double delInfec_A2(int &remove_index, epiParms &parms, sinrEpi &epidata, double A2) {

  /* This is a 2 part algorithm to update A2 if an infection is deleted */
  double partPressure;
  int loopSize;
  int i;

  partPressure = 0.0;
  loopSize = epidata.infected.size();
#pragma omp parallel for default(shared) private(i) schedule(static) reduction(+:partPressure)
  for(i=0;i<loopSize;++i) {
    if(i == remove_index) continue;

    // subtract the pressure on the removee from the other notifieds */
    partPressure -= betastar(parms,epidata,epidata.infected.at(i)->label,epidata.infected.at(remove_index)->label) * epidata.exposureN(epidata.infected.at(i)->label,epidata.infected.at(remove_index)->label);

    // add the pressure from the notifieds on our removee AS IF it were a susceptible */
    partPressure += betastar(parms,epidata,epidata.infected.at(i)->label,epidata.infected.at(remove_index)->label) * epidata.NTime(epidata.infected.at(i)->label);
  }

  A2 += partPressure;

  return(A2);
}




//////////////////////////////////////////////////////
// DIAGNOSTICS FUNCTIONS 
//////////////////////////////////////////////////////

void checkProdVec(vector<double> *prodVec,epiParms& parms) {

  vector<double>::iterator vecIter = prodVec->begin();
  vector<int> ones;
  int counter = 0;

  while(vecIter != prodVec->end()) {
    //cout << "At " << counter << ": ";


    if(*vecIter == 1) {
      ones.push_back(1);
    }
    else if(*vecIter == 0) {
      cout << "ERROR: product vector contains a 0!" << endl;
    }
    else if(*vecIter < 0) {
      cout << "ERROR: negative value in product vector!" << endl;
    }
    else if(*vecIter > 0 && *vecIter < parms.beta[0] - 1e-9) {
      cout << "ERROR: 0 < product vector < beta[0] at position " << counter << endl;
      cout << endl;
    }
    counter++;
    vecIter++;
  }

//  if(ones.size() > 1) {
//    cout << "ERROR: More than 1 unitary value in product vector!" << endl;
//  }
  if(ones.empty()) {
    cout << "ERROR: No instance of 1 in product vector!" << endl;
  }
}



double getContactStart(sinrEpi& epidata, size_t label)
{
  //! Returns the contact tracing start point of label

  return epidata.individuals.at(label).contactStart;
}



void dumpContacts(sinrEpi& epidata, size_t label)
{
  //! Dumps a table of contacts with time, type, I, N, and isInfectious to
  //! standard out

  // Table header

  cout << "Label \t Type \t Time \t I \t N \t isInfectious \n\n";

  set<Contact>::iterator itContact = epidata.individuals.at(label).contacts.begin();
  
  while(itContact != epidata.individuals.at(label).contacts.end()) {
    
    cout << itContact->source->label << "\t"
	 << itContact->type << "\t"
	 << itContact->time << "\t"
	 << itContact->source->I << "\t"
	 << itContact->source->N << "\t"
	 << itContact->isInfectious() << "\n";

    itContact++;
  }

  cout << endl;
}



void dumpBinoms(epiParms& parms, sinrEpi& epidata)
{
  // Dumps all the binomial components to stdout

  double prod = 0.0;
  double myBinom;
  int j;
  int numInfectives = epidata.infected.size();
  infection* indiv;

  cout << "Label \t Binom \n";

#pragma omp parallel for default(shared) private(j,indiv) schedule(static) reduction(+:prod)
  for(j=0; j < numInfectives; ++j) {

    indiv = epidata.infected[j];
    myBinom = binomComponent(parms,indiv,indiv->I);
    cout << indiv->label << "\t" << myBinom << "\n";
    prod += log(myBinom);

  }

  cout << "Total: \t " << prod << endl;
}



size_t numInfecByCT(sinrEpi& epidata)
{
  // Returns the number of individuals infected by
  // a contact.

  size_t num = 0;

  for(int i=0; i<epidata.infected.size(); ++i) {
    if(epidata.infected[i]->isInfecByContact()) num++;
  }

  return num;
}



size_t numInfecContacts(sinrEpi& epidata)
{
  // Returns the total number of potentially
  // infectious contacts in the system

  size_t num = 0;
  vector<const Contact*> myContacts;

  for(int i=0; i<epidata.infected.size(); ++i) {
    myContacts = epidata.infected[i]->getInfecContacts();
    num += myContacts.size();
  }

  return num;
}
