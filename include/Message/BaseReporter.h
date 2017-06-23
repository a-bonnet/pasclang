#ifndef PASCLANG_MESSAGE_REPORTER_H
#define PASCLANG_MESSAGE_REPORTER_H

#include "Parsing/Location.h"
#include <string>
#include <fstream>

namespace pasclang::Message {

enum MessageType {
    Note,
    Warning,
    Error
};

class BaseReporter
{
    private:
        std::string file;
        std::fstream source;

    protected:
        std::fstream& getStream() { return this->source; }

    public:
        void openStream(std::string file) { this->source.open(file, this->source.in); this->file = file; }
        char readStream() { return this->source.get(); }
        char peekStream() { return this->source.peek(); }
        bool endOfStream() { return this->source.eof(); }
        void closeStream() { this->source.close(); }
        const std::string& getFileName() { return this->file; }
        BaseReporter() { }
        virtual ~BaseReporter() { }
        virtual void message(MessageType type, std::string message, const Parsing::Position* start, const Parsing::Position* end) = 0;
};

} // namespace pasclang::Message

#endif

