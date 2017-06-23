#ifndef PASCLANG_MESSAGE_CONSOLEREPORTER_H
#define PASCLANG_MESSAGE_CONSOLEREPORTER_H

#include "Message/BaseReporter.h"

namespace pasclang::Message {

class ConsoleReporter : public BaseReporter
{
    protected:
        virtual void note(std::string message, const Parsing::Position* start, const Parsing::Position* end);
        virtual void warning(std::string message, const Parsing::Position* start, const Parsing::Position* end);
        virtual void error(std::string message, const Parsing::Position* start, const Parsing::Position* end);

    public:
        ConsoleReporter() { }
        virtual ~ConsoleReporter() { }
        virtual void message(MessageType type, std::string message, const Parsing::Position* start, const Parsing::Position* end) override;
};

} // namespace pasclang::Message

#endif

