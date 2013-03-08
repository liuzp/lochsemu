#pragma once
 
#ifndef __PROPHET_GUI_MAINFRAME_H__
#define __PROPHET_GUI_MAINFRAME_H__
 
#include "Prophet.h"
#include "gui.h"

class ProphetFrame : public wxFrame {
public:
    ProphetFrame(ProEngine *engine, Emulator *emu);
    ~ProphetFrame();


    void    PreExecSingleStepCallback(const Processor *cpu, const Instruction *inst);

    CpuPanel *              GetCpuPanel() const { return m_cpuPanel; }
    ContextPanel *          GetRegPanel() const { return m_contextPanel; }
    CompositeTracePanel *   GetTracePanel() const { return m_tracePanel; }
    MemInfoPanel *          GetMemInfoPanel() const { return m_memInfoPanel; }
    void    ReportBusy(bool isBusy);
    void    OnProcessLoaded(LPCSTR path);
    void    OnArchiveLoaded(Archive *arc);

    void    ShowInMemory(u32 addr);
private:
    void    OnExit(wxCommandEvent &event);
    void    OnAbout(wxCommandEvent &event);
    void    OnClose(wxCloseEvent &event);

    /* View */
    void    OnSavePerspective(wxCommandEvent &event);
    void    OnLoadPerspective(wxCommandEvent &event);
    void    OnResetPerspective(wxCommandEvent &event);

    /* Debug */
    void    OnRun(wxCommandEvent &event);
    void    OnStepOver(wxCommandEvent &event);
    void    OnStepInto(wxCommandEvent &event);
    void    OnStepOut(wxCommandEvent &event);
    void    OnToggleBreakpoint(wxCommandEvent &event);
    void    OnRemoveBreakpoint(wxCommandEvent &event);
    void    OnToggleTraceClicked(wxCommandEvent &event);
//     void    OnToggleCRTEntryClicked(wxCommandEvent &event);
//     void    OnToggleSkipDllEntryClicked(wxCommandEvent &event);
    void    OnToggleTaintClicked(wxCommandEvent &event);
    void    OnShowMemory(wxCommandEvent &event);
    void    OnPluginCheckEnable(wxCommandEvent &event);

    void    OnStatusTimer(wxTimerEvent &event);
    
private:
    void    InitMisc();
    void    InitUI();
    void    InitMenu();
    void    InitStatusBar();
    void    InitToolbars();
private:
    Emulator *      m_emulator;
    ProEngine *     m_engine;
    Archive *       m_archive;
    ProTracer *     m_tracer;
    TaintEngine *   m_taint;
    ProPluginManager *  m_plugins;

    CpuPanel *      m_cpuPanel;
    ContextPanel *  m_contextPanel;
    CompositeTracePanel *   m_tracePanel;
    MemInfoPanel *  m_memInfoPanel;
    MemDataPanel *  m_memDataPanel;
    BreakpointsPanel    *   m_bpsPanel;

    wxString        m_defaultPerspective;
    wxAuiManager    m_auiManager;

    wxMenuBar *     m_mainMenu;
    wxMenu *        m_menuFile;
    wxMenu *        m_menuView;
    wxMenu *        m_menuDebug;
    wxMenu *        m_menuPlugins;
    wxMenu *        m_menuHelp;

    wxTimer         m_statusTimer;
    wxStatusBar *   m_statusbar;
    wxString        m_pathText;

    MySwitch *      m_toggleTrace;
    //MySwitch *      m_toggleCRTEntry;
    //MySwitch *      m_toggleSkipDllEntry;
    MySwitch *      m_toggleTaint;
};
 
#endif // __PROPHET_GUI_MAINFRAME_H__