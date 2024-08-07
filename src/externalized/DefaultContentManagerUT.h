#pragma once

#include "externalized/DefaultContentManager.h"
#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"
extern ContentManager * GCM;
#endif

struct EngineOptions;

class DefaultContentManagerUT : public DefaultContentManager
{
	DefaultContentManagerUT(RustPointer<EngineOptions> engineOptions);

public:
	/** Create DefaultContentManager for usage in unit testing. */
	static DefaultContentManagerUT* createDefaultCMForTesting();

	bool loadGameData() override;

#ifdef WITH_UNITTESTS

// Helper class to ease writing unit test that use the
// UT content manager. To use this class, first define
// an alias for this class:
//
// using MyTest = DefaultContentManagerUT::BaseTest;
//
// Then use
//
// TEST_F(MyTest, Testname) {}
//
// for the actual test.

class BaseTest : public testing::Test
{
protected:
	ContentManager * oldGCM;
	DefaultContentManagerUT * cm;

	BaseTest() :
		oldGCM{ GCM },
		cm{ DefaultContentManagerUT::createDefaultCMForTesting() }
	{
		GCM = cm;
		cm->loadGameData();
	}

	~BaseTest()
	{
		GCM = oldGCM;
		delete cm;
	}
};

#endif
};
