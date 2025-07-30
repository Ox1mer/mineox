#pragma once

#include "CommandRegistrarMacros.h"
#include "ServiceLocator.h"

#include "IChatCommand.h"
#include "ClearCommand.h"
#include "TpCommand.h"
#include "ChooseBlockCommand.h"

REGISTER_CHAT_COMMAND(ChatCommandID::Clear, ClearCommand, *ServiceLocator::GetChatController());
REGISTER_CHAT_COMMAND(ChatCommandID::Tp, TpCommand, *ServiceLocator::GetChatController());
REGISTER_CHAT_COMMAND(ChatCommandID::ChooseBlock, ChooseBlockCommand, *ServiceLocator::GetChatController());