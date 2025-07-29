#pragma once

enum class ChatCommandID {
    Help,
    Clear,
    Tp
};

static const std::unordered_map<std::string, ChatCommandID> ChatCommandNameMap = {
    {"tp", ChatCommandID::Tp},
    {"help", ChatCommandID::Help},
    {"clear", ChatCommandID::Clear},
};
