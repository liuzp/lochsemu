#include "stdafx.h"
#include "disassembler.h"

#include "processor.h"
#include "instruction.h"
#include "memory.h"

Disassembler::Disassembler()
{
    //m_ptrChangeHandler  = nullptr;
    m_dataUpdateHandler = nullptr;
    m_lastSec = NULL;
}

Disassembler::~Disassembler()
{

}

void Disassembler::OnPreExecute( const Processor *cpu, const Instruction *inst )
{
    u32 eip = cpu->EIP;
    Section *sec = cpu->Mem->GetSection(eip);
    if (m_secMap.find(sec) == m_secMap.end()) {
        m_secMap[sec] = InstDisasmMap();
    }
    bool update = false;
    const InstDisasmMap &instMap = m_secMap[sec];
    if (instMap.find(eip) == instMap.end()) {
        LxDebug("Disassembling...\n");
        RecursiveDisassemble(cpu, eip, sec, eip);

        int count = 0;
        for (auto &iter : m_secMap) 
            count += iter.second.size();
        LxDebug("Disassemble complete, count = %d\n", count);
        update = true;
    }
    if (m_lastSec != sec || update) {
        if (m_dataUpdateHandler) m_dataUpdateHandler(&instMap);
    }
    m_lastSec = sec;
}

void Disassembler::RecursiveDisassemble( const Processor *cpu, u32 eip, const Section *sec, u32 entryEip )
{
    Assert(sec);
    Assert(entryEip != 0);

    while (true) {
        InstDisasmMap &instMap = m_secMap[sec];
        if (instMap.find(eip) != instMap.end()) return; // already disassembled

        InstPtr inst(new Instruction);
        LxDecode(cpu->Mem->GetRawData(eip), inst.get(), eip);
        instMap[eip] = Inst(inst, eip);
        AttachApiInfo(cpu, eip, sec, instMap[eip]);
        //LxInfo("%08x  %s\n", eip, inst->Main.CompleteInstr);

        u32 opcode = inst->Main.Inst.Opcode;
        if (opcode == 0xc3 || opcode == 0xcb || opcode == 0xc2 || opcode == 0xca) {
            // 'ret' is met
            instMap[eip].entry = entryEip;
            return;
        }

        u32 addrValue = (u32) inst->Main.Inst.AddrValue;
        const char *mnemonics = inst->Main.Inst.Mnemonic;
        if (addrValue != 0) {
            if (OPERAND_TYPE(inst->Main.Argument1.ArgType) == CONSTANT_TYPE) {
                if (cpu->Mem->GetSection(addrValue)) {
                    u32 nextEntry = Instruction::IsCall(inst.get()) ? addrValue : entryEip;
                    RecursiveDisassemble(cpu, addrValue, cpu->Mem->GetSection(addrValue), nextEntry);
                }
            }
        }
        u32 nextEip = eip + inst->Length;
        const Section *nextSec = cpu->Mem->GetSection(nextEip);
        if (sec != nextSec) return;
        eip = nextEip;
    }
}

void Disassembler::AttachApiInfo( const Processor *cpu, u32 eip, const Section *sec, Inst &inst )
{
    u32 target = 0;
    u32 opcode = inst.ptr->Main.Inst.Opcode;
    const char *mnemonic = inst.ptr->Main.Inst.Mnemonic;
    if (opcode == 0xff) {
        // CALL or JMP r/m32
        if (strstr(mnemonic, "jmp") == mnemonic || Instruction::IsCall(inst.ptr.get())) {
            if (OPERAND_TYPE(inst.ptr->Main.Argument1.ArgType) == MEMORY_TYPE &&
                inst.ptr->Main.Argument1.Memory.BaseRegister == 0 &&
                inst.ptr->Main.Argument1.Memory.IndexRegister == 0) {
                target = cpu->ReadOperand32(inst.ptr.get(), inst.ptr->Main.Argument1, NULL);
            }
        }
    } else if (opcode == 0xe8) {
        target = (u32) inst.ptr->Main.Inst.AddrValue;
        
        if (cpu->Mem->GetSection(target))
            RecursiveDisassemble(cpu, target, cpu->Mem->GetSection(target), target);
        auto iter = m_secMap[sec].find(target);
        Assert(iter != m_secMap[sec].end());
        const Instruction &instCalled = *iter->second.ptr;
        if (instCalled.Main.Inst.Opcode == 0xff &&
            (strstr(instCalled.Main.Inst.Mnemonic, "jmp") == instCalled.Main.Inst.Mnemonic)) 
        {
            target = cpu->ReadOperand32(&instCalled, instCalled.Main.Argument1, NULL);
        }
    } else if (*mnemonic == 'j') {
        target = (u32) inst.ptr->Main.Inst.AddrValue;
    }

    if (target) {
        inst.target = target;
    }

    const ApiInfo *info = cpu->Proc()->GetApiInfoFromAddress(target);
    if (info) {
        inst.dllName = info->ModuleName;
        inst.funcName = info->FunctionName;
    }
}

