/* ./src/mcmc/aiMCMC.cpp
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

#include "aiMCMC.h"

#define THINBY 1

int
main(int argc, char *argv[])
{

  /* Parse command line arguments, and initialise command line variables */

  if (argc <= 1)
    {
      cerr
          << "USAGE:\naiMCMC [-c <config file>] [-n <iterations>][-o <output file>][-s <random seed>][-t <Observation Time>]"
          << endl;
      exit(1);
    }

  // Set up config file
    {
      strcpy(config_filename, "aiMCMC.conf"); // Default value
      char** tempArgv = argv;

      for (int h = 0; h < argc; ++h)
        {
          if (tempArgv[h][0] == '-')
            {
              if (strcmp(tempArgv[h], "-c") == 0)
                {
                  strcpy(config_filename, tempArgv[h + 1]);
                  break;
                }
              else if (strcmp(argv[h], "-h") == 0)
                {
                  cout
                      << "USAGE:\naiMCMC [-c <config file>][-n <iterations>][-o <output file>]"
                      << endl;
                  return 0;
                }
            }
        }
    }

  cout << "Using config file: " << config_filename << endl;

  /* Initialise our global variables */

  ObsTime = GSL_POSINF; // DEFAULT OBSERVATION TIME SET TO INFINITY
  rv = inputConf();

  /* And then update anything from the config that we've overridden on the cmd line */

  ++argv; // Advance cmd line argument array past prog name
  --argc; // And subtract 1 from the number of cmd line arguments

  while ((argc >= 1))
    {

      if (argv[0][0] == '-')
        {

          switch (argv[0][1])
            { // Extensible switch statement if we need to add command line parameters
          case 'n':
            max_iter = atoi(argv[1]);
            argc -= 2;
            argv += 2;
            break;

          case 'o':
            strcpy(output_filename, argv[1]);
            argc -= 2;
            argv += 2;
            break;

          case 's':
            seed = atoi(argv[1]);
            argc -= 2;
            argv += 2;
            break;

          case 'b':
            block_update = true;
            --argc;
            ++argv;
            break;

          case 't':
            ObsTime = atof(argv[1]);
            --argc;
            ++argv;
            break;

          default:
            // Ignores all command line parameters not starting with '-'
            --argc;
            ++argv;
            break;
            }
        }
      else
        {
          // Advance if we've not found a switch
          --argc;
          ++argv;
        }

    }

  /* Read in epidemiological data */

  cout << "Reading epi data..." << endl;
  try
    {
      epidata.init(epidata.N_total, epidataFile, cMat_prefix, loc_filename, 9,
          ObsTime);
    }
  catch (exception& e)
    {
      cout << "Exception occurred initialising data from '" << epidataFile << "': " << e.what()
          << "\n Unsafe to continue!" << endl;
      exit(-1);
    }

  initConnections(parms, epidata);

  cout << "Read epi data.  Continuing..." << endl;

  /* Set up random number generator */

  rng = gsl_rng_alloc(gsl_rng_ranlux);
  gsl_rng_set(rng, seed);

  /* Evaluate I1 */

  epidata.updateI1();

  /* Write all parameter values to the screen to confirm before we start */

  cout << "Running aiMCMC with the following epidemic:\n";
  cout << "===========================================\n\n";
  cout << "Epidemic file: '" << epidataFile << "'\n";
  cout << "Distance file: '" << loc_filename << "'\n";
  cout << "Total Population Size: " << epidata.N_total << "\n";
  cout << "Number infected: " << epidata.infected.size() << "\n";
  cout << "Number non-infected: " << epidata.susceptible.size() << "\n";
  cout << "Inital infection: " << epidata.infected[epidata.I1]->label << "\n";
  cout << "RNG Seed: " << seed << "\n";
  cout << "=====================\n\n";
  cout << "Using parameter values:\n";
  cout << "=======================\n";

  for (int k = 0; k < parms.p; ++k)
    {
      printf("Beta%i: %f\n", k, parms.beta[k]);
    }
  for (int k = 0; k < parms.p; ++k)
    {
      printf("Sigma beta%i: %f\n", k, sigma_mult[k]);
    }
  for (int k = addOffset; k < parms.p; ++k)
    {
      printf("Sigma beta%i_add: %f\n", k, sigma_add[k]);
    }
  for (int k = 0; k < parms.p; ++k)
    {
      printf("Lambda beta%i: %f\n", k, priors.lambda[k]);
    }
  for (int k = 0; k < parms.p; ++k)
    {
      printf("Nu beta%i: %f\n", k, priors.nu[k]);
    }

  cout << "a: " << priors.a << "\n";
  cout << "b: " << priors.b << "\n";
  cout << "f: " << parms.f << "\n";
  cout << "g: " << parms.g << "\n";
  cout << "Observation Time: " << ObsTime << "\n";
  cout << "============\n\n";
  cout << "No iterations: " << max_iter << "\n";
  cout << "Output file: '" << output_filename << "'\n";
  cout << "Block update: " << block_update << "\n";
  cout << "I1 = " << epidata.I1 << endl;

  /* Now we run the model........................*/

  /* Set up MCMC variables */

  vector<double> prodCurr_vec(epidata.infected.size());
  vector<double> prodCan_vec(epidata.infected.size());
  vector<double> *prodCurr_ptr = &prodCurr_vec;
  vector<double> *prodCan_ptr = &prodCan_vec;
  double q, r;

  int accept[] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  double p;
  double log_piCurr, log_piCan;
  double log_prodCurr, log_prodCan;
  double loglikCurr, loglikCan;
  int move_index = 0;
  double inProp;
  double percent_done;
  double A1, A2, A1_can, A2_can, bgPress, bgPress_can;
  double logCT, logCT_can;
  double qRatio;
  ofstream resultsFile;

  parms_can = parms;
  parms_can.f = parms.f;
  parms_can.g = parms.g;

  McmcOutput multVariance(parms.p, 50, max_iter, sigma_mult);
  McmcOutput multaddVariance(parms.p, 50, max_iter, sigma_add);

  gsl_matrix* identityMatrix = gsl_matrix_alloc(parms.p, parms.p);
  gsl_matrix_set_identity(identityMatrix);
  gsl_matrix_scale(identityMatrix, 0.1 * 0.1 / parms.p);
  /////////////////////////////////////////////////////

  /* Open the output file */

  char outputFilenameFull[200];
  sprintf(outputFilenameFull, "%s.parms", output_filename);
  resultsFile.open(outputFilenameFull, ios::out); // Open our results file
  if (!resultsFile.is_open())
    {
      cout << "Cannot open " << output_filename << " for writing!" << endl;
      return (-1);
    }

  // Opens occult output file
  char occultFilename[200];
  sprintf(occultFilename, "%s.occ", output_filename);
  OccultWriter occultWriter;
  try
    {
      occultWriter.open(occultFilename);
    }
  catch (exception& e)
    {
      cerr << "Exception thrown opening state file\n" << "\tException: "
          << e.what() << endl;
      return (-1);
    }

  /* Compute the conditional posterior to start */

  log_prodCurr = compute_log_prod_pressure(parms, epidata, prodCurr_ptr);
  logCT = computeLogCT(parms, epidata);
  bgPress = compute_bgPress(parms, epidata);
  A1 = compute_A1(parms, epidata);
  A2 = compute_A2(parms, epidata);
  loglikCurr = log_prodCurr - bgPress - A1 - A2 + logCT;
  cout << "log_prodCurr: " << log_prodCurr << endl;
  cout << "bgPress: " << bgPress << endl;
  cout << "logCT: " << logCT << endl;
  cout << "A1: " << A1 << endl;
  cout << "A2: " << A2 << endl;
  cout << "loglikCurr: " << loglikCurr << endl;
  prodCan_vec = prodCurr_vec; // Copy the prodCurr vector into prodCan

  // ofstream debugOut;
  //   debugOut.open("/scratch/hpc/55/jewellc/debugOut.txt",ios::out);
  //   if(!debugOut.is_open()) {
  //     cout << "Cannot open debugOut.txt" << endl;
  //     return(-1);
  //   }


  //   for(int i=0; i<epidata.infected.size(); ++i) {
  //     cout << fixed;
  //     cout << i << " (" << epidata.infected.at(i)->label << "," << setprecision(6) << epidata.infected.at(i)->I
  // 	 << ") : " << binomComponent(parms,epidata.infected.at(i),epidata.infected.at(i)->I) << "\n";

  //   }

  ////////////////////////////////////////////////////
  // STOP HERE FOR NOW ///////////////////////////////
  //return 0;
  ////////////////////////////////////////////////////


  // Count number of infectious and non-infectious contacts:
  size_t fmNumNonInfec = 0;
  size_t shNumNonInfec = 0;
  size_t fmNumInfec = 0;
  size_t shNumInfec = 0;
  infection* dummy;
  vector<infection>::iterator iIter = epidata.individuals.begin();

  while (iIter != epidata.individuals.end())
    {
      fmNumNonInfec += iIter->numContactsByUntil(FEEDMILL, iIter->I);
      if (iIter->isInfecByWhoAt(FEEDMILL, iIter->I, dummy))
        fmNumInfec++;

      shNumNonInfec += iIter->numContactsByUntil(SHOUSE, iIter->I);
      if (iIter->isInfecByWhoAt(SHOUSE, iIter->I, dummy))
        shNumInfec++;

      iIter++;
    }
  cout << "\n\n=========================\n";
  cout << "\tinfec\tnon-infec\n\n";
  cout << "FM:\t" << fmNumInfec << "\t" << fmNumNonInfec << "\n";
  cout << "SH:\t" << shNumInfec << "\t" << shNumNonInfec << "\n\n" << flush;
  cout << "=========================\n";

  // Quick check:

  assert(epidata.infected.size()+epidata.susceptible.size() == epidata.individuals.size());

  /* MCMC LOOP BEGINS */

  time(&t_start);

  for (int h = 0; h < max_iter; ++h)
    {
      if (h % 100 == 0)
        {
          percent_done = (float) h / (float) max_iter * 100;
          cout << "\r" << percent_done << "% done" << "  Acceptance rates: "
              << (float) accept[0] / (float) h * 100 << " "
              << (float) accept[1] / (float) h * 100 << " "
              << (float) accept[2] / (float) h * 100 << " "
              << (float) accept[3] / (float) h * 100 << " "
              << (float) accept[4] / (float) h * 100 << " "
              << (float) accept[5] / (float) h * 100 << " "
              << (float) accept[6] / (float) h * 100 << "\n";
        }

#ifdef __DEBUG__
      if(h > 1)
      cout << "Iteration: " << h << "\n";
#endif

      /* Draw betas by multi-site MH */

      epidata.updateI1();

      //bgPress = compute_bgPress(parms,epidata);
      //loglikCurr = log_prodCurr - bgPress - A1 - A2;


      /* Draw betas by MH */

      log_piCurr = loglikCurr;

      // Conditional density:

      log_piCurr += log(gsl_ran_gamma_pdf(parms.beta[0], priors.lambda[0], 1.0
          / priors.nu[0]));
      log_piCurr += log(gsl_ran_beta_pdf(parms.beta[1], priors.lambda[1],
          priors.nu[1]));
      log_piCurr += log(gsl_ran_beta_pdf(parms.beta[2], priors.lambda[2],
          priors.nu[2]));

      for (int k = 3; k < parms.p; ++k)
        { // Calculate \pi(\beta)
          log_piCurr += log(gsl_ran_gamma_pdf(parms.beta[k], priors.lambda[k],
              1.0 / priors.nu[k]));
        }

      // Draw a U[0,1] to decide whether to use ARWM or MRWM for betas 4-16:
      q = gsl_rng_uniform(rng);
      r = gsl_rng_uniform(rng);

      // Proposals: with probability xi, sample with covar 0.1^2 * I / parms.p
      //            with probability 1 - xi sample with covar 2.38^2 * Var / parms.p

      if (r < xi)
        {
          if (q > a_m_ratio)
            drawBetaCan(parms.p, parms.beta, identityMatrix, parms_can.beta,
                parms.p);
          else
            drawBetaCan(parms.p, parms.beta, identityMatrix, parms_can.beta,
                addOffset);
        }
      else
        {
          if (q > a_m_ratio)
            drawBetaCan(parms.p, parms.beta, multVariance.scaleChol(2.38 * 2.38
                / parms.p), parms_can.beta, parms.p);
          else
            drawBetaCan(parms.p, parms.beta, multaddVariance.scaleChol(2.38
                * 2.38 / parms.p), parms_can.beta, addOffset);
        }

      if (!parms_can.isBetaNegative() && parms_can.beta[5] > 0.000
          && parms_can.beta[6] > 0.1 && parms_can.beta[4] >= parms_can.beta[5])
        { // Parameter constraints

          log_prodCan = compute_log_prod_pressure(parms_can, epidata,
              prodCan_ptr);
          logCT_can = computeLogCT(parms_can, epidata);
          bgPress_can = compute_bgPress(parms_can, epidata);
          A1_can = compute_A1(parms_can, epidata);
          A2_can = compute_A2(parms_can, epidata);
          loglikCan = log_prodCan - bgPress_can - A1_can - A2_can + logCT_can;

          log_piCan = loglikCan;

          log_piCan += log(gsl_ran_gamma_pdf(parms_can.beta[0],
              priors.lambda[0], 1.0 / priors.nu[0]));
          log_piCan += log(gsl_ran_beta_pdf(parms_can.beta[1],
              priors.lambda[1], priors.nu[1]));
          log_piCan += log(gsl_ran_beta_pdf(parms_can.beta[2],
              priors.lambda[2], priors.nu[2]));
          for (int k = 3; k < parms.p; ++k)
            {
              log_piCan += log(gsl_ran_gamma_pdf(parms_can.beta[k],
                  priors.lambda[k], 1.0 / priors.nu[k]));
            }

          p = gsl_rng_uniform(rng);

          double q_ratio = 0;

          for (int k = 0; k < addOffset; ++k)
            {
              q_ratio += log(parms_can.beta[k]);
              q_ratio -= log(parms.beta[k]);
            }

          for (int k = addOffset; k < parms.p; ++k)
            {
              if (q > a_m_ratio)
                {
                  q_ratio += log(parms_can.beta[k]);
                  q_ratio -= log(parms.beta[k]);
                }
            }

#ifdef __DEBUG__
          if(h > 1)
            {
              cout << "Betas: log_prodCurr=" << log_prodCurr
              << ", bgPress_curr=" << bgPress
              << ", logCT=" << logCT
              << ", A1_curr=" << A1
              << ", A2_curr=" << A2
              << ", log_piCurr=" << log_piCurr
              << "\n";

              cout << "Betas: log_prodCan=" << log_prodCan
              << ", bgPress=" << bgPress_can
              << ", logCT_can=" << logCT_can
              << ", A1_can=" << A1_can
              << ", A2_can=" << A2_can
              << ", log_piCan=" << log_piCan
              << "\n";
            }
#endif

          if (log(p) < log_piCan - log_piCurr + q_ratio)
            { // Do we accept or reject our proposal?
              parms = parms_can;
              log_prodCurr = log_prodCan;
              loglikCurr = loglikCan;
              bgPress = bgPress_can;
              logCT = logCT_can;
              A1 = A1_can;
              A2 = A2_can;
              //SWITCH_PROD_PTR;
              *prodCurr_ptr = *prodCan_ptr;
              accept[0] = accept[0] + 1;
            }
          else
            {
              parms_can = parms;
              *prodCan_ptr = *prodCurr_ptr;
            }
        }
      else
        {
          parms_can = parms;
          *prodCan_ptr = *prodCurr_ptr;
          //cout << "Condition false" << "\n";
        }

      /* Now we fiddle with the infections times :-) */

      for (int k = 0; k < infecFiddle; ++k)
        {
          epidata.updateI1();

          *prodCan_ptr = *prodCurr_ptr;
          float choose = gsl_ran_flat(rng, 0, 3);
          //float choose = 0.5;


#ifdef __DEBUG__
          if(h > 1)
            {
              cout << "FIDDLE---------> I1 = " << epidata.I1
              << ", label=" << epidata.infected[epidata.I1]->label
              << ", I=" << epidata.infected[epidata.I1]->I
              << "\n";

              // 	    cout << "FIDDLE---------> I1-1 = " << epidata.I1-1
              // 		 << ", label=" << epidata.infected[epidata.I1-1]->label
              // 		 << ", I=" << epidata.infected[epidata.I1-1]->I
              // 		 << "\n";
            }
#endif

//          cout << "I1: index = " << epidata.I1 << ", label = "
//              << epidata.infected.at(epidata.I1)->label << endl;

          if (choose < 1)
            { // MOVE INFECTIONS

              bool crossDim = false;
              bool isInfecByContact = false;
              vector<const Contact*> myContacts, myNewContacts;
              int state;

              move_index = gsl_rng_uniform_int(rng, epidata.infected.size()); // Choose I to move
              //if(move_index == 0) continue; // Ignore I1

              isInfecByContact
                  = epidata.infected[move_index]->isInfecByContact();
              myContacts = epidata.infected[move_index]->getInfecContacts();


              double (*proposal_func)(const double&,const double&);

              if (epidata.infected[move_index]->known && !epidata.infected[move_index]->isDC)
                { // Known infection
                  log_piCurr
                      = log(
                          rng_extreme_pdf(epidata.infected[move_index]->N
                              - epidata.infected[move_index]->I, priors.a,
                              priors.b)) + loglikCurr;
                  proposal_func = rng_extreme;
                }
              else
                {
                  log_piCurr = log(1 - rng_extreme_cdf(ObsTime
                      - epidata.infected[move_index]->I, priors.a, priors.b))
                      + loglikCurr;
                  proposal_func = occultProposal;
                }

              // Decide if we're going between CT infection time and Ext(a,b) infection time
              if (gsl_rng_uniform(rng) > 0.5)
                crossDim = true;

              if (isInfecByContact)
                { // Implies that move_index has contacts!
                  if (crossDim)
                    { // Contact -> frequency
                      // Propose from Extreme function
                      inProp = (*proposal_func)(priors.a, priors.b); // Choose a new I->N time
                      parms.Ican = epidata.infected[move_index]->N - inProp; // Set the new infection time
                      state = 1;
                    }
                  else
                    { // Contact -> Contact
                      // Propose a contact time from potentially infectious contacts
                      size_t conPos = gsl_rng_uniform_int(rng,
                          myContacts.size());
                      parms.Ican = myContacts[conPos]->time; // Set new I->N time
                      inProp = epidata.infected[move_index]->N - parms.Ican;
                      state = 2;
                    }
                }
              else
                {
                  if (crossDim)
                    { // Frequency -> Contact
                      // Propose a contact time
                      if (myContacts.empty())
                        continue; // Abort if we've not got any infectious contacts to move to.
                      size_t conPos = gsl_rng_uniform_int(rng,
                          myContacts.size());
                      parms.Ican = myContacts[conPos]->time; // Set new I->N time
                      inProp = epidata.infected[move_index]->N - parms.Ican;
                      state = 3;
                    }
                  else
                    { // Frequency -> Frequency
                      // Propose from Extreme function
                      inProp = (*proposal_func)(priors.a, priors.b); // Choose a new I->N time
                      parms.Ican = epidata.infected[move_index]->N - inProp; // Set the new infection time
                      state = 4;
                    }
                }

#ifdef __DEBUG__
              if(h > 1)
              cout << "MOVE " << epidata.infected[move_index]->label
              << "FROM " << epidata.infected[move_index]->I
              << " to " << parms.Ican
              << " (I1 = " << epidata.I1
              << "/" << epidata.infected[epidata.I1]->I << ")" << "\n";
#endif

              // Reject the proposal if we're proposing an infection time
              // before the time at which we know the individual was still susceptible.
              if (parms.Ican < epidata.individuals[move_index].niAt)
                continue;

              logCT_can = update_logCT(move_index, parms, epidata, logCT);

              if (logCT_can != GSL_NEGINF)
                { // If our proposal makes sense...

                  bgPress_can = update_bgPress(move_index, parms, epidata,
                      bgPress);
                  A1_can = update_A1(move_index, parms, epidata, A1);
                  A2_can = update_A2(move_index, parms, epidata, A2);
                  log_prodCan = update_log_prod(move_index, parms, epidata,
                      log_prodCurr, prodCurr_ptr, prodCan_ptr);
                  loglikCan = log_prodCan - bgPress_can - A1_can - A2_can
                      + logCT_can;

                  double
                  (*proposal_pdf)(double, const double&, const double&);

                  if (epidata.infected[move_index]->known && !epidata.infected[move_index]->isDC)
                    { // Known infection
                      proposal_pdf = rng_extreme_pdf;
                      log_piCan = log(rng_extreme_pdf(inProp, priors.a,
                          priors.b)) + loglikCan;
                    }
                  else
                    { // Occult infection
                      proposal_pdf = occultProposal_pdf;
                      log_piCan = loglikCan + log(1 - rng_extreme_cdf(inProp,
                          priors.a, priors.b));
                    }

                  // Calculate q-ratio

                  if (isInfecByContact)
                    {
                      if (crossDim)
                        { // Contact -> Frequency
                          if (state != 1)
                            throw logic_error("Wrong state!");
                          qRatio = log(1.0 / (double) myContacts.size()) - log(
                              (*proposal_pdf)(inProp, priors.a, priors.b));
                        }
                      else
                        { // Contact -> Contact
                          if (state != 2)
                            throw logic_error("Wrong state!");
                          qRatio = 0;
                        }
                    }
                  else
                    { // Frequency -> Contact
                      if (crossDim /*&& epidata.infected[move_index]->hasInfecContacts()*/)
                        {
                          if (state != 3)
                            throw logic_error("Wrong state!");
                          qRatio = log((*proposal_pdf)(
                              epidata.infected[move_index]->N
                                  - epidata.infected[move_index]->I, priors.a,
                              priors.b))
                              - log(1.0 / (double) myContacts.size());
                        }
                      else
                        { // Frequency -> Frequency
                          if (state != 4)
                            throw logic_error("Wrong state!");
                          qRatio = log((*proposal_pdf)(
                              epidata.infected[move_index]->N
                                  - epidata.infected[move_index]->I, priors.a,
                              priors.b)) - log((*proposal_pdf)(inProp,
                              priors.a, priors.b));
                        }
                    }

#ifdef __DEBUG__
                  if(h > 1)
                    {
                      cout << "Move: log_prodCurr=" << log_prodCurr
                      << ", bgPress_curr=" << bgPress
                      << ", logCT=" << logCT
                      << ", A1_curr=" << A1
                      << ", A2_curr=" << A2
                      << ", log_piCurr=" << log_piCurr
                      << "\n";
                      cout << "Move: log_prodCan=" << log_prodCan
                      << ", bgPress=" << bgPress_can
                      << ", logCT_can=" << logCT_can
                      << ", A1_can=" << A1_can
                      << ", A2_can=" << A2_can
                      << ", log_piCan=" << log_piCan
                      << "\n";
                    }
#endif

                  // 	      cout << "ARratio: " << crossDim << " " << isInfecByContact << " " << exp(log_piCan - log_piCurr + qRatio) << endl;
                  // 	      cout << "Lratio: " << crossDim << " " << isInfecByContact << " " << exp(log_piCan - log_piCurr) << endl;
                  // 	      cout << "Qratio: " << crossDim << " " << isInfecByContact << " " << exp(qRatio) << endl;

                  if (log(gsl_rng_uniform(rng)) < log_piCan - log_piCurr
                      + qRatio)
                    {
                      epidata.infected[move_index]->I = parms.Ican;
                      log_prodCurr = log_prodCan;
                      loglikCurr = loglikCan;
                      *prodCurr_ptr = *prodCan_ptr;
                      logCT = logCT_can;
                      bgPress = bgPress_can;
                      A1 = A1_can;
                      A2 = A2_can;
                      if (parms.Ican < epidata.infected[epidata.I1]->I)
                        {
                          epidata.I1 = move_index; // If we're the new I1, update I1
                        }
                      else if (move_index == epidata.I1)
                        { // If we've moved I1, find the new I1
                          epidata.updateI1();
                        }
                      ++accept[7];

//                      cout << "MOVED " << epidata.infected[move_index]->label
//                          << ", index = " << move_index << endl;

                    }
                  else
                    {
                      *prodCan_ptr = *prodCurr_ptr;
                    }
                }

              //}
            }

          else if (choose >= 1 && choose < 2)
            {

              /* Propose a new infection */

              move_index = gsl_rng_uniform_int(rng, epidata.susceptible.size());

              //#ifdef __DEBUG__
              //if(h > 1)
              //cout << "ADD " << epidata.susceptible[move_index]->label
              //<< "(" << move_index << ")" << "\n";
              //#endif

              inProp = ObsTime - truncNorm(-(1 / priors.b), 1 / (priors.a
                  * priors.b * priors.b));

              if (inProp > epidata.infected[epidata.I1]->I)
                {

                  epidata.addInfec(move_index, inProp, ObsTime, ObsTime);
                  log_piCurr = loglikCurr;

                  log_prodCan = addInfec_log_prod(parms, epidata, log_prodCurr,
                      prodCurr_ptr, prodCan_ptr);
                  bgPress_can = addInfec_bgPress(parms, epidata, bgPress);
                  logCT_can = addInfec_logCT(parms, epidata, logCT);
                  A1_can = addInfec_A1(parms, epidata, A1);
                  A2_can = addInfec_A2(parms, epidata, A2);

                  loglikCan = log_prodCan - bgPress_can - A1_can - A2_can
                      + logCT_can;
                  log_piCan = loglikCan + log(1 - rng_extreme_cdf(
                      epidata.infected.back()->N - epidata.infected.back()->I,
                      priors.a, priors.b));

#ifdef __DEBUG__
                  if(h > 1)
                    {
                      cout << "addIndex=" << move_index << ", inProp=" << inProp << ", logCDF="
                      << log(1 - rng_extreme_cdf(epidata.infected.back()->N - epidata.infected.back()->I,priors.a,priors.b))
                      << "\n";
                      cout << "Add: log_prodCurr=" << log_prodCurr
                      << ", bgPress_curr=" << bgPress
                      << ", logCTcurr=" << logCT << ", A1_curr="
                      << A1 << ", A2_curr=" << A2 << ", loglikCurr="
                      << loglikCurr << ", log_piCurr=" << log_piCurr << "\n";
                      cout << "Add: log_prodCan=" << log_prodCan << ", bgPress="
                      << bgPress_can << ", logCT_can=" << logCT_can << ", A1_can=" << A1_can << ", A2_can="
                      << A2_can << ", loglikCan=" << loglikCan << ", log_piCan=" << log_piCan << "\n";
                    }
#endif

                  qRatio = (((double) epidata.susceptible.size() + 1.0)
                      / ((double) epidata.numAdditions() * truncNorm_pdf(
                          epidata.infected.back()->N
                              - epidata.infected.back()->I, -1 / priors.b, 1
                              / (priors.a * priors.b * priors.b))));

                  if (log(gsl_rng_uniform(rng)) < (log_piCan - log_piCurr
                      + log(qRatio)))
                    {
//                      cout << "ADDED " << epidata.infected.back()->label
//                          << ", index = " << epidata.infected.size() - 1
//                          << endl;
                      log_prodCurr = log_prodCan;
                      loglikCurr = loglikCan;
                      bgPress = bgPress_can;
                      logCT = logCT_can;
                      A1 = A1_can;
                      A2 = A2_can;
                      *prodCurr_ptr = *prodCan_ptr;
                      if (epidata.infected.back()->I
                          < epidata.infected[epidata.I1]->I)
                        epidata.I1 = epidata.infected.size() - 1; // If we've proposed a new I1, update epidata.I1
                      ++accept[8];

                    }
                  else
                    {
//                      cout << "ADDED: REJECTED" << endl;
                      epidata.delInfec(epidata.infected.size() - 1);
                      *prodCan_ptr = *prodCurr_ptr;
                    }
                }
            }

          else
            {

              if (epidata.infected.size() != epidata.knownInfections)
                {
                  /* Delete an infection */

                  move_index = epidata.knownInfections + gsl_rng_uniform_int(
                      rng, epidata.numAdditions());

#ifdef __DEBUG__
                  if(h > 1)
                  cout << "DELETING " << epidata.infected[move_index]->label << "(" << move_index << ")" << "\n";
#endif

                  if (epidata.infected[move_index]->known == 1)
                    {
                      cout << "ERROR: Deleting a known infection! ("
                          << epidata.infected[move_index]->label << ")" << endl;
                    }

                  log_piCurr = loglikCurr + log(
                      1
                          - rng_extreme_cdf(epidata.infected[move_index]->N
                              - epidata.infected[move_index]->I, priors.a,
                              priors.b));

                  log_prodCan = delInfec_log_prod(move_index, parms, epidata,
                      log_prodCurr, prodCurr_ptr, prodCan_ptr);
                  bgPress_can = delInfec_bgPress(move_index, parms, epidata,
                      bgPress);
                  logCT_can = delInfec_logCT(move_index, parms, epidata, logCT);
                  A1_can = delInfec_A1(move_index, parms, epidata, A1);
                  A2_can = delInfec_A2(move_index, parms, epidata, A2);

                  loglikCan = log_prodCan - bgPress - A1_can - A2_can
                      + logCT_can;
                  log_piCan = loglikCan;

#ifdef __DEBUG__
                  if(h > 1)
                    {
                      cout << "Delete: log_prodCurr=" << log_prodCurr << ", bgPress_curr=" << bgPress << ", A1_curr=" << A1 << ", A2_curr=" << A2 << ", log_piCurr=" << log_piCurr << "\n";
                      cout << "Delete: log_prodCan=" << log_prodCan << ", bgPress=" << bgPress_can << ", A1_can=" << A1_can << ", A2_can=" << A2_can << ", log_piCan=" << log_piCan << "\n";
                    }
#endif

                  qRatio = (truncNorm_pdf(epidata.infected[move_index]->N
                      - epidata.infected[move_index]->I, -1 / priors.b, 1
                      / (priors.a * priors.b * priors.b))
                      * epidata.numAdditions())
                      / ((double) epidata.susceptible.size() + 1.0);

                  if (log(gsl_rng_uniform(rng)) < (log_piCan - log_piCurr
                      + log(qRatio)))
                    {
//                      cout << "DELETED " << epidata.infected[move_index]->label
//                          << ", index = " << move_index << endl;
                      epidata.delInfec(move_index);
                      if (move_index == epidata.I1)
                        {
                          epidata.updateI1();
                        }
                      log_prodCurr = log_prodCan;
                      bgPress = bgPress_can;
                      logCT = logCT_can;
                      A1 = A1_can;
                      A2 = A2_can;
                      loglikCurr = loglikCan;
                      *prodCurr_ptr = *prodCan_ptr;
                      ++accept[9];
                    }
                  else
                    {
                      *prodCan_ptr = *prodCurr_ptr;
                    }
                }

            }
          checkProdVec(prodCurr_ptr, parms);

        }
      checkProdVec(prodCurr_ptr, parms);
      assert(epidata.susceptible.size() + epidata.infected.size() == epidata.N_total);

      // Add output row to the variance objects:

      for (int k = 0; k < addOffset; ++k)
        parmsTemp.beta[k] = log(parms.beta[k]);
      multaddVariance.add(parmsTemp);
      for (int k = addOffset; k < parms.p; ++k)
        parmsTemp.beta[k] = log(parms.beta[k]);
      multVariance.add(parmsTemp);

      //vector<double> tempProd = *prodCurr_ptr;
      double reallogCT = computeLogCT(parms, epidata);
      if (fabs(reallogCT - logCT) > 0.0000000001)
        {
//          cout << "Iteration: " << h << ", Current logCT = " << logCT
//              << ", Real logCT = " << reallogCT << endl;
          // 		log_prodCurr = realProd;
          // 		for (int z = 0; z<tempProd.size(); ++z) {
          // 			cout << z << "\t|\t" << epidata.infected[z]->label << "\t|\t" << prodCurr_ptr->at(z) << "\t|\t" << tempProd[z] << "\t|\t" << prodCurr_ptr->at(z) - tempProd[z] << endl;
          // 		}
          // 		cout << "================================" << endl;
          abort();
        }

      // Write results to file
      if ((h % THINBY) == 0)
        {
          for (size_t k = 0; k < parms.p; ++k)
            resultsFile << parms.beta[k] << " ";

          resultsFile << epidata.mean_I() << " " << epidata.numAdditions()
              << " " << loglikCurr << " " << log_prodCurr << " " << bgPress
              << " " << logCT << " " << A1 << " " << A2 << " " << numInfecByCT(
              epidata) << " " << numInfecContacts(epidata) << endl;

#ifdef __DEBUG__
          if(h > 1)
          resultsFile << flush;
#endif

          // Store occult states
          occultWriter.write(epidata.infected.begin(), epidata.infected.end());
        }

    } /* END OF MCMC LOOP */

  time(&t_end);
  //debugOut.close();
  resultsFile.close();

  cout << "Model ran in " << difftime(t_end, t_start) / 60 << " minutes"
      << endl;
  cout << "Acceptance beta0: " << (float) accept[0] / (float) max_iter * 100
      << "%" << endl;
  cout << "Acceptance beta1: " << (float) accept[1] / (float) max_iter * 100
      << "%" << endl;
  cout << "Acceptance beta2: " << (float) accept[2] / (float) max_iter * 100
      << "%" << endl;
  cout << "Acceptance beta3: " << (float) accept[3] / (float) max_iter * 100
      << "%" << endl;
  cout << "Acceptance beta4: " << (float) accept[4] / (float) max_iter * 100
      << "%" << endl;
  cout << "Acceptance beta5: " << (float) accept[5] / (float) max_iter * 100
      << "%" << endl;
  cout << "Acceptance delta: " << (float) accept[6] / (float) max_iter * 100
      << "%" << endl;
  cout << "Acceptance move I: " << (float) accept[7] / (float) max_iter * 100
      << "%" << endl;
  cout << "Acceptance add I: " << accept[8] << endl;
  cout << "Acceptance del I: " << accept[9] << endl;
  return (0);
}

int
inputConf()
{

  ifstream configfile(config_filename, ios::in); // Open the configuration file
  if (!configfile.is_open())
    {
      cout << "Configuration file not found. Abort!" << endl;
    }

  char buffer[200]; // Buffer in which to store lines from the file
  char *split_position; //Pointer to the position of the equals sign in the config line
  char *item;

  while (!configfile.eof())
    {
      item = &buffer[0];
      configfile.getline(buffer, 99); // Read a line into the buffer
      split_position = strchr(buffer, '='); // Get the pointer to the = sign in the buffer
      if (split_position == NULL)
        continue; // If we haven't found an = sign then go to the next line
      else if (buffer[0] == '#')
        continue; // If the first character of the line is # then go to next line
      else
        {
          char variable[50];
          char value[150];
          *split_position = '\0'; // Zero out the = to terminate the buffer string at this point
          *split_position++; // Advance pointer to the first letter of our parameter
          int i = 0; // Counter for the copying process below
          while (item < split_position)
            { // Put the variable name into 'variable'
              variable[i] = *item;
              item++;
              i++;
            }
          i = 0; // Reset our counter
          while (*split_position != '\0')
            { // Copy the value into 'value'
              value[i] = *split_position;
              split_position++;
              i++;
            }
          value[i] = '\0';
          //cout << "Parsing: " << variable << endl;
          if (strcmp(variable, "EpiData") == 0)
            {
              strcpy(epidataFile, value);
            }
          else if (strcmp(variable, "Location_file") == 0)
            {
              strcpy(loc_filename, value);
            }
          else if (strcmp(variable, "total_pop_size") == 0)
            {
              epidata.N_total = atoi(value);
            }
          else if (strcmp(variable, "a") == 0)
            {
              priors.a = atof(value);
            }
          else if (strcmp(variable, "b") == 0)
            {
              priors.b = atof(value);
            }
          else if (strcmp(variable, "f") == 0)
            {
              parms.f = atof(value);
            }
          else if (strcmp(variable, "g") == 0)
            {
              parms.g = atof(value);
            }
          else if (strstr(variable, "beta_start"))
            {
              for (int k = 0; k < parms.p; ++k)
                {
                  char myStr[50];
                  sprintf(myStr, "beta_start%i", k);
                  if (strcmp(variable, myStr) == 0)
                    {
                      parms.beta[k] = atof(value);
                      break;
                    }
                }
            }
          else if (strstr(variable, "sigma_beta"))
            {
              if (strstr(variable, "_add"))
                {
                  for (int k = 0; k < parms.p; ++k)
                    {
                      char myStr[50];
                      sprintf(myStr, "sigma_beta%i_add", k);
                      if (strcmp(variable, myStr) == 0)
                        {
                          cout << "Got " << myStr << " (" << variable << ") = "
                              << value << endl;
                          sigma_add[k] = atof(value);
                          break;
                        }
                    }
                }
              else
                {
                  for (int k = 0; k < parms.p; ++k)
                    {
                      char myStr[50];
                      sprintf(myStr, "sigma_beta%i", k);
                      if (strcmp(variable, myStr) == 0)
                        {
                          cout << "Got " << myStr << " (" << variable << ") = "
                              << value << endl;
                          sigma_mult[k] = atof(value);
                          break;
                        }
                    }
                }
            }
          else if (strstr(variable, "lambda_beta"))
            {
              for (int k = 0; k < parms.p; ++k)
                {
                  char myStr[50];
                  sprintf(myStr, "lambda_beta%i", k);
                  if (strcmp(variable, myStr) == 0)
                    {
                      priors.lambda[k] = atof(value);
                      break;
                    }
                }
            }
          else if (strstr(variable, "nu_beta"))
            {
              for (int k = 0; k < parms.p; ++k)
                {
                  char myStr[50];
                  sprintf(myStr, "nu_beta%i", k);
                  if (strcmp(variable, myStr) == 0)
                    {
                      priors.nu[k] = atof(value);
                      break;
                    }
                }
            }
          else if (strcmp(variable, "add_mult_ratio") == 0)
            {
              a_m_ratio = atof(value);
            }
          else if (strcmp(variable, "no_moves") == 0)
            {
              infecFiddle = atoi(value);
            }
          else if (strcmp(variable, "max_iterations") == 0)
            {
              max_iter = atoi(value);
            }
          else if (strcmp(variable, "contact_matrix_prefix") == 0)
            {
              strcpy(cMat_prefix, value);
            }
          else if (strcmp(variable, "output_file") == 0)
            {
              strcpy(output_filename, value);
            }
          else if (strcmp(variable, "ObsTime") == 0)
            {
              ObsTime = atof(value);
            }
          else if (strcmp(variable, "add_offset") == 0)
            {
              addOffset = atoi(value);
            }
          else if (strcmp(variable, "xi") == 0)
            {
              xi = atof(value);
            }
        } // End if statement
    } // End while statement

  configfile.close(); // Close configuration file
  return (0);
}
