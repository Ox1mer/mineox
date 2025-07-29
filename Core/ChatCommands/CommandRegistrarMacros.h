#pragma once

#include "ChatCommandFactory.h"

#define REGISTER_CHAT_COMMAND(ID, CommandClass, ...) \
    namespace { \
        struct CommandClass##Register { \
            CommandClass##Register() { \
                ChatCommandFactory::getInstance().registerCommand( \
                    ID, \
                    []() { return std::make_unique<CommandClass>(__VA_ARGS__); } \
                ); \
            } \
        }; \
        static CommandClass##Register global_##CommandClass##Register; \
    }