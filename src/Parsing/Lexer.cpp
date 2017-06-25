#include "Parsing/Lexer.h"
#include <map>
#include <string>
#include <iostream>
#include <cctype>
#include <cstdlib>

namespace pasclang::Parsing {

static std::vector<std::string> tokenString =
{
    "end of file", "int literal", "boolean literal", "identifier",
    "program", ".", "begin", "end", "var", "function", "procedure",
    "(", ")", "[", "]", ":=", "or", "and", "not", "=", "<>", "+", "-", "*", "/",
    "<=", "<", ">=", ">", ":", ";", ",", "array", "of", "int", "bool",
    "new", "if", "then", "else", "while", "do"
};

static std::map<std::string, Token::TokenType> keywordsMap =
{
    { "program", Token::TokenType::PROGRAM },
    { "begin", Token::TokenType::BEGIN },
    { "end", Token::TokenType::END },
    { "var", Token::TokenType::VAR },
    { "function", Token::TokenType::FUNCTION },
    { "procedure", Token::TokenType::PROCEDURE },
    { "array", Token::TokenType::ARRAY },
    { "and", Token::TokenType::AND },
    { "or", Token::TokenType::OR },
    { "not", Token::TokenType::NOT },
    { "of", Token::TokenType::OF },
    { "new", Token::TokenType::NEW },
    { "integer", Token::TokenType::INTTYPE },
    { "boolean", Token::TokenType::BOOLTYPE },
    { "if", Token::TokenType::IF },
    { "then", Token::TokenType::THEN },
    { "else", Token::TokenType::ELSE },
    { "while", Token::TokenType::WHILE },
    { "do", Token::TokenType::DO }
};

std::string& tokenToString(Token::TokenType type)
{
    return tokenString[type];
}

const std::string Token::getTypeString() const
{
    return tokenToString(this->type);
}

void Lexer::doLexing(std::string& file)
{
    this->file = file;
    this->buildTokenList();
}

char Lexer::getNextChar()
{
    char nextChar;
    do {
        nextChar = this->reporter->readStream();
        this->offset++;
        if(nextChar == '\n')
        {
            this->line++;
            this->beginningOfLine = this->offset + 1;
        }
    }
    while(std::isspace(nextChar));

    return nextChar;
}

void Lexer::skipComment()
{
    int currentDepth = 1;

    while(currentDepth > 0)
    {
        this->currentChar = this->getNextChar();
        if(this->currentChar == '}')
        {
            currentDepth--;
            this->currentChar = this->getNextChar();
        }
        if(this->currentChar == '{')
        {
            currentDepth++;
            this->currentChar = this->getNextChar();
        }
    }
}

Token Lexer::getNextToken()
{
    buffer = "";
    this->currentChar = this->getNextChar();

    Position start(this->line, this->beginningOfLine, this->offset, this->file);

    Token::TokenType type;

    while(this->currentChar == '{')
    {
        this->skipComment();
    }

    switch(this->currentChar)
    {
        case EOF:
            type = Token::TokenType::ENDFILE;
            break;

        case '(':
            type = Token::TokenType::LEFTPAR;
            break;

        case ')':
            type = Token::TokenType::RIGHTPAR;
            break;

        case '[':
            type = Token::TokenType::LEFTBRACK;
            break;

        case ']':
            type = Token::TokenType::RIGHTBRACK;
            break;

        case '.':
            type = Token::TokenType::DOT;
            break;

        case ':':
            if(this->reporter->peekStream() == '=') {
                this->currentChar = this->getNextChar();
                type = Token::TokenType::ASSIGN;
                break;
            }
            type = Token::TokenType::COLON;
            break;

        case ';':
            type = Token::TokenType::SEMICOLON;
            break;

        case ',':
            type = Token::TokenType::COMMA;
            break;

        case '+':
            type = Token::TokenType::PLUS;
            break;

        case '-':
            type = Token::TokenType::MINUS;
            break;

        case '*':
            type = Token::TokenType::STAR;
            break;

        case '/':
            type = Token::TokenType::SLASH;
            break;

        case '=':
            type = Token::TokenType::EQUAL;
            break;

        case '<':
            if(this->reporter->peekStream() == '=') {
                currentChar = this->getNextChar();
                type = Token::TokenType::LEQUAL;
                break;
            }
            if(this->reporter->peekStream() == '>') {
                currentChar = this->getNextChar();
                type = Token::TokenType::NEQUAL;
                break;
            }
            type = Token::TokenType::LTHAN;
            break;

        case '>':
            if(this->reporter->peekStream() == '=') {
                currentChar = this->getNextChar();
                type = Token::TokenType::GEQUAL;
                break;
            }
            type = Token::TokenType::GTHAN;
            break;

        default:

            // Lexing integer literals
            if(std::isdigit(this->currentChar)) {
                this->buffer += this->currentChar;
                while(std::isdigit(this->reporter->peekStream()))
                {
                    this->currentChar = this->getNextChar();
                    this->buffer += this->currentChar;
                }

                type = Token::TokenType::INTLITERAL;
                break;
            }

            // Lexing identifier/keyword/booleans
            if(std::isalpha(this->currentChar)) {
                this->buffer += this->currentChar;
                while(std::isalnum(this->reporter->peekStream()))
                {
                    this->currentChar = this->getNextChar();
                    this->buffer += this->currentChar;
                }

                if(keywordsMap.find(this->buffer) != keywordsMap.end()) {
                    type = keywordsMap[this->buffer];
                    break;
                }

                if(this->buffer == "true" || this->buffer == "false")
                {
                    type = Token::TokenType::BOOLLITERAL;
                    break;
                }

                type = Token::TokenType::IDENTIFIER;
                break;
            }

            type = Token::TokenType::ENDFILE;
            this->errorHappened = true;
            break;
    }

    Position end(this->line, this->beginningOfLine, this->offset, this->file);

    Token t(start, end, type, buffer);

    return t;
}

void Lexer::buildTokenList()
{
    this->reporter->openStream(file);
    while(!this->reporter->endOfStream())
    {
        Token t(this->getNextToken());
        tokens.push_back(t);
    }
    this->reporter->closeStream();
}

} // namespace pasclang::Parsing

