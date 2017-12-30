#include "Message/ConsoleReporter.h"
#include <iostream>
#include <sstream>

namespace pasclang::Message {

void ConsoleReporter::note(std::string message, const Parsing::Position* start,
                           const Parsing::Position* end) {
    std::ostringstream output;
    output << "note: ";
    if (start != nullptr) {
        if (end != nullptr) {
            if (start->getLine() == end->getLine())
                output << "at line " << start->getLine();
            else
                output << "starting from line " << start->getLine();
        } else
            output << "starting from line " << start->getLine();
    } else if (end != nullptr)
        output << "up to line " << end->getLine();

    output << "\n\t" << message << "\n";
    // print line(s) here
    output << "\n";

    std::cout << output.str() << std::endl;
}

void ConsoleReporter::warning(std::string message,
                              const Parsing::Position* start,
                              const Parsing::Position* end) {
    std::ostringstream output;
    output << "warning: ";
    if (start != nullptr) {
        if (end != nullptr) {
            if (start->getLine() == end->getLine())
                output << "at line " << start->getLine();
            else
                output << "starting from line " << start->getLine();
        } else
            output << "starting from line " << start->getLine();
    } else if (end != nullptr)
        output << "up to line " << end->getLine();

    output << "\n\t" << message << "\n";
    // print line(s) here
    output << "\n";

    std::cout << output.str() << std::endl;
}

void ConsoleReporter::error(std::string message, const Parsing::Position* start,
                            const Parsing::Position* end) {
    std::ostringstream output;
    output << "error: ";
    if (start != nullptr) {
        if (end != nullptr) {
            if (start->getLine() == end->getLine())
                output << "at line " << start->getLine();
            else
                output << "from line " << start->getLine();
        } else
            output << "from line " << start->getLine();
    } else if (end != nullptr)
        output << "to line " << end->getLine();

    output << "\n\t" << message << "\n";

    std::fstream file;
    file.open(this->getFileName(), file.in);
    char currentChar = 0;

    if (start != nullptr) {
        file.seekg(start->getBeginningOfLine(), file.beg);
        int lengthToEndOfLine = 0;

        do {
            currentChar = file.get();
            output << currentChar;
        } while (currentChar != '\n' && currentChar != EOF);

        file.seekg(start->getOffset(), file.beg);
        while (currentChar != '\n' && currentChar != EOF) {
            currentChar = file.get();
            lengthToEndOfLine++;
        }

        for (int i = 0; i < start->getOffset() - start->getBeginningOfLine();
             i++)
            output << ' ';
        output << '^';

        if (end != nullptr && end->getLine() == start->getLine()) {
            for (int i = 0; i < end->getOffset() - start->getOffset(); i++)
                output << '^';
        }

        file.close();
    }

    std::cout << output.str() << std::endl;
}

void ConsoleReporter::message(MessageType type, std::string message,
                              const Parsing::Position* start,
                              const Parsing::Position* end) {
    switch (type) {
    case MessageType::Note:
        this->note(message, start, end);
        break;
    case MessageType::Warning:
        this->warning(message, start, end);
        break;
    case MessageType::Error:
        this->error(message, start, end);
    }
}

} // namespace pasclang::Message
