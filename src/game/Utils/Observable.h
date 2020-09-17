#pragma once

#include <functional>
#include <vector>

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
	 * @param callback a Callable taking the expected args
	 */
	Observable<ARG1, ARGS...> listen(std::function<void(ARG1, ARGS...)> callback)
	{
		observers.push_back(callback);
		return *this;
	}

	/**
	 * @brief This override is only available with 'Observable<>'
	 * @throw logic_error
	 */
	Observable<ARG1, ARGS...> listen(std::function<void()> callback)
	{
		throw new std::logic_error("no-args callback only allowed with 'Observable<>'");
	}

	/**
	 * @brief Notifies all the registered callbacks, in the same order as registration
	 * @tparam ARG1 first argument to the callback; uses default value if skipped
	 * @tparam ...ARGS the remaining arguments to pass to the callback 
	 */
	void notify(ARG1 arg1 = ARG1(), ARGS... args) const
	{
		for (auto f : observers)
		{
			f(arg1, args...);
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
		observers.clear();
		return *this;
	}

private:
	std::vector<std::function<void(ARG1, ARGS...)>> observers;
};

/**
 * @brief Specialized listen() override for no-args callbacks. This can only be used with 'Observable<>'
 * @param callback a Callable taking no arguments
 * @return the current instance for method chaining
*/
template<>
Observable<_observable::Nil> Observable<_observable::Nil>::listen(std::function<void()> f)
{
	return listen([f](_observable::Nil) { f(); });
}