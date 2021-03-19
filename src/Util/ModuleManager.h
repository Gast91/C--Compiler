#pragma once
#include <set>
#include <functional>

class ASTNode;
class IObserver
{
public:
    virtual ~IObserver() = default;
    virtual void SetCallback(std::function<std::string(const int)> callback) = 0;
    virtual bool ShouldRun() const = 0;
    virtual void SetToRun() = 0;
    virtual void Update(ASTNode* n) {}
    virtual void Run() = 0;
};

class Subject
{
protected:
    std::set<IObserver*> observers;
public:
    virtual ~Subject() = default;

    template<typename ...Args>
    void RegisterObservers(Args... obs) { (observers.insert(observers.end(), obs), ...); }
};

class ModuleManager : public Subject
{
private:    
    ModuleManager() = default;
public:
    virtual ~ModuleManager() = default;

    static ModuleManager* Instance()
    {
        static ModuleManager managerInstance;
        return &managerInstance;
    }

    void NotifyModulesToRun() { for (auto& obs : observers) obs->SetToRun(); }
    void UpdateGetSourceLineCallback(std::function<std::string(const int)> callback)
    {
        for (auto obs : observers) obs->SetCallback(callback);
    }
    void RunModulesUpTo(IObserver* obs)
    {
        for (auto it = observers.begin(); it != std::next(observers.find(obs)); ++it)
            (*it)->Run();
    }
};