#pragma once
 
#ifndef __ARIETIS_PLUGIN_PLUGIN_H__
#define __ARIETIS_PLUGIN_PLUGIN_H__
 
#include "Arietis.h"
#include "utilities.h"

enum FuncOverride : uint {
    Func_PreExecute     = 1,
    Func_PostExecute    = 1 << 2,
    Func_MemRead        = 1 << 3,
    Func_MemWrite       = 1 << 4,
    Func_ProcessPreRun  = 1 << 5,
    Func_ProcessPostRun = 1 << 6,
    Func_ProcessPreLoad = 1 << 7,
    Func_ProcessPostLoad= 1 << 8,
    Func_WinapiPreCall  = 1 << 9,
    Func_WinapiPostCall = 1 << 10,
};



class Plugin : public ISerializable {
public:
    Plugin(const std::string name, uint ovd);
    virtual ~Plugin() {}

    std::string     GetName() const { return m_name; }
    bool            IsEnabled() const { return m_enabled; }
    void            Enable(bool isEnabled) { m_enabled = isEnabled; }
    bool            HasOverrideFlag(FuncOverride f) const { return (m_ovdFlag & f) != 0; }
    APluginManager* GetManager() { return m_manager; }
    const APluginManager *  GetManager() const { return m_manager; }

    void            Serialize(Json::Value &root) const override;
    void            Deserialize(Json::Value &root) override;

    virtual void    Initialize() {}
    virtual void    OnPreExecute        (PreExecuteEvent        &event, bool firstTime) {}
    virtual void    OnPostExecute       (PostExecuteEvent       &event, bool firstTime) {}
    virtual void    OnMemRead           (MemReadEvent           &event, bool firstTime) {}
    virtual void    OnMemWrite          (MemWriteEvent          &event, bool firstTime) {}
    virtual void    OnProcessPreRun     (ProcessPreRunEvent     &event, bool firstTime) {}
    virtual void    OnProcessPostRun    (ProcessPostRunEvent    &event, bool firstTime) {}
    virtual void    OnProcessPreLoad    (ProcessPreLoadEvent    &event, bool firstTime) {}
    virtual void    OnProcessPostLoad   (ProcessPostLoadEvent   &event, bool firstTime) {}
    virtual void    OnWinapiPreCall     (WinapiPreCallEvent     &event, bool firstTime) {}
    virtual void    OnWinapiPostCall    (WinapiPostCallEvent    &event, bool firstTime) {}
    
private:
    std::string     m_name;
    bool            m_enabled;
    APluginManager *m_manager;
    uint    m_ovdFlag;
};


class APluginManager : public ISerializable {
public:
    APluginManager(AEngine *engine);
    ~APluginManager();

    void            Initialize();
    void            RegisterPlugin(Plugin *plugin);
    AEngine *       GetEngine() { return m_engine; }
    const AEngine * GetEngine() const { return m_engine; }

    virtual void    Serialize(Json::Value &root) const override;
    virtual void    Deserialize(Json::Value &root) override;

    void            OnPreExecute        (PreExecuteEvent        &event, bool firstTime);
    void            OnPostExecute       (PostExecuteEvent       &event, bool firstTime);
    void            OnMemRead           (MemReadEvent           &event, bool firstTime);
    void            OnMemWrite          (MemWriteEvent          &event, bool firstTime);
    void            OnProcessPreRun     (ProcessPreRunEvent     &event, bool firstTime);
    void            OnProcessPostRun    (ProcessPostRunEvent    &event, bool firstTime);
    void            OnProcessPreLoad    (ProcessPreLoadEvent    &event, bool firstTime);
    void            OnProcessPostLoad   (ProcessPostLoadEvent   &event, bool firstTime);
    void            OnWinapiPreCall     (WinapiPreCallEvent     &event, bool firstTime);
    void            OnWinapiPostCall    (WinapiPostCallEvent    &event, bool firstTime);

private:
    std::vector<Plugin *>   m_plugins;
    AEngine *       m_engine;
};

#endif // __ARIETIS_PLUGIN_PLUGIN_H__