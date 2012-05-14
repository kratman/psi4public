#include <psi4-dec.h>
#include <psiconfig.h>
#include <libmints/molecule.h>
#include <libmints/extern.h>
#include <boost/algorithm/string.hpp>

//MKL Header
#ifdef HAVE_MKL
#include <mkl.h>
#endif

//OpenMP Header
//_OPENMP is defined by the compiler if it exists
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using namespace psi;
using namespace boost;

Process::Environment Process::environment;
Process::Arguments Process::arguments;
const std::string empty_;

// Need to split each entry by the first '=', left side is key, right the value
void Process::Environment::init(char **envp)
{
    string psi4datadir;

    // First set some defaults:
    environment_["PSIDATADIR"] = INSTALLEDPSIDATADIR;
    environment_["MAD_NUM_THREADS"] = "1";

    // Go through user provided environment overwriting defaults if necessary
    int i=0;
    while (envp[i] != NULL) {
        std::vector<std::string> strs;
        boost::split(strs, envp[i], boost::is_any_of("="));
        environment_[strs[0]] = strs[1];

        // I'm tired of having to (re)set PSIDATADIR for PSI3/4
        // If PSI4DATADIR is set it overrides PSIDATADIR
        if (strs[0] == "PSI4DATADIR")
            psi4datadir = strs[1];

        ++i;
    }

    if (psi4datadir.empty() == false)
        environment_["PSIDATADIR"] = psi4datadir;

    nthread_ = 1;

#ifdef _OPENMP
    nthread_ = omp_get_max_threads();
#endif

    // If madness and or MPI is not set up, COMMUNICATOR is changed to a value
    // that makes sense. (i.e. MPI or LOCAL)
#ifdef HAVE_MADNESS
    if ( (Process::environment("COMMUNICATOR") != "MADNESS") &&
         (Process::environment("COMMUNICATOR") != "LOCAL") )
    {
        environment_["COMMUNICATOR"] = "MADNESS";
        std::cout << "WARNING: COMMUNICATOR was changed to MADNESS" << std::endl;
    }
#else
    if (Process::environment("COMMUNICATOR") != "LOCAL") {
        environment_["COMMUNICATOR"] = "LOCAL";
    }
#endif
}

void Process::Environment::set_n_threads(int nthread)
{
    nthread_ = nthread;
#ifdef _OPENMP
    omp_set_num_threads(nthread_);
#endif
#ifdef HAVE_MKL
    mkl_set_num_threads(nthread_);
#endif

    // HACK: TODO: CC-pthread codes should ask us how many threads
    // No, this didn't work, back this out for now (and we won't need
    // this once the final solution is in anyway)  --CDS
    // Process::environment.options.set_global_int("NUM_THREADS",nthread);
}

const string& Process::Environment::operator()(const string& key) const
{
    // Search for the key:
    map<string, string>::const_iterator it = environment_.find(key);

    if (it == environment_.end())
        return empty_;      // Not found return empty string.
    else
        return it->second;  // Found, return the value
}

string Process::Environment::operator()(const string& key)
{
    // Search for the key:
    map<string, string>::const_iterator it = environment_.find(key);

    if (it == environment_.end())
        return string(); // Not found return empty string.
    else
        return it->second; // Found, return the value
}

const string& Process::Environment::set(const std::string &key, const std::string &value)
{
    const string& old = operator()(key);
    environment_[key] = value;

    // Attempt to set the variable in the system.
#ifdef HAVE_SETENV
    setenv(key.c_str(), value.c_str(), 1);
#elif HAVE_PUTENV
    size_t len = key.length() + value.length() + 2; // 2 = 1 (equals sign) + 1 (null char)
    char *str = new char[len];
    sprintf(str, "%s=%s", key.c_str(), value.c_str());
    // we give up ownership of the memory allocation
    putenv(str);
#else
#pragma error setenv and putenv not available.
#endif

    return old;
}

void Process::Environment::set_molecule(const boost::shared_ptr<Molecule>& molecule)
{
    molecule_ = molecule;
}

boost::shared_ptr<Molecule> Process::Environment::molecule() const
{
    return molecule_;
}

void Process::Environment::set_reference_wavefunction(const boost::shared_ptr<Wavefunction>& reference_wavefunction)
{
    reference_wavefunction_ = reference_wavefunction;
}

boost::shared_ptr<Wavefunction> Process::Environment::reference_wavefunction() const
{
    return reference_wavefunction_;
}

void Process::Arguments::init(int argc, char **argv)
{
    argc_ = argc;
    argv_ = argv;

    for (int i=0; i<argc; ++i)
        arguments_.push_back(argv[i]);
}

const string& Process::Arguments::operator ()(int argc) const
{
    if (argc < 0 || argc >= arguments_.size())
        throw SanityCheckError("Process:Arguments::operator ()(int argc): argc is either < 0 or >= the number of arguments.\n", __FILE__, __LINE__);

    return arguments_[argc];
}

string Process::Arguments::operator ()(int argc)
{
    if (argc < 0 || argc >= arguments_.size())
        throw SanityCheckError("Process:Arguments::operator ()(int argc): argc is either < 0 or >= the number of arguments.\n", __FILE__, __LINE__);

    return arguments_[argc];
}

Process::Environment Process::get_environment()
{
    return environment;
}

unsigned long int Process::Environment::get_memory() const { return memory_; }
void Process::Environment::set_memory(unsigned long int m)  { memory_ = m; }

int Process::Environment::get_n_threads() const {return nthread_; }

int Process::Arguments::argc() const { return arguments_.size(); }

namespace psi {
void die_if_not_converged()
{
  fprintf(outfile, "Iterations did not converge.");

  if (Process::environment.options.get_bool("DIE_IF_NOT_CONVERGED"))
    throw PSIEXCEPTION("Iterations did not converge.");
  else {
    fprintf(stderr, "Iterations did not converge.");
  }
}
}

