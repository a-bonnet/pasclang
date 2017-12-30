#ifndef PASCLANG_H
#define PASCLANG_H

#include <stdexcept>

namespace pasclang {

enum ExitCode {
    Success = 0,
    WrongUsage,
    InternalError,
    LexicalError,
    SyntaxError,
    TypeError,
    GeneratorError
};

class PasclangException : public std::runtime_error {
  private:
    ExitCode code;

  public:
    PasclangException(ExitCode code, const char* what = "")
        : std::runtime_error(what), code(code) {}
    ~PasclangException() {}

    ExitCode getCode() { return this->code; }
};

} // namespace pasclang

#endif
