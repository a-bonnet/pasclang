// The parsing driver glues the different components
// responsible for building the AST from the source
// file together.

#ifndef PASCLANG_PARSING_DRIVER_H
#define PASCLANG_PARSING_DRIVER_H

#include "AST/AST.h"
#include "Message/BaseReporter.h"
#include "Parsing/Lexer.h"
#include "Parsing/Parser.h"
#include "Pasclang.h"
#include <map>
#include <memory>
#include <string>

namespace pasclang::Parsing {

class Location;

class Driver {
  private:
    std::string fileName;
    Message::BaseReporter* reporter;

  public:
    Driver(Message::BaseReporter* reporter) : reporter(reporter) {}
    virtual ~Driver() {}

    void beginLexing();
    void endLexing();

    std::unique_ptr<AST::Program> parse(std::string& file);

    void error(const Parsing::Location& location, const std::string& message);
    void error(const std::string& message);
};

} // namespace pasclang::Parsing

#endif
