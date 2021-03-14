#pragma once
#include <set>

class IObserver
{
public:
    virtual ~IObserver() = default;
    virtual bool ShouldRun() const = 0;
    virtual void SetToRun() = 0;
    virtual void Run() = 0;
};

class ModuleManager
{
private:
    std::set<IObserver*> observers;
    
    ModuleManager() = default;
public:
    static ModuleManager* Instance()
    {
        static ModuleManager managerInstance;
        return &managerInstance;
    }

    template<typename ...Args>
    void RegisterObservers(Args... obs) { (observers.insert(observers.end(), obs), ...); }
    void NotifyModulesToRun() { for (auto& obs : observers) obs->SetToRun(); }
    void RunModulesUpTo(IObserver* obs)
    {
        for (auto it = observers.begin(); it != std::next(observers.find(obs)); ++it)
            (*it)->Run();
    }
};