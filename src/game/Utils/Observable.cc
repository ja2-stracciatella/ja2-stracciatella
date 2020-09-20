#include "Observable.h"
#include <stdexcept>

template<typename ARG1, typename ...ARGS>
Observable<ARG1, ARGS...> Observable<ARG1, ARGS...>::addListener(const ST::string key, const std::function<void()> f)
{
	throw std::logic_error("no-args callback only allowed with 'Observable<>'");
}

/**
 * @brief Specialized addListener() override for no-args callbacks. This can only be used with 'Observable<>'
 * @param callback a Callable taking no arguments
 * @return the current instance for method chaining
*/
template<>
Observable<_observable::Nil> Observable<_observable::Nil>::addListener(const ST::string key, const std::function<void()> f)
{
	return addListener(key, [f](_observable::Nil) { f(); });
}