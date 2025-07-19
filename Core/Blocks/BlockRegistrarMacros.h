#pragma once

#include "BlockFactory.h"

#define REGISTER_BLOCK(id, className)                         \
    namespace {                                               \
        struct className##Registrar {                         \
            className##Registrar() {                          \
                BlockFactory::getInstance().registerBlock(    \
                    id, []() {                                \
                        return std::make_unique<className>();\
                    }                                         \
                );                                            \
            }                                                 \
        };                                                    \
        static className##Registrar global_##className##Registrar; \
    }
