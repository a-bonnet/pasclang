#ifndef PASCLANG_PARSING_LOCATION_H
#define PASCLANG_PARSING_LOCATION_H

#include <string>

namespace pasclang::Parsing {

class Position {
    private:
        int line;
        int bol; // beginning of line, used to quote file in error report
        int offset;
        std::string& file;

    public:
        Position(int line, int bol, int offset, std::string& file) :
            line(line), bol(bol), offset(offset), file(file) { }

        int getLine() const { return this->line; }
        int getBeginningOfLine() const { return this->bol; }
        int getOffset() const { return this->offset; }
        const std::string& getName() const { return this->file; }
};

class Location {
    private:
        Position start;
        Position end;

    public:
        Location(const Position start, const Position end) : start(start), end(end) { }

        const Position& getStart() const { return this->start; }
        const Position& getEnd() const { return this->end; }
};

}

#endif

