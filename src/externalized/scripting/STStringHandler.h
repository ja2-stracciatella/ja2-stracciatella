#pragma once
#define SOL_ALL_SAFETIES_ON 1
#include <limits>
#include <sol/sol.hpp>
#include <string_theory/string>
#include <string>

// Implements pusher and getter for string_theory/string. It should be handled just like a std::string
// @see https://sol2.readthedocs.io/en/latest/tutorial/customization.html
template <typename Handler>
bool sol_lua_check(sol::types<ST::string>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	// check_usertype is a backdoor for directly checking sol3 usertypes
	if (!sol::stack::check_usertype<ST::string>(L, index)
		&& !sol::stack::check<std::string>(L, index)) {
		handler(L, index, sol::type_of(L, index), sol::type::userdata, "expected a ST::string or a string");
		return false;
	}
	tracking.use(1);
	return true;
}

ST::string sol_lua_get(sol::types<ST::string>, lua_State* L, int index, sol::stack::record& tracking)
{
	if (sol::stack::check_usertype<ST::string>(L, index)) {
		ST::string& ns = sol::stack::get_usertype<ST::string>(L, index, tracking);
		return ns;
	}
	std::string str = sol::stack::get<std::string>(L, index, tracking);
	return ST::string(str);
}

int sol_lua_push(sol::types<ST::string>, lua_State* L, const ST::string& st)
{
	return sol::stack::push(L, st.to_std_string());
}


// Now ST::string*
template <typename Handler>
bool sol_lua_check(sol::types<ST::string*>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
	// check_usertype is a backdoor for directly checking sol3 usertypes
	if (!sol::stack::check_usertype<ST::string*>(L, index)
	    && !sol::stack::check<std::string>(L, index)) {
		handler(L, index, sol::type_of(L, index), sol::type::userdata, "expected a ST::string* ot std::string");
		return false;
	}
	tracking.use(1);
	return true;
}

ST::string* sol_lua_get(sol::types<ST::string*>, lua_State* L, int index, sol::stack::record& tracking)
{
	if (sol::stack::check_usertype<ST::string*>(L, index)) {
		ST::string* ns = sol::stack::get_usertype<ST::string*>(L, index, tracking);
		return ns;
	}
	std::string str = sol::stack::get<std::string>(L, index, tracking);
	return new ST::string(str); // C++ to own the pointer
}

int sol_lua_push(sol::types<ST::string*>, lua_State* L, const ST::string* st)
{
	return sol::stack::push(L, st->to_std_string());
}
