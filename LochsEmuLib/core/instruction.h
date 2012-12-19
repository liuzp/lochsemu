#pragma once

#ifndef __CORE_INSTRUCTION_H__
#define __CORE_INSTRUCTION_H__

#include "lochsemu.h"

#include "3rdparty/libdasm/libdasm.h"

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "3rdparty/BeaEngine/BeaEngine.h"

BEGIN_NAMESPACE_LOCHSEMU()

class LX_API Instruction {
public:
    Instruction() {
        ZeroMemory(&Main, sizeof(DISASM));
        ZeroMemory(&Aux, sizeof(INSTRUCTION));
        Length = 0;
    }

    DISASM          Main;  /* BeaEngine */

    INSTRUCTION     Aux;   /* Libdasm */

    int             Length;

}; // class Instruction


END_NAMESPACE_LOCHSEMU()

#endif // __CORE_INSTRUCTION_H__
