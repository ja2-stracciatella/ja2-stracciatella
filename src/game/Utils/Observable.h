#pragma once

#include <functional>
#include <list>
#include <vector>

template<typename ...A>
class Observable
{
public:
	void listen(std::function<void(A...)> callback)
	{
		observers.push_back(callback);
	}

	void notify(A... args) const
	{
		for (auto f : observers)
		{
			f(args...);
		}
	}
	void clear()
	{
		observers.clear();
	}

private:
	std::list<std::function<void(A...)>> observers;
};
