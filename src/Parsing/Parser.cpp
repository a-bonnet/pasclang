#include <Parsing/Parser.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <stdexcept>

#include "Pasclang.h"
#include <AST/Types.h>
#include <Semantic/TypeChecker.h>

namespace pasclang::Parsing {

static std::map<Token::TokenType, AST::EBinaryOperation::Type>
    binaryOperatorToAst = {
        {Token::TokenType::PLUS, AST::EBinaryOperation::Type::BinaryAddition},
        {Token::TokenType::MINUS,
         AST::EBinaryOperation::Type::BinarySubtraction},
        {Token::TokenType::STAR,
         AST::EBinaryOperation::Type::BinaryMultiplication},
        {Token::TokenType::SLASH, AST::EBinaryOperation::Type::BinaryDivision},
        {Token::TokenType::LEQUAL,
         AST::EBinaryOperation::Type::BinaryLogicalLessEqual},
        {Token::TokenType::LTHAN,
         AST::EBinaryOperation::Type::BinaryLogicalLessThan},
        {Token::TokenType::GEQUAL,
         AST::EBinaryOperation::Type::BinaryLogicalGreaterEqual},
        {Token::TokenType::GTHAN,
         AST::EBinaryOperation::Type::BinaryLogicalGreaterThan},
        {Token::TokenType::AND, AST::EBinaryOperation::Type::BinaryLogicalAnd},
        {Token::TokenType::OR, AST::EBinaryOperation::Type::BinaryLogicalOr},
        {Token::TokenType::EQUAL, AST::EBinaryOperation::Type::BinaryEquality},
        {Token::TokenType::NEQUAL,
         AST::EBinaryOperation::Type::BinaryNonEquality}};

static std::map<Token::TokenType, AST::EUnaryOperation::Type>
    unaryOperatorToAst = {
        {Token::TokenType::MINUS, AST::EUnaryOperation::Type::UnaryMinus},
        {Token::TokenType::NOT, AST::EUnaryOperation::Type::UnaryNot}};

static std::map<Token::TokenType, AST::TableOfTypes::TypeKind> typeToAst = {
    {Token::TokenType::INTTYPE, AST::TableOfTypes::TypeKind::Integer},
    {Token::TokenType::BOOLTYPE, AST::TableOfTypes::TypeKind::Boolean}};

void Parser::syntaxError(const std::vector<TokenType> expectedTokens) {
    std::string errorMessage =
        "unexpected token " + tokenToString(this->peek().getType());
    if (expectedTokens.size() > 0) {
        errorMessage += " when expecting any of the following: ";
        for (TokenType token : expectedTokens) {
            errorMessage += tokenToString(token);
            if (token != expectedTokens.back())
                errorMessage += ", ";
        }
    }

    // Unexpected EOF while parsing
    if (this->peek().getType() == TokenType::ENDFILE)
        this->reporter->message(Message::MessageType::Error, errorMessage,
                                nullptr, nullptr);
    // Wrong syntax
    else
        this->reporter->message(Message::MessageType::Error, errorMessage,
                                &this->peek().getLocation().getStart(),
                                &this->peek().getLocation().getEnd());

    if (!this->errorHappened) {
        this->errorHappened = true;
        std::string noteMessage =
            "Pasclang will now look for additional syntax errors. However ";
        noteMessage += "since the input already contains an error, some "
                       "reports may be wrong.";
        this->reporter->message(Message::MessageType::Note, noteMessage,
                                nullptr, nullptr);
    }

    // Panic mode: try to find some more errors before exiting
    while (this->peek().getType() != TokenType::ENDFILE) {
        if (this->match({TokenType::BEGIN, TokenType::DO, TokenType::THEN,
                         TokenType::ELSE}))
            this->instruction();
        else if (this->match({TokenType::COLON, TokenType::NEW}))
            this->primitiveType();
        else if (this->match({TokenType::WHILE, TokenType::IF,
                              TokenType::ASSIGN, TokenType::LEFTPAR,
                              TokenType::LEFTBRACK}))
            this->expression();
        else if (this->match({TokenType::FUNCTION, TokenType::PROCEDURE}))
            this->procedure();
        else if (this->match(TokenType::VAR))
            this->variableDeclaration();

        this->advance();
    }

    throw PasclangException(ExitCode::SyntaxError);
}

// program = PROGRAM [VAR declaration {declarations}] {procedure} sequence DOT
std::unique_ptr<AST::Program>
Parser::program(std::unique_ptr<AST::TableOfTypes>& types) {
    std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>>
        globals;
    std::list<std::unique_ptr<AST::Procedure>> procedures;
    std::unique_ptr<AST::Instruction> main;

    this->expect(TokenType::PROGRAM);

    Parsing::Position start = this->previous().getLocation().getStart();

    if (this->match(TokenType::VAR))
        globals = this->localsDeclarations();

    while (this->match({Token::PROCEDURE, Token::FUNCTION}))
        procedures.push_back(this->procedure());

    main = this->sequence();

    this->expect(TokenType::DOT);

    Parsing::Position end = this->previous().getLocation().getEnd();

    std::unique_ptr<Parsing::Location> location =
        std::make_unique<Parsing::Location>(start, end);
    return std::make_unique<AST::Program>(globals, procedures, main, location,
                                          types);
}

// declarations = {declaration SEMICOLON}
std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>>
Parser::localsDeclarations() {
    std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>>
        bindings;

    this->expect(TokenType::IDENTIFIER);

    do {
        bindings.splice(bindings.end(), this->variableDeclaration());
        this->expect(TokenType::SEMICOLON);
    } while (this->match(TokenType::IDENTIFIER));

    return bindings;
}

// formals = [formal {COMMA formal}]
// formal = declaration
std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>>
Parser::formalsDeclarations() {
    std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>>
        bindings;

    if (this->match(TokenType::IDENTIFIER))
        bindings.splice(bindings.end(), this->variableDeclaration());

    while (this->match(TokenType::SEMICOLON)) {
        this->expect(TokenType::IDENTIFIER);
        bindings.splice(bindings.end(), this->variableDeclaration());
    }

    return bindings;
}

// declaration = IDENTIFIER {COMMA IDENTIFIER} COLON type
std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>>
Parser::variableDeclaration() {
    std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>>
        declarationList;
    std::list<std::string> identifiers;

    identifiers.push_back(this->previous().getLiteral());
    while (this->match(TokenType::COMMA)) {
        this->expect(TokenType::IDENTIFIER);
        identifiers.push_back(this->previous().getLiteral());
    }

    this->expect(TokenType::COLON);

    std::unique_ptr<AST::PrimitiveType> type = this->primitiveType();

    for (std::string identifier : identifiers) {
        std::unique_ptr<Location> location = std::make_unique<Location>(
            type->getLocation()->getStart(), type->getLocation()->getEnd());
        std::unique_ptr<AST::PrimitiveType> pairType =
            std::make_unique<AST::PrimitiveType>(type->getType(), location);
        declarationList.push_back(
            std::make_pair(identifier, std::move(pairType)));
    }

    return declarationList;
}

// type = {ARRAY OF} TYPE
std::unique_ptr<AST::PrimitiveType> Parser::primitiveType() {
    // In case we declare an array
    std::unique_ptr<AST::PrimitiveType> result(nullptr);
    std::unique_ptr<Location> location(nullptr);
    const AST::TableOfTypes::Type* type;

    this->expect({TokenType::INTTYPE, TokenType::BOOLTYPE, TokenType::ARRAY});

    Position start = this->previous().getLocation().getStart();
    Position end = this->previous().getLocation().getEnd();

    switch (this->previous().getType()) {
    case TokenType::INTTYPE:
    case TokenType::BOOLTYPE:
        location = std::make_unique<Location>(start, end);
        type = this->types->get(typeToAst[this->previous().getType()], 0);
        return std::make_unique<AST::PrimitiveType>(type, location);
    case TokenType::ARRAY:
        this->expect(TokenType::OF);
        result = this->primitiveType();
        result->increaseDimension();
        return result;
    default: // Should never happen
        this->syntaxError({});
    }
    return result;
}

// procedure = function | voidprocedure
// function = FUNCTION IDENTIFIER LEFTPAR formals RIGHTPAR COLON type SEMICOLON
// [VAR declarations] sequence SEMICOLON voidprocedure = PROCEDURE IDENTIFIER
// LEFTPAR formals RIGHTPAR SEMICOLON [VAR declarations] sequence SEMICOLON
std::unique_ptr<AST::Procedure> Parser::procedure() {
    bool isFunction;

    std::unique_ptr<AST::PrimitiveType> result(nullptr);
    std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>>
        locals;

    Position start = this->previous().getLocation().getStart();

    if (this->previous().getType() == TokenType::FUNCTION)
        isFunction = true;
    else
        isFunction = false;

    this->expect(TokenType::IDENTIFIER);
    std::string name = this->previous().getLiteral();
    this->expect(TokenType::LEFTPAR);
    std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>>
        formals = this->formalsDeclarations();
    this->expect(TokenType::RIGHTPAR);

    if (isFunction) {
        this->expect(TokenType::COLON);
        result = this->primitiveType();
    }

    this->expect(TokenType::SEMICOLON);

    if (this->match(TokenType::VAR)) {
        if (!this->check(TokenType::IDENTIFIER))
            this->syntaxError({TokenType::IDENTIFIER});
        locals = this->localsDeclarations();
    }

    std::unique_ptr<AST::Instruction> body = this->sequence();

    this->expect(TokenType::SEMICOLON);

    Position end = this->previous().getLocation().getEnd();
    std::unique_ptr<Location> location = std::make_unique<Location>(start, end);

    return std::make_unique<AST::Procedure>(name, formals, result, locals, body,
                                            location);
}

// sequence = BEGIN [instruction {SEMICOLON instruction}] END
std::unique_ptr<AST::Instruction> Parser::sequence() {
    std::list<std::unique_ptr<AST::Instruction>> instructions;

    this->expect(TokenType::BEGIN);

    Position start = this->previous().getLocation().getStart();

    if (!this->check(TokenType::END)) {
        do {
            instructions.push_back(this->instruction());
        } while (this->match(TokenType::SEMICOLON));
    }

    this->expect(TokenType::END);

    Position end = this->previous().getLocation().getEnd();
    std::unique_ptr<Location> location = std::make_unique<Location>(start, end);

    std::unique_ptr<AST::Instruction> result =
        std::make_unique<AST::ISequence>(instructions, location);
    return result;
}

// instruction = sequence | condition | repetition | call | variableassignment |
// arrayassignment
std::unique_ptr<AST::Instruction> Parser::instruction() {
    if (this->check(Token::BEGIN))
        return this->sequence();
    if (this->check(Token::IF))
        return this->condition();
    if (this->check(Token::WHILE))
        return this->repetition();
    if (this->check(Token::IDENTIFIER))
        return this->instructionWithIdentifier();

    this->syntaxError(
        {Token::BEGIN, Token::IF, Token::WHILE, Token::IDENTIFIER});
    return std::unique_ptr<AST::Instruction>(nullptr);
}

// condition = IF expression THEN instruction [ELSE instruction]
std::unique_ptr<AST::Instruction> Parser::condition() {
    std::unique_ptr<AST::Expression> condition(nullptr);
    std::unique_ptr<AST::Instruction> branchThen(nullptr);
    std::unique_ptr<AST::Instruction> branchElse(nullptr);

    Position start = this->peek().getLocation().getStart();

    this->expect(TokenType::IF);
    condition = this->expression();

    this->expect(TokenType::THEN);
    branchThen = this->instruction();

    if (this->match(TokenType::ELSE))
        branchElse = this->instruction();

    Position end = this->previous().getLocation().getEnd();
    std::unique_ptr<Location> location = std::make_unique<Location>(start, end);

    std::unique_ptr<AST::Instruction> result =
        std::make_unique<AST::ICondition>(condition, branchThen, branchElse,
                                          location);
    return result;
}

// repetition = WHILE expression DO instruction
std::unique_ptr<AST::Instruction> Parser::repetition() {
    std::unique_ptr<AST::Expression> condition;
    std::unique_ptr<AST::Instruction> instruction;

    Position start = this->peek().getLocation().getStart();

    this->expect(TokenType::WHILE);
    condition = this->expression();

    this->expect(TokenType::DO);
    instruction = this->instruction();

    Position end = this->previous().getLocation().getEnd();
    std::unique_ptr<Location> location = std::make_unique<Location>(start, end);

    std::unique_ptr<AST::Instruction> result =
        std::make_unique<AST::IRepetition>(condition, instruction, location);
    return result;
}

// see instruction
std::unique_ptr<AST::Instruction> Parser::instructionWithIdentifier() {
    this->expect(TokenType::IDENTIFIER);

    if (this->check(TokenType::LEFTPAR))
        return this->procedureCall();
    if (this->check(TokenType::ASSIGN))
        return this->variableAssignment();
    // Otherwise backtracking for array assignment instruction
    this->currentToken--;
    return this->arrayAssignment();
}

// variableassignment = IDENTIFIER ASSIGN expression
std::unique_ptr<AST::Instruction> Parser::variableAssignment() {
    std::string identifier = this->previous().getLiteral();
    std::unique_ptr<AST::Expression> assignment;

    Position start = this->previous().getLocation().getStart();

    this->expect(TokenType::ASSIGN);

    assignment = this->expression();

    Position end = this->previous().getLocation().getEnd();
    std::unique_ptr<Location> location = std::make_unique<Location>(start, end);

    std::unique_ptr<AST::Instruction> result =
        std::make_unique<AST::IVariableAssignment>(identifier, assignment,
                                                   location);
    return result;
}

// arrayassignment = arrayaccess ASSIGN expression
// We reuse the postfix node so we have to cast the parsed expression
std::unique_ptr<AST::Instruction> Parser::arrayAssignment() {
    Position start = this->peek().getLocation().getStart();

    std::unique_ptr<AST::Expression> expression = this->postfix();
    AST::EArrayAccess* array =
        dynamic_cast<AST::EArrayAccess*>(expression.get());
    if (array == nullptr)
        this->syntaxError({});
    this->expect(TokenType::ASSIGN);
    std::unique_ptr<AST::Expression> value = this->expression();
    Position end = this->previous().getLocation().getEnd();

    std::unique_ptr<Location> location = std::make_unique<Location>(start, end);
    return std::make_unique<AST::IArrayAssignment>(expression, value, location);
}

// call = IDENTIFIER LEFTPAR [expression {COMMA expression}] RIGHTPAR
std::unique_ptr<AST::Instruction> Parser::procedureCall() {
    std::string identifier = this->previous().getLiteral();
    std::list<std::unique_ptr<AST::Expression>> actuals;

    Position start = this->previous().getLocation().getStart();

    this->expect(TokenType::LEFTPAR);

    actuals = this->actuals();

    this->expect(TokenType::RIGHTPAR);

    Position end = this->previous().getLocation().getEnd();
    std::unique_ptr<Location> location = std::make_unique<Location>(start, end);

    std::unique_ptr<AST::Instruction> result =
        std::make_unique<AST::IProcedureCall>(identifier, actuals, location);
    return result;
}

// expression = arrayallocation | logicalor
std::unique_ptr<AST::Expression> Parser::expression() {
    if (this->match(TokenType::NEW)) {
        Position start = this->previous().getLocation().getStart();
        std::unique_ptr<AST::PrimitiveType> type = this->primitiveType();
        this->expect(TokenType::LEFTBRACK);
        std::unique_ptr<AST::Expression> expression = this->expression();
        this->expect(TokenType::RIGHTBRACK);
        Position end = this->previous().getLocation().getEnd();
        std::unique_ptr<Location> location =
            std::make_unique<Location>(start, end);
        return std::make_unique<AST::EArrayAllocation>(type, expression,
                                                       location);
    }
    return this->logicalOr();
}

// logicalor = logicaland {OR logicaland}
std::unique_ptr<AST::Expression> Parser::logicalOr() {
    Position start = this->peek().getLocation().getStart();

    std::unique_ptr<AST::Expression> expression = this->logicalAnd();

    while (this->match(TokenType::OR)) {
        TokenType type = this->previous().getType();
        std::unique_ptr<AST::Expression> rhs = this->logicalAnd();
        Position end = this->previous().getLocation().getEnd();
        std::unique_ptr<Location> location =
            std::make_unique<Location>(start, end);
        expression = std::make_unique<AST::EBinaryOperation>(
            binaryOperatorToAst[type], expression, rhs, location);
    }

    return expression;
}

// logicaland = logicalunary {AND logicalunary}
std::unique_ptr<AST::Expression> Parser::logicalAnd() {
    Position start = this->peek().getLocation().getStart();
    std::unique_ptr<AST::Expression> expression = this->logicalUnary();

    while (this->match(TokenType::AND)) {
        TokenType type = this->previous().getType();
        std::unique_ptr<AST::Expression> rhs = this->logicalUnary();
        Position end = this->previous().getLocation().getEnd();
        std::unique_ptr<Location> location =
            std::make_unique<Location>(start, end);
        expression = std::make_unique<AST::EBinaryOperation>(
            binaryOperatorToAst[type], expression, rhs, location);
    }

    return expression;
}

// logicalunary = (NOT) equality
std::unique_ptr<AST::Expression> Parser::logicalUnary() {
    Position start = this->peek().getLocation().getStart();

    if (this->match(TokenType::NOT)) {
        std::unique_ptr<AST::Expression> expression;
        expression = this->equality();
        Position end = this->previous().getLocation().getEnd();
        std::unique_ptr<Location> location =
            std::make_unique<Location>(start, end);
        expression = std::make_unique<AST::EUnaryOperation>(
            unaryOperatorToAst[TokenType::NOT], expression, location);
        return expression;
    } else
        return this->equality();
}

// equality = relational [EQUAL relational]
std::unique_ptr<AST::Expression> Parser::equality() {
    Position start = this->peek().getLocation().getStart();
    std::unique_ptr<AST::Expression> expression = this->relational();

    if (this->match({TokenType::EQUAL, TokenType::NEQUAL})) {
        TokenType type = this->previous().getType();
        std::unique_ptr<AST::Expression> rhs = this->relational();
        Position end = this->previous().getLocation().getEnd();
        std::unique_ptr<Location> location =
            std::make_unique<Location>(start, end);
        expression = std::make_unique<AST::EBinaryOperation>(
            binaryOperatorToAst[type], expression, rhs, location);
    }

    return expression;
}

// relational = additive [(GTHAN | GEQUAL | LTHAN | LEQUAL) additive]
std::unique_ptr<AST::Expression> Parser::relational() {
    Position start = this->peek().getLocation().getStart();
    std::unique_ptr<AST::Expression> expression = this->additive();

    if (this->match({TokenType::GTHAN, TokenType::GEQUAL, TokenType::LTHAN,
                     TokenType::LEQUAL})) {
        TokenType type = this->previous().getType();
        std::unique_ptr<AST::Expression> rhs = this->additive();
        Position end = this->previous().getLocation().getEnd();
        std::unique_ptr<Location> location =
            std::make_unique<Location>(start, end);
        expression = std::make_unique<AST::EBinaryOperation>(
            binaryOperatorToAst[type], expression, rhs, location);
    }

    return expression;
}

// additive = multiplicative {(PLUS | MINUS) multiplicative}
std::unique_ptr<AST::Expression> Parser::additive() {
    Position start = this->peek().getLocation().getStart();
    std::unique_ptr<AST::Expression> expression = this->multiplicative();

    while (this->match({TokenType::PLUS, TokenType::MINUS})) {
        TokenType type = this->previous().getType();
        std::unique_ptr<AST::Expression> rhs = this->multiplicative();
        Position end = this->previous().getLocation().getEnd();
        std::unique_ptr<Location> location =
            std::make_unique<Location>(start, end);
        expression = std::make_unique<AST::EBinaryOperation>(
            binaryOperatorToAst[type], expression, rhs, location);
    }

    return expression;
}

// multiplicative = arithmeticunary {(STAR SLASH) arithmeticunary}
std::unique_ptr<AST::Expression> Parser::multiplicative() {
    Position start = this->peek().getLocation().getStart();
    std::unique_ptr<AST::Expression> expression = this->arithmeticUnary();

    while (this->match({TokenType::STAR, TokenType::SLASH})) {
        TokenType type = this->previous().getType();
        std::unique_ptr<AST::Expression> rhs = this->arithmeticUnary();
        Position end = this->previous().getLocation().getEnd();
        std::unique_ptr<Location> location =
            std::make_unique<Location>(start, end);
        expression = std::make_unique<AST::EBinaryOperation>(
            binaryOperatorToAst[type], expression, rhs, location);
    }

    return expression;
}

// arithmeticunary = (MINUS) postfix
std::unique_ptr<AST::Expression> Parser::arithmeticUnary() {
    Position start = this->peek().getLocation().getStart();
    if (this->match(TokenType::MINUS)) {
        std::unique_ptr<AST::Expression> expression;
        expression = this->postfix();
        Position end = this->previous().getLocation().getEnd();
        std::unique_ptr<Location> location =
            std::make_unique<Location>(start, end);
        expression = std::make_unique<AST::EUnaryOperation>(
            unaryOperatorToAst[TokenType::MINUS], expression, location);
        return expression;
    } else
        return this->postfix();
}

// postfix = primary (LEFTPAR actuals RIGHTPAR) | (LEFTBRACK expression
// RIGHTBRACK {LEFTBRACK expression RIGHTBRACK})
std::unique_ptr<AST::Expression> Parser::postfix() {
    Position start = this->peek().getLocation().getStart();
    std::unique_ptr<AST::Expression> primary = this->primary();
    std::string literal = this->previous().getLiteral();

    if (this->match(TokenType::LEFTPAR)) {
        std::list<std::unique_ptr<AST::Expression>> arguments = this->actuals();
        this->expect(TokenType::RIGHTPAR);
        Position end = this->previous().getLocation().getEnd();
        std::unique_ptr<Location> location =
            std::make_unique<Location>(start, end);
        return std::make_unique<AST::EFunctionCall>(literal, arguments,
                                                    location);
    }
    if (this->match(TokenType::LEFTBRACK)) {
        std::unique_ptr<AST::Expression> array(nullptr);
        std::unique_ptr<AST::Expression> index = this->expression();
        Position end = this->previous().getLocation().getEnd();
        std::unique_ptr<Location> location =
            std::make_unique<Location>(start, end);
        array = std::make_unique<AST::EArrayAccess>(primary, index, location);

        this->expect(TokenType::RIGHTBRACK);

        while (this->match(TokenType::LEFTBRACK)) {
            index = this->expression();
            this->expect(TokenType::RIGHTBRACK);
            Position end = this->previous().getLocation().getEnd();
            std::unique_ptr<Location> location =
                std::make_unique<Location>(start, end);
            array = std::make_unique<AST::EArrayAccess>(array, index, location);
        }

        return array;
    }

    return primary;
}

// primary = INTLITERAL | BOOLLITERAL | IDENTIFIER | LEFTPAR expression RIGHTPAR
std::unique_ptr<AST::Expression> Parser::primary() {
    Position start = this->peek().getLocation().getStart();
    std::unique_ptr<Location> location(nullptr);
    this->expect({TokenType::BOOLLITERAL, TokenType::INTLITERAL,
                  TokenType::IDENTIFIER, TokenType::LEFTPAR});
    std::string identifier;
    std::unique_ptr<AST::Expression> expression;

    Position end = this->peek().getLocation().getEnd();

    switch (this->previous().getType()) {
    case TokenType::BOOLLITERAL:
        this->previous().getLocation().getEnd();
        location = std::make_unique<Location>(start, end);
        return std::make_unique<AST::ECBoolean>(
            this->previous().getLiteral() == "true" ? true : false, location);
    case TokenType::INTLITERAL:
        location = std::make_unique<Location>(start, end);
        return std::make_unique<AST::ECInteger>(
            std::stoi(this->previous().getLiteral()), location);
    case TokenType::IDENTIFIER:
        identifier = this->previous().getLiteral();
        location = std::make_unique<Location>(start, end);
        return std::make_unique<AST::EVariableAccess>(identifier, location);
    case TokenType::LEFTPAR:
        expression = this->expression();
        this->expect(TokenType::RIGHTPAR);
        return expression;
    default:
        this->syntaxError({TokenType::BOOLLITERAL, TokenType::INTLITERAL,
                           TokenType::IDENTIFIER, TokenType::LEFTPAR});
    }
    return expression;
}

// actuals = [expression {COMMA expression}]
std::list<std::unique_ptr<AST::Expression>> Parser::actuals() {
    std::list<std::unique_ptr<AST::Expression>> result;

    if (!this->check(TokenType::RIGHTPAR)) {
        do {
            result.push_back(this->expression());
        } while (this->match(TokenType::COMMA));
    }

    return result;
}

} // namespace pasclang::Parsing
