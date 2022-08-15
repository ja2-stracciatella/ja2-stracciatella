#pragma once

#include "Logger.h"
#include <functional>
#include <map>
#include <string_theory/format>
#include <string_theory/string>

namespace _observable
{
	// placeholder struct for a no-args callback function to satisfy class template 
	struct Nil {};
}

/**
 * @brief Implements the Observer design pattern, for dependency inversion and allow registering callbacks at runtime.
 * @tparam ARG1 the first argument the callback function takes
 * @tparam ...ARGS arguments that the callback function takes
*/
template<typename ARG1 = _observable::Nil, typename ...ARGS>
class Observable
{
public:
	/**
	 * @brief Register a function to be called when callback is invoked
	 * @param key a key identifier one callback. Callback is replaced if there is an existing one with the same key
	 * @param callback a Callable taking the expected args
	 * @return the current instance for method chaining
	 */
	Observable<ARG1, ARGS...> addListener(const ST::string key, std::function<void(ARG1, ARGS...)> callback)
	{
		listeners[key] = callback;
		return *this;
	}

	/**
	 * @brief This override is only available with 'Observable<>'
	 * @param key a key identifier one callback. Callback is replaced if there is an existing one with the same key
	 * @param callback a Callable taking no-args
	 * @return the current instance for method chaining
	 * @throw logic_error if the current instance is not 'Observable<>'
	 */
	Observable<ARG1, ARGS...> addListener(const ST::string key, const std::function<void()> callback);

	/**
	 * @brief Un-registers a listen identified by the given key
	 * @param key 
	 * @return the current instance for method chaining
	*/
	Observable<ARG1, ARGS...> removeListener(const ST::string key)
	{
		if (listeners.find(key) == listeners.end())
		{
			SLOGW("There is no listener for key '{}'", key);
			return *this;
		}

		listeners.erase(key);
		return *this;
	}

	/**
	 * @brief Notifies all the registered callbacks, in lexical order of the listener keys
	 * @tparam ARG1 first argument to the callback; uses default value if skipped
	 * @tparam ...ARGS the remaining arguments to pass to the callback 
	 */
	void notify(ARG1 arg1 = ARG1(), ARGS... args) const
	{
		if (listeners.empty()) {
			SLOGD("Observable has no listeners");
		}

		for (auto const& l : listeners)
		{
			l.second(arg1, args...);
		}
	}

	/**
	 * @brief Alias to notify(), to make the object callable.
	 * @tparam ARG1 first argument to the callback; uses default value if skipped
	 * @tparam ...ARGS the remaining arguments to pass to the callback
	*/
	void operator()(ARG1 arg1 = ARG1(), ARGS... args) const
	{
		notify(arg1, args...);
	}

	/**
	 * @brief Clears the list of registered callbacks
	 * @return the current instance for method chaining
	 */
	Observable<ARG1, ARGS...>& reset()
	{
		listeners.clear();
		return *this;
	}

private:
	std::map<const ST::string, std::function<void(ARG1, ARGS...)>> listeners;
};
