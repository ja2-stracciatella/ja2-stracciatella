// -*-coding: utf-8-unix;-*-

#include "gtest/gtest.h"

#include "Logger.h"


TEST(Logger, sourcePathLength)
{
#if defined(_WIN32) && !defined(__MINGW32__)
	static_assert(std::string_view(__FILENAME__) == "sgp\\Logger_unittest.cc");

	EXPECT_EQ(GetSourcePathSize("C:\\Jagged Alliance 2\\src\\sgp\\Logger.h"), strlen("C:\\Jagged Alliance 2\\src\\"));
	EXPECT_EQ(GetSourcePathSize("..\\src\\game\\Init.cc"), strlen("..\\src\\"));
	EXPECT_EQ(GetSourcePathSize("sgp\\Logger.src"), 0);
#else
	static_assert(std::string_view(__FILENAME__) == "sgp/Logger_unittest.cc");

	EXPECT_EQ(GetSourcePathSize("/usr/share/ja2/src/sgp/Logger.h"), strlen("/usr/share/ja2/src/"));
	EXPECT_EQ(GetSourcePathSize("../src/game/Init.cc"), strlen("../src/"));
	EXPECT_EQ(GetSourcePathSize("sgp/Logger.src"), 0);
#endif
}

#undef EXPECTED_FILENAME
