#pragma once

enum class ChatCommandID {
    Help,
    Clear,
    Tp,
    ChooseBlock
};

static const std::unordered_map<std::string, ChatCommandID> ChatCommandNameMap = {
    {"tp", ChatCommandID::Tp},
    {"help", ChatCommandID::Help},
    {"clear", ChatCommandID::Clear},
    {"chblock", ChatCommandID::ChooseBlock},
};
