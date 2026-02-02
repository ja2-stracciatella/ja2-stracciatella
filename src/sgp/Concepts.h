#pragma once

#include <type_traits>

template<typename T>
concept memcpyable = std::is_trivially_copyable_v<T>;
