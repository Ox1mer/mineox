#pragma once

#include "BlockRegistrarMacros.h"

#include "Block.h"

#include "AirBlock.h"
#include "DirtBlock.h"
#include "StoneBlock.h"
#include "GneissBlock.h"
#include "GravelBlock.h"
#include "MigmatiteBlock.h"
#include "SandBlock.h"
#include "SporeMossBlock.h"

REGISTER_BLOCK(Blocks::Air, AirBlock);
REGISTER_BLOCK(Blocks::Dirt, DirtBlock);
REGISTER_BLOCK(Blocks::Stone, StoneBlock);
REGISTER_BLOCK(Blocks::Gneiss, GneissBlock);
REGISTER_BLOCK(Blocks::Gravel, GravelBlock);
REGISTER_BLOCK(Blocks::Migmatite, MigmatiteBlock);
REGISTER_BLOCK(Blocks::Sand, SandBlock);
REGISTER_BLOCK(Blocks::SporeMoss, SporeMossBlock);