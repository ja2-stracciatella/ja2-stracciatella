#pragma once

#include <sol/sol.hpp>
#include <string_theory/string>
#include <string>

// Implements pusher and getter for string_theory/string. It should be handled just like a std::string
// @see https://github.com/ThePhD/sol2/blob/v2.20.6/examples/customization.cpp

namespace sol {
	template <>
	struct lua_type_of<ST::string> : std::integral_constant<sol::type, sol::type::string> {};

	namespace stack {
		template <>
		struct checker<ST::string> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				int absolute_index = lua_absindex(L, index);
				bool success = stack::check<std::string>(L, absolute_index, handler);
				return success;
			}
		};

		template <>
		struct getter<ST::string> {
			static ST::string get(lua_State* L, int index, record& tracking) {
				int absolute_index = lua_absindex(L, index);
				std::string str = stack::get<std::string>(L, absolute_index);
				return ST::string(str);
			}
		};

		template <>
		struct pusher<ST::string> {
			static int push(lua_State* L, const ST::string& st) {
				return stack::push(L, st.to_std_string());
			}
		};
	}
}
