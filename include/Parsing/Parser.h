// The current parser implementation performs recursive descent with
// backtracking. It reports correctly the first syntactic error,
// then can merely make guesses as the AST will be ill-formed.

#ifndef PASCLANG_PARSING_PARSER_H
#define PASCLANG_PARSING_PARSER_H

#include "Parsing/Lexer.h"
#include "AST/AST.h"
#include "Message/BaseReporter.h"
#include <map>
#include <string>
#include <vector>

namespace pasclang::Parsing {

class Parser {
    public:
        typedef Token::TokenType TokenType;

    private:
        bool errorHappened = false;
        std::vector<Token> tokens;
        int currentToken = 0;
        Message::BaseReporter* reporter;
        AST::TableOfTypes* types = nullptr;

        // Token reading
        bool isAtEnd() { return this->peek().getType() == TokenType::ENDFILE; }
        const Token& advance()
        {
            if(!this->isAtEnd()) this->currentToken++;
            return this->previous();
        }
        const Token& peek() const { return this->tokens[currentToken]; }
        const Token& previous() const { return this->tokens[currentToken - 1]; }
        bool match(TokenType type)
        {
            if(this->check(type)) {
                this->advance();
                return true;
            }
            return false;
        }
        bool match(const std::vector<TokenType> tokens)
        {
            for(TokenType token : tokens)
            {
                if(this->match(token))
                    return true;
            }
            return false;
        }
        bool check(TokenType type) {
            if(this->isAtEnd()) return false;
            return this->peek().getType() == type;
        }
        void expect(TokenType token) {
            if(!this->match(token))
                this->syntaxError({token});
        }
        void expect(const std::vector<TokenType> tokens) {
            if(!this->match(tokens))
                this->syntaxError(tokens);
        }

        // Error handling and panic mode
        void syntaxError(const std::vector<TokenType> expectedTokens);
        void lookForAdditionalErrors();

        // Recursive descent
        std::unique_ptr<AST::Program> program(std::unique_ptr<AST::TableOfTypes>& types);
        std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>> localsDeclarations();
        std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>> formalsDeclarations();
        std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>> variableDeclaration();
        std::unique_ptr<AST::PrimitiveType> primitiveType();
        std::unique_ptr<AST::Procedure> procedure();
        std::unique_ptr<AST::Instruction> sequence();
        std::unique_ptr<AST::Instruction> instruction();
        std::unique_ptr<AST::Instruction> instructionWithIdentifier();
        std::unique_ptr<AST::Instruction> procedureCall();
        std::unique_ptr<AST::Instruction> variableAssignment();
        std::unique_ptr<AST::Instruction> arrayAssignment();
        std::unique_ptr<AST::Instruction> condition();
        std::unique_ptr<AST::Instruction> repetition();
        std::unique_ptr<AST::Expression> expression();
        std::unique_ptr<AST::Expression> logicalOr();
        std::unique_ptr<AST::Expression> logicalAnd();
        std::unique_ptr<AST::Expression> logicalUnary();
        std::unique_ptr<AST::Expression> equality();
        std::unique_ptr<AST::Expression> relational();
        std::unique_ptr<AST::Expression> additive();
        std::unique_ptr<AST::Expression> multiplicative();
        std::unique_ptr<AST::Expression> arithmeticUnary();
        std::unique_ptr<AST::Expression> postfix();
        std::unique_ptr<AST::Expression> primary();
        std::list<std::unique_ptr<AST::Expression>> actuals();

    public:
        Parser(Message::BaseReporter* reporter) : reporter(reporter) { }
        void swapTokensList(std::vector<Token>& tokens) { std::swap(this->tokens, tokens); }
        std::unique_ptr<AST::Program> parse() {
            std::unique_ptr<AST::TableOfTypes> tot = std::make_unique<AST::TableOfTypes>();
            this->types = tot.get();
            auto ast = this->program(tot);
            if(this->errorHappened)
                throw pasclang::PasclangException(ExitCode::SyntaxError);
            return ast;
        }
};

} // namespace pasclang::Parsing

#endif

