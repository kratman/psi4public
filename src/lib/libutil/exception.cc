#include <exception.h>
#include <sstream>

using namespace psi;
using namespace std;

PsiException::PsiException(
    string msg,
    const char* _file,
    int _line
) throw() : runtime_error(msg)
{
    msg_ = msg;
    file_ = _file;
    line_ = _line;
}

PsiException::PsiException(const PsiException& copy) throw()
    : runtime_error(copy.msg_), msg_(copy.msg_), file_(strdup(copy.file_)),
        line_(copy.line_)
{
}

void
PsiException::rewrite_msg(string msg) throw()
{
    msg_ = msg;
}
    
const char* 
PsiException::what() const throw()
{
    stringstream sstr;
    sstr << msg_ << "\n";
    sstr << location();
    return sstr.str().c_str();
}

const char* 
PsiException::file() const throw()
{
    return file_;
}

int 
PsiException::line() const throw()
{
    return line_;
}

const char*
PsiException::location() const throw()
{
    stringstream sstr;
    sstr << "file: " << file_ << "\n";
    sstr << "line: " << line_;
    return sstr.str().c_str();
}

PsiException::~PsiException() throw()
{
}

SanityCheckError::SanityCheckError(
    string message,
    const char* _file,
    int _line
    ) throw() 
  : PsiException(message, _file, _line)
{
    stringstream sstr;
    sstr << "sanity check failed! " << message;
    rewrite_msg(sstr.str());
}

SanityCheckError::~SanityCheckError() throw() {}

InputException::InputException(
    string msg,
    string param_name,
    int value,
    const char* _file,
    int _line
) throw() : PsiException(msg, _file, _line)
{
    write_input_msg<int>(msg, param_name, value);
}

InputException::InputException(
    string msg,
    string param_name,
    string value,
    const char* _file,
    int _line
) throw() : PsiException(msg, _file, _line)
{
    write_input_msg<string>(msg, param_name, value);
}

InputException::InputException(
    string msg,
    string param_name,
    double value,
    const char* _file,
    int _line
) throw() : PsiException(msg, _file, _line)
{
    write_input_msg<double>(msg, param_name, value);
}

InputException::InputException(
    string msg,
    string param_name,
    const char* _file,
    int _line
) throw() : PsiException(msg, _file, _line)
{
    write_input_msg<string>(msg, param_name, "in input");
}

template <
    class T
> void
InputException::write_input_msg(
    string msg,
    string param_name,
    T value
) throw()
{
    stringstream sstr;
    sstr << msg << "\n";
    sstr << "value " << value << " is incorrect" << "\n";
    sstr << "please change " << param_name << " in input";
    rewrite_msg(sstr.str());
}

StepSizeError::StepSizeError(
    string value_name,
    double max,
    double actual,
    const char* _file,
    int _line) throw()
    : LimitExceeded<double>(value_name + " step size", max, actual, _file, _line)
{
}

StepSizeError::~StepSizeError() throw() {}

MaxIterationsExceeded::MaxIterationsExceeded(
    string routine_name,
    int max,
    const char* _file,
    int _line)  throw()
    : LimitExceeded<int>(routine_name + " iterations", max, max, _file, _line)
{
}

MaxIterationsExceeded::~MaxIterationsExceeded() throw() {}

ConvergenceError::ConvergenceError(
    string routine_name,
    int max,
    double _desired_accuracy,
    double _actual_accuracy,
    const char* _file,
    int _line) throw()
    : MaxIterationsExceeded(routine_name + " iterations", max, _file, _line), desired_acc_(_desired_accuracy), actual_acc_(_actual_accuracy)
{
    stringstream sstr;
    sstr << "could not converge " << routine_name << ".  desired " << _desired_accuracy << " but got " << _actual_accuracy << "\n";
    sstr << description();
    rewrite_msg(sstr.str());
}

ConvergenceError::~ConvergenceError() throw() {}

double 
ConvergenceError::desired_accuracy() const throw() {return desired_acc_;}

double 
ConvergenceError::actual_accuracy() const throw() {return actual_acc_;}

ResourceAllocationError::ResourceAllocationError(
    string resource_name,
    size_t max,
    size_t actual,
    const char* _file,
    int _line)  throw()
    : LimitExceeded<size_t>(resource_name, max, actual, _file, _line)
{
}

FeatureNotImplemented::FeatureNotImplemented(
    string module_name,
    string feature_name,
    const char* _file,
    int _line
) throw() 
 : PsiException("psi exception", _file, _line)
{
    stringstream sstr;
    sstr << feature_name << " not implemented in " << module_name;
    rewrite_msg(sstr.str());
}

FeatureNotImplemented::~FeatureNotImplemented() throw()
{
}

ResourceAllocationError::~ResourceAllocationError() throw() {}

