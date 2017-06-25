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
                output += "starting from line " + std::to_string(start->getLine());
        }
        else
            output += "starting from line " + std::to_string(start->getLine());
    }
    else if(end != nullptr)
        output += "up to line " + std::to_string(end->getLine());

    output += "\n\t" + message + "\n";

    std::fstream file;
    std::string buffer;
    file.open(this->getFileName(), file.in);
    char currentChar = 0;

    // Look into this
    std::cout << output << std::endl;
    return;

    file.seekg(start->getBeginningOfLine(), file.beg);

    if(start != nullptr)
    {
        int lengthToEndOfLine = 0;

        do {
            currentChar = file.get();
            buffer += currentChar;
        } while(currentChar != '\n' && !file.eof());

        file.seekg(start->getOffset(), file.beg);
        do {
            currentChar = file.get();
            lengthToEndOfLine++;
        } while(currentChar != '\n' && !file.eof());

        for(int i = 0 ; i < start->getOffset() - start->getBeginningOfLine() ; i++)
            buffer += ' ';
        buffer += '^';

        if(end != nullptr && end->getLine() == start->getLine())
        {
            for(int i = 0 ; i < end->getOffset() - start->getOffset() ; i++)
                buffer += '^';
        }
        if(end != nullptr && end->getLine() == start->getLine() + 1)
        {
            for(int i = 0 ; i < lengthToEndOfLine - 2 ; i++)
                buffer += '~';
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

