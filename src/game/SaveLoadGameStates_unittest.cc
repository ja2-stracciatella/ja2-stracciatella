#include "gtest/gtest.h"

#include "SaveLoadGameStates.h"
#include <string_theory/string>
#include <vector>

TEST(SaveLoadGameStatesTest, getSetGameStates)
{
	SavedGameStates states;

	states.Set("test_BOOL", true);
	states.Set("test_INT", 123);
	states.Set("test_FLOAT", 4.56f);
	states.Set("test_STRING", ST::string("abc"));
	states.Set("test_STRING_LIT", "def");

	EXPECT_EQ(states.Get<bool>("test_BOOL"), true);
	EXPECT_EQ(states.Get<int32_t>("test_INT"), 123);
	ASSERT_DOUBLE_EQ(states.Get<float>("test_FLOAT"), 4.56f);
	EXPECT_STREQ(states.Get<ST::string>("test_STRING").c_str(), "abc");
	EXPECT_STREQ(states.Get<ST::string>("test_STRING_LIT").c_str(), "def");
}

TEST(SaveLoadGameStatesTest, getSetVectors)
{
	SavedGameStates states;

	states.SetVector("vec_int", std::vector<int32_t>{1,2});
	auto vec = states.GetVector<int32_t>("vec_int");
	EXPECT_EQ(vec[0], 1);
	EXPECT_EQ(vec[1], 2);

	states.SetVector("vec_str", std::vector{ST::string("abc")});
	auto vec2 = states.GetVector<ST::string>("vec_str");
	EXPECT_EQ(vec2[0], "abc");
}

TEST(SaveLoadGameStatesTest, getSetMaps)
{
	SavedGameStates states;

	states.SetMap("M1", std::map<ST::string, bool>{{"3", true},{"2", false}});
	auto map1 = states.GetMap<bool>("M1");
	EXPECT_EQ(map1["3"], true);
	EXPECT_EQ(map1["2"], false);

	states.SetMap("M2", std::map<ST::string, float>{{"a", 0.5f}});
	auto map2 = states.GetMap<float>("M2");
	EXPECT_DOUBLE_EQ(map2["a"], 0.5f);
}

TEST(SaveLoadGameStatesTest, edgeCases)
{
	SavedGameStates states;
	states.Set("K", true);

	ASSERT_THROW(states.Get<bool>(""), std::out_of_range);
	ASSERT_THROW(states.Get<int32_t>(ST::string("k")), std::out_of_range);
	ASSERT_THROW(states.Get<int32_t>("K"), std::bad_variant_access);
	ASSERT_THROW(states.Get<float>("K"), std::bad_variant_access);
}

TEST(SaveLoadGameStatesTest, serializeJSON)
{
	SavedGameStates s;
	s.Set("B", true);
	s.Set("I", 987);
	s.Set("F", 6.5f);
	s.Set("S", ST::string("abc"));

	std::stringstream ss;
	s.Serialize(ss);
	EXPECT_EQ(ss.str(), "{\"B\":true,\"F\":6.5,\"I\":987,\"S\":\"abc\"}");
}

TEST(SaveLoadGameStatesTest, serializeJSONVector)
{
	SavedGameStates s;
	s.SetVector("B", std::vector<bool>{false, true});
	s.SetVector("I", std::vector<int32_t>{3});
	s.SetVector("F", std::vector<float>{4.5f});
	s.SetVector("S", std::vector<ST::string>{"a", "b"});

	std::stringstream ss;
	s.Serialize(ss);
	EXPECT_EQ(ss.str(), R"({"B":[false,true],"F":[4.5],"I":[3],"S":["a","b"]})");
}

TEST(SaveLoadGameStatesTest, serializeJSONMap)
{
	SavedGameStates s;
	s.SetMap("M", std::map<ST::string, int32_t>{{"one", 1}});

	std::stringstream ss;
	s.Serialize(ss);
	EXPECT_EQ(ss.str(), "{\"M\":{\"one\":1}}");
}

TEST(SaveLoadGameStatesTest, deserializeEmpty)
{
	std::string json = "{}";
	std::stringstream ss(json);

	SavedGameStates s;
	s.Deserialize(ss);
	EXPECT_EQ(s.GetAll().size(), 0);
}

TEST(SaveLoadGameStatesTest, deserializeJSON)
{
	std::string json = R"({"B":true,"F":3.4,"I":567,"S":"xyz"})";
	std::stringstream ss(json);

	SavedGameStates s;
	s.Deserialize(ss);

	EXPECT_EQ(s.Get<bool>("B"), true);
	EXPECT_EQ(s.Get<float>("F"), 3.4f);
	EXPECT_EQ(s.Get<int32_t>("I"), 567);
	EXPECT_EQ(s.Get<ST::string>("S"), "xyz");
	EXPECT_EQ(s.GetAll().size(), 4);
}

TEST(SaveLoadGameStatesTest, vectorOfVariant)
{
	SavedGameStates states;
	auto vec = std::vector<PRIMITIVE_VALUE>();
	vec.emplace_back(1);
	vec.emplace_back(2.5f);
	vec.emplace_back(true);
	vec.emplace_back(ST::string("str"));
	states.Set("vec", vec);

	std::stringstream ss;
	states.Serialize(ss);
	EXPECT_EQ(ss.str(), "{\"vec\":[1,2.5,true,\"str\"]}");

	std::stringstream is(ss.str());
	states.Deserialize(is);
	auto vec2 = states.Get<std::vector<PRIMITIVE_VALUE>>("vec");
	EXPECT_EQ(std::get<int>(vec2[0]),   1);
	EXPECT_EQ(std::get<float>(vec2[1]), 2.5f);
	EXPECT_EQ(std::get<bool>(vec2[2]),  true);
	EXPECT_EQ(std::get<ST::string>(vec2[3]), ST::string("str"));
}

TEST(SaveLoadGameStatesTest, mapOfVariants)
{
	SavedGameStates states;
	auto map = std::map<ST::string, PRIMITIVE_VALUE> {
		{ ST::string("f"), 1.5f },
		{ ST::string("i"), 1 },
		{ ST::string("b"), false },
		{ ST::string("s"), ST::string("st") }
	};
	states.Set("m", map);

	std::stringstream ss;
	states.Serialize(ss);
	EXPECT_EQ(ss.str(), R"({"m":{"b":false,"f":1.5,"i":1,"s":"st"}})");

	std::stringstream is(ss.str());
	states.Deserialize(is);
	auto map2 = states.Get<std::map<ST::string, PRIMITIVE_VALUE>>("m");
	EXPECT_EQ(std::get<int  >(map2[ST::string("i")]), 1);
	EXPECT_EQ(std::get<float>(map2[ST::string("f")]), 1.5f);
	EXPECT_EQ(std::get<bool >(map2[ST::string("b")]), false);
	EXPECT_EQ(std::get<ST::string>(map2[ST::string("s")]), ST::string("st"));
}