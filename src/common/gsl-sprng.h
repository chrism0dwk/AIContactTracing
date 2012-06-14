/*
gsl-sprng.h

Code declaring a new GSL random number type "gsl_rng_sprng20"
which is a thin wrapper over the SPRNG 2.0 parallel random
number generator.

To use in serial mode, just add the line:
#include "gsl-sprng.h"
immediately after the line:
#include <gsl/gsl_rng.h>
near the start of your code. 

To use in parallel mode put
#define USE_MPI before the
#include "gsl-sprng.h"
line.

The new type should now be available.
Make sure you alloc the rng on each processor. If you wish to
set a seed, you should set it to be the same on each processor.

Darren Wilkinson
d.j.wilkinson@ncl.ac.uk
http://www.staff.ncl.ac.uk/d.j.wilkinson/

Modified C. Jewell 12/02/2008

*/

#ifndef SPRNG_H
#define SPRNG_H

#include "sprng.h"
#include <iostream>

extern "C" {


  static int* defaultSPRNGgen = NULL;

  void sprng_reset(unsigned long int s,
		   /*unsigned long*/ int myId,
		   /*unsigned long*/ int numProcs)
  {
    int* temp;
    std::cerr << "Stream " << myId << " out of " << numProcs << std::endl;
    temp = init_sprng(DEFAULT_RNG_TYPE,myId,numProcs,s,SPRNG_DEFAULT);

    if(defaultSPRNGgen != NULL)
      free_rng(defaultSPRNGgen);
    defaultSPRNGgen = temp;
    
   }

  static void sprng_set(void * vstate,unsigned long int s)
  {
    int myId,numProcs;

    if(defaultSPRNGgen != NULL)
      free_rng(defaultSPRNGgen);

#ifdef USE_MPI
    MPI_Comm_rank(MPI_COMM_WORLD,&myId);
    MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
#else
    myId = 0;
    numProcs = 1;
#endif
    defaultSPRNGgen = init_sprng(DEFAULT_RNG_TYPE,myId,numProcs,s,SPRNG_DEFAULT);
  }
  
  static unsigned long sprng_get(void * vstate)
  {
    return( (long) isprng(defaultSPRNGgen) );
  }
  
  static double sprng_get_double(void * vstate)
  {
    return( (double) sprng(defaultSPRNGgen));
  }
  
  static const gsl_rng_type sprng_type =
  {"sprng20",        /* name */
   0x7fffffffUL,     /* RAND_MAX */
   0,                /* RAND_MIN */
   0,                /* size of state - not sure about this */
   &sprng_set,          /* initialisation */
   &sprng_get,          /* get integer RN */
   &sprng_get_double};  /* get double RN */
  
  const gsl_rng_type *gsl_rng_sprng20 = &sprng_type;
  
}
  /* eof */
  
  
#endif
