#pragma once

#ifndef __ARIETIS_DEBUGGER_H__
#define __ARIETIS_DEBUGGER_H__

#include "Arietis.h"
#include "parallel.h"
#include "utilities.h"
#include "breakpoint.h"

class ADebugger : public ISerializable {
public:
    enum State {
        STATE_RUNNING,
        STATE_SINGLESTEP,
        STATE_STEPOVER,
        STATE_STEPOUT,
        STATE_TERMINATED,
    };
public:
    ADebugger(AEngine *engine);
    ~ADebugger();

    void        Initialize();
    void        OnPreExecute(PreExecuteEvent &event);
    //void        OnPostExecute(Processor *cpu, const Instruction *inst);
    void        OnProcessPreRun(ProcessPreRunEvent &event);
    void        OnProcessPostLoad(ProcessPostLoadEvent &event);
    void        OnStepInto();
    void        OnStepOver();
    void        OnStepOut();
    void        OnRun();
    void        AddBreakpoint(u32 eip, const std::string &desc);
    void        ToggleBreakpoint(u32 eip);
    void        RemoveBreakpoint(u32 eip);
    const Breakpoint * GetBreakpoint(u32 eip) const;
    const Breakpoint & GetBreakpointIndex(int index) const { return m_breakpoints[index]; }
    int         GetNumBreakpoints() const { return (int) m_breakpoints.size(); }
    void        OnTerminate();

    void        UpdateInstContext(InstContext *ctx) const;
    void        UpdateTraceContext(TraceContext *ctx, u32 eip) const;

    void        Serialize(Json::Value &root) const override;
    void        Deserialize(Json::Value &root) override;
private:
    void        DoPreExecSingleStep(const Processor *cpu, const Instruction *inst);
    void        CheckBreakpoints(const Processor *cpu, const Instruction *inst);
    //void        AnalyzeCRTEntry(const Processor *cpu, const Instruction *inst);
private:
    State               m_state;
    u32                 m_stepOverEip;

    AEngine *           m_engine;
    Archive *           m_archive;

    Semaphore           m_semaphore;

    const Processor *   m_currProcessor;
    const Instruction * m_currInst;
    bool                m_crtEntryFound;
    u32                 m_mainEntry;

    std::vector<Breakpoint>     m_breakpoints;
};

#endif // __ARIETIS_DEBUGGER_H__
