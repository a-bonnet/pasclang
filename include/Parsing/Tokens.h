#ifndef PASCLANG_PARSING_TOKENS_H
#define PASCLANG_PARSING_TOKENS_H

#include <vector>
#include <string>
#include <memory>
#include "Parsing/Location.h"

namespace pasclang::Parsing {

class Token {

    public:
        enum TokenType {
            ENDFILE,
            INTLITERAL,
            BOOLLITERAL,
            IDENTIFIER,
            PROGRAM,
            DOT,
            BEGIN,
            END,
            VAR,
            FUNCTION,
            PROCEDURE,
            LEFTPAR,
            RIGHTPAR,
            LEFTBRACK,
            RIGHTBRACK,
            ASSIGN,
            OR,
            AND,
            NOT,
            EQUAL,
            NEQUAL,
            PLUS,
            MINUS,
            STAR,
            SLASH,
            LEQUAL,
            LTHAN,
            GEQUAL,
            GTHAN,
            COLON,
            SEMICOLON,
            COMMA,
            ARRAY,
            OF,
            INTTYPE,
            BOOLTYPE,
            NEW,
            IF,
            THEN,
            ELSE,
            WHILE,
            DO
        };

    private:
        Location location;
        TokenType type;
        std::string literal;

    public:
        Token(Position& start, Position& end, TokenType type, std::string& literal) :
            location(start, end), type(type), literal(literal) { }

        const Location& getLocation() const { return this->location; }
        TokenType getType() { return this->type; }
        const std::string& getLiteral() const { return this->literal; }
        const std::string getTypeString() const;
        TokenType getType() const { return this->type; }

};

std::string& tokenToString(Token::TokenType type);

} // namespace pasclang::Parsing

#endif

