// Produces the token list from the input file.

#ifndef PASCLANG_PARSING_LEXER_H
#define PASCLANG_PARSING_LEXER_H

#include "Message/BaseReporter.h"
#include "Parsing/Tokens.h"
#include "Pasclang.h"
#include <fstream>
#include <string>
#include <vector>

namespace pasclang::Parsing {

class Lexer {
  private:
    std::string file;
    std::fstream stream;
    char currentChar = ' ';
    int line = 1;
    int offset = -1;
    int beginningOfLine = 0;
    std::string buffer = "";
    std::vector<Token> tokens;
    bool errorHappened = false;
    bool traceLexing = false;
    Message::BaseReporter* reporter;

    char getNextChar();
    void skipComment();
    void buildTokenList();
    Token getNextToken();
    [[noreturn]] void lexicalError(std::string& errorMessage);

  public:
    Lexer(Message::BaseReporter* reporter) : reporter(reporter) {}
    void doLexing(std::string& file);
    ~Lexer() {}
    void toggleTrace(bool trace) { this->traceLexing = trace; }

    std::vector<Token>& getTokensList() { return this->tokens; }
};

} // namespace pasclang::Parsing

#endif
