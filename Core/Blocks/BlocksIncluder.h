#pragma once

#include "Block.h"
#include "AirBlock.h"
#include "DirtBlock.h"
#include "BlockRegistrarMacros.h"

REGISTER_BLOCK(Blocks::Air, AirBlock);
REGISTER_BLOCK(Blocks::Dirt, DirtBlock);