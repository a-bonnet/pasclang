#include "Parsing/Driver.h"

namespace pasclang::Parsing {

std::unique_ptr<AST::Program> Driver::parse(std::string& file) {
    Lexer lexer(reporter);
    Parser parser(reporter);
    lexer.doLexing(file);
    parser.swapTokensList(lexer.getTokensList());
    return parser.parse();
}

void Driver::error(const Location&, const std::string&) {}

void Driver::error(const std::string&) {}

} // namespace pasclang::Parsing
