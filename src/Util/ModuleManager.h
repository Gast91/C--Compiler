#pragma once
#include <vector>
#include <functional>

class ASTNode;

template<typename T = void>
class IObserver
{
public:
	virtual ~IObserver() = default;
	virtual void Update(T* n) = 0;
};

template<>
class IObserver<void>
{
protected:
	std::function<std::string(const int)> GetSourceLine;
public:
	virtual ~IObserver() = default;
	virtual void SetCallback(std::function<std::string(const int)> callback) { GetSourceLine = callback; }
	virtual bool ShouldRun() const = 0;
	virtual void SetToRun() = 0;
	virtual void Update() = 0;
};

template<typename T>
class Subject
{
protected:
	std::vector<IObserver<T>*> observers;
public:
	virtual ~Subject() = default;

	template<typename ...Args>
	void RegisterObservers(Args... obs) { (observers.push_back(obs), ...); }

	virtual void NotifyObservers() = 0;
};

class ModuleManager : public Subject<void>
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

	virtual void NotifyObservers() override { for (auto& obs : observers) obs->SetToRun(); }
	void UpdateGetSourceLineCallback(std::function<std::string(const int)> callback)
	{
		for (auto obs : observers) obs->SetCallback(callback);
	}
	void RunModulesUpTo(IObserver<void>* obs)
	{
		for (auto it = observers.begin(); it != std::next(std::find(observers.begin(), observers.end(), obs)); ++it)
			(*it)->Update();
	}
};