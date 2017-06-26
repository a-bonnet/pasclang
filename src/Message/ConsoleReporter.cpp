#include "Message/ConsoleReporter.h"
#include <iostream>

namespace pasclang::Message {

void ConsoleReporter::note(std::string message, const Parsing::Position* start, const Parsing::Position* end)
{
    std::string output = "note: ";
    if(start != nullptr)
    {
        if(end != nullptr)
        {
            if(start->getLine() == end->getLine())
                output += "at line " + std::to_string(start->getLine());
            else
                output += "starting from line " + std::to_string(start->getLine());
        }
        else
            output += "starting from line " + std::to_string(start->getLine());
    }
    else if(end != nullptr)
        output += "up to line " + std::to_string(end->getLine());

    output += "\n\t" + message + "\n";
    // print line(s) here
    output += "\n";

    std::cout << output;
}

void ConsoleReporter::warning(std::string message, const Parsing::Position* start, const Parsing::Position* end)
{
    std::string output = "warning: ";
    if(start != nullptr)
    {
        if(end != nullptr)
        {
            if(start->getLine() == end->getLine())
                output += "at line " + std::to_string(start->getLine());
            else
                output += "starting from line " + std::to_string(start->getLine());
        }
        else
            output += "starting from line " + std::to_string(start->getLine());
    }
    else if(end != nullptr)
        output += "up to line " + std::to_string(end->getLine());

    output += "\n\t" + message + "\n";
    // print line(s) here
    output += "\n";

    std::cout << output;
}

void ConsoleReporter::error(std::string message, const Parsing::Position* start, const Parsing::Position* end)
{
    std::string output = "error: ";
    if(start != nullptr)
    {
        if(end != nullptr)
        {
            if(start->getLine() == end->getLine())
                output += "at line " + std::to_string(start->getLine());
            else
                output += "from line " + std::to_string(start->getLine());
        }
        else
            output += "from line " + std::to_string(start->getLine());
    }
    else if(end != nullptr)
        output += "to line " + std::to_string(end->getLine());

    output += "\n\t" + message + "\n";

    std::fstream file;
    std::string buffer;
    file.open(this->getFileName(), file.in);
    char currentChar = 0;

    if(start != nullptr)
    {
        file.seekg(start->getBeginningOfLine(), file.beg);
        int lengthToEndOfLine = 0;

        do {
            currentChar = file.get();
            buffer += currentChar;
        } while(currentChar != '\n' && currentChar != EOF);

        file.seekg(start->getOffset(), file.beg);
        while(currentChar != '\n' && currentChar != EOF)
        {
            currentChar = file.get();
            lengthToEndOfLine++;
        }

        for(int i = 0 ; i < start->getOffset() - start->getBeginningOfLine() ; i++)
            buffer += ' ';
        buffer += '^';

        if(end != nullptr && end->getLine() == start->getLine())
        {
            for(int i = 0 ; i < end->getOffset() - start->getOffset() ; i++)
                buffer += '^';
        }

        file.close();
    }

    std::cout << output << '\n' << buffer << std::endl;
}

void ConsoleReporter::message(MessageType type, std::string message, const Parsing::Position* start, const Parsing::Position* end)
{
    switch(type)
    {
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

