/*! \file
    \ingroup CCENERGY
    \brief Enter brief description of file here 
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cmath>
#include <libipv1/ip_lib.h>
#include <libciomr/libciomr.h>
#include <psifiles.h>
#include "Params.h"
#include "Local.h"
#include "MOInfo.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace ccenergy {

void get_params()
{
  int errcod, iconv, forceit;
  std::string cachetype = "";
  sdt::string  junk;

  params.newtrips = options.get_bool("NEWTRIPS");

  params.wfn = options.get_str("WFN")

  if(params.wfn == "NONE")
     throw PsiException("Invalid value of input keyword WFN", __FILE__, __LINE__);

  if(params.wfn == "BCCD" || params.wfn == "BCCD_T") 
    params.brueckner = 1;
  else params.brueckner = 0;

  params.semicanonical = 0;
  junk = options.get_str("REFERENCE");
  /* if no reference is given, assume rhf */
  if(junk == "RHF") params.ref = 0;
  else if(junk == "ROHF" && 
    params.wfn == "MP2" || params.wfn == "CCSD_T" ||
    params.wfn == "CC3" || params.wfn == "EOM_CC3" ||
    params.wfn == "CC2" || params.wfn == "EOM_CC2") {
    params.ref = 2;
    params.semicanonical = 1;
  }
  else if(junk == "ROHF") params.ref = 1;
  else if(junk == "UHF" ) params.ref = 2;
  else  
   throw PsiException("Invalid value of input keyword REFERENCE", __FILE__, __LINE__);

  params.analyze = options.get_bool("ANALYZE");

  params.dertype = 0;
  junk = options.get_str("DERTYPE")
  if(junk == "NONE") params.dertype = 0;
  else if(junk == "FIRST") params.dertype = 1;
  else if(junk == "RESPONSE") params.dertype = 3; /* linear response */
  else 
   throw PsiException("Invalid value of input keyword DERTYPE", __FILE__, __LINE__);

  params.print = options.get_int("PRINT");
  params.maxiter = options.get_int("MAXITER");
  iconv = options.get_int("CONVERGENCE");
  params.convergence = 1.0*pow(10.0,(double) -iconv);
  params.restart = options.get_bool("RESTART");
  /* If the MO orbital phases are screwed up, don't restart */
  if(!moinfo.phase) params.restart = 0;
  /* BUT, the user can force an override of the phase problem */
  forceit = options.get_bool("FORCE_RESTART");
  if(forceit) params.restart = 1;

  fndcor(&(params.memory),infile,outfile);

  params.aobasis = options.get_str("AO_BASIS";)

//  if(strcmp(params.aobasis,"DISK") && strcmp(params.aobasis,"DIRECT") &&
//     strcmp(params.aobasis,"NONE")) 
//   throw PsiException("Error in input: invalid AO_BASIS", __FILE__, __LINE__);
  

  params.cachelev = options.get_int("CACHELEV");

  params.cachetype = 1;
  cachetype = options.get_str("CACHETYPE");
  if(cachetype == "LOW") params.cachetype = 1;
  else if(cachetype == "LRU") params.cachetype = 0;
  else 
    throw PsiException("Error in input: invalid CACHETYPE", __FILE__, __LINE__);
 

 if(params.ref == 2) /* No LOW cacheing yet for UHF references */
    params.cachetype = 0;

  params.nthreads = options.get_int("NTHREADS");
  params.diis = options.get_bool("DIIS");
  params.t2_coupled = options.get_bool("T2_COUPLED");
  params.prop = options.get_str("PROPERTY");
  params.abcd = options.get_str("ABCD");
  params.local = options.get_bool("LOCAL");
  local.cutoff = options.get_double("LOCAL_CUTOFF");
  params.local_mos = options.get_bool("LOCAL_MOS");
  local.method = options.get_cstr("LOCAL_METHOD");
  local.weakp = options.get_cstr("LOCAL_WEAKP");

  local.filter_singles = options.get_bool("LOCAL_FILTER_SINGLES");
  if(params.dertype == 3) local.filter_singles = 0;

  local.cphf_cutoff = options.get_double("LOCAL_CPHF_CUTOFF");
  local.freeze_core = options.get_bool("FREEZE_CORE");

  local.pairdef = options.get_cstr("LOCAL_PAIRDEF");
  if(params.local && params.dertype == 3)
    local.pairdef = strdup("RESPONSE");
  else if(params.local)
    local.pairdef = strdup("BP");

  params.num_amps = options.get_int("NUM_AMPS");
  iconv = options.get_int("BRUECKNER_CONV");
  params.bconv = 1.0*pow(10.0,(double) -iconv);

  params.print_mp2_amps = options.get_bool("PRINT_MP2_AMPS");
  params.print_pair_energies = options.get_bool("PRINT_PAIR_ENERGIES");
  params.spinadapt_energies = options.get_bool("SPINADAPT_ENERGIES");
  params.t3_Ws_incore = options.get_bool("T3_WS_INCORE");

  /* get parameters related to SCS-MP2 or SCS-N-MP2 */
  /* see papers by S. Grimme or J. Platz */
  params.scsn = options.get_bool("SCSN_MP2");
  params.scs = options.get_bool("SCS_MP2");
  params.scscc = options.get_bool("SCS_CCSD");
  params.scsmp2_scale_os = options.get_double("MP2_SCALE_OS");
  params.scsmp2_scale_ss = options.get_double("MP2_SCALE_SS");
  /* see paper by T. Takatani*/
  params.scscc_scale_os = options.get_double("CC_SCALE_OS");
  params.scscc_scale_ss = options.get_double("CC_SCALE_SS");

  if (options["MP2_SCALE_OS"].has_changed() || options["MP2_SCALE_SS"].has_changed()) {
    params.scs = 1;
    }

  if (options["CC_SCALE_OS"].has_changed() || options["CC_SCALE_SS"].has_changed()) {
    params.scscc = 1;
    }


  fprintf(outfile, "\n\tInput parameters:\n");
  fprintf(outfile, "\t-----------------\n");
  fprintf(outfile, "\tWave function   =   %6s\n", params.wfn);
  
  if(params.semicanonical) {
    fprintf(outfile, "\tReference wfn   =     ROHF changed to UHF for Semicanonical Orbitals\n");
  }
  else {
    fprintf(outfile, "\tReference wfn   =   %5s\n",
	    (params.ref == 0) ? "RHF" : ((params.ref == 1) ? "ROHF" : "UHF"));
  }
  if(params.brueckner) 
    fprintf(outfile, "\tBrueckner conv. =     %3.1e\n", params.bconv);
  fprintf(outfile, "\tMemory (Mbytes) =     %5.1f\n",params.memory/1e6);
  fprintf(outfile, "\tMaxiter         =   %4d\n", params.maxiter);
  fprintf(outfile, "\tConvergence     =     %3.1e\n", params.convergence);
  fprintf(outfile, "\tRestart         =     %s\n", 
	  params.restart ? "Yes" : "No");
  fprintf(outfile, "\tDIIS            =     %s\n", params.diis ? "Yes" : "No");
  fprintf(outfile, "\tAO Basis        =     %s\n", params.aobasis);
  fprintf(outfile, "\tABCD            =     %s\n", params.abcd);
  fprintf(outfile, "\tCache Level     =     %1d\n", params.cachelev);
  fprintf(outfile, "\tCache Type      =    %4s\n", 
	  params.cachetype ? "LOW" : "LRU");
  fprintf(outfile, "\tPrint Level     =     %1d\n",  params.print);
  fprintf(outfile, "\tNum. of threads =     %d\n",  params.nthreads);
  fprintf(outfile, "\t# Amps to Print =     %1d\n",  params.num_amps);
  fprintf(outfile, "\tPrint MP2 Amps? =     %s\n",  params.print_mp2_amps ?
	  "Yes" : "No" );
  fprintf(outfile, "\tAnalyze T2 Amps =     %s\n",  params.analyze ? "Yes" : "No" );
  fprintf(outfile, "\tPrint Pair Ener =     %s\n",  params.print_pair_energies ? "Yes" : "No" );

  if (params.print_pair_energies)
    fprintf(outfile, "\tSpinadapt Ener. =     %s\n",  params.spinadapt_energies ? "Yes" : "No" );
  fprintf(outfile, "\tLocal CC        =     %s\n", params.local ? "Yes" : "No");

  if ( params.wfn == "CC3" || params.wfn == "EOM_CC3") 
    fprintf(outfile, "\tT3 Ws incore    =     %s\n", params.t3_Ws_incore ? "Yes" : "No");

  if(params.local) {
    fprintf(outfile, "\tLocal Cutoff       =     %3.1e\n", local.cutoff);
    fprintf(outfile, "\tLocal Method      =     %s\n", local.method);
    fprintf(outfile, "\tWeak pairs        =     %s\n", local.weakp);
    fprintf(outfile, "\tFilter singles    =     %s\n", local.filter_singles ? "Yes" : "No");
    fprintf(outfile, "\tLocal pairs       =     %s\n", local.pairdef);
    fprintf(outfile, "\tLocal CPHF cutoff =     %3.1e\n", local.cphf_cutoff);
  }
  fprintf(outfile, "\tSCS-MP2         =     %s\n", (params.scs == 1) ? "True" : "False");
  fprintf(outfile, "\tSCSN-MP2        =     %s\n", (params.scsn == 1) ? "True" : "False");
  fprintf(outfile, "\tSCS-CCSD        =     %s\n", (params.scscc == 1) ? "True" : "False");
  if (params.scs) {
    fprintf(outfile, "\tSCS_MP2_SCALE_OS =     %.2f\n",params.scsmp2_scale_os);
    fprintf(outfile, "\tSCS_MP2_SCALE_SS =     %.2f\n",params.scsmp2_scale_ss);
  }
  if (params.scsn) {
    fprintf(outfile, "\tSCSN_MP2_SCALE_OS =     %.2f\n",0.0);
    fprintf(outfile, "\tSCSN_MP2_SCALE_SS =     %.2f\n",1.76);
  }
  if (params.scscc) {
    fprintf(outfile, "\tCC_SCALE_OS     =     %.2f\n",params.scscc_scale_os);
    fprintf(outfile, "\tCC_SCALE_SS     =     %.2f\n",params.scscc_scale_ss);
  }

  fprintf(outfile, "\n");

}
}} // namespace psi::ccenergy
