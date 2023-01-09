#include "Random.h"

#include <chrono>
#include <random>

/// Pseudo-random number engine.
std::mt19937 gRandomEngine;

/// Uniform distribution in the range [0,UINT32_MAX].
static std::uniform_int_distribution<UINT32> guiDistribution(0, UINT32_MAX);

// Pregenerated pseudo-random numbers.
UINT32 guiPreRandomIndex = 0;
UINT32 guiPreRandomNums[ MAX_PREGENERATED_NUMS ];

/// Pre-generated pseudo-random number engine.
struct PreRandomEngine {
	typedef UINT32 result_type;
	static constexpr result_type min() { return 0; }
	static constexpr result_type max() { return UINT32_MAX; }
	result_type operator()()
	{
		// Extract the current pregenerated number
		UINT32 uiNum = guiPreRandomNums[ guiPreRandomIndex ];

		// Replace the current pregenerated number with a new one.
		// NOTE the original code has this commented out in the name of optimization.
		guiPreRandomNums[ guiPreRandomIndex ] = guiDistribution(gRandomEngine);

		// Go to the next index.
		guiPreRandomIndex++;
		if (guiPreRandomIndex >= (UINT32)MAX_PREGENERATED_NUMS)
			guiPreRandomIndex = 0;
		return uiNum;
	}
};
static PreRandomEngine gPreRandomEngine;

void InitializeRandom(void)
{
	// Seed the pseudo-random number engine with the current time
	// so that the numbers will be different every time we run.
	UINT32 uiSeed1 = std::chrono::system_clock::now().time_since_epoch().count();

	// Also try to seed the pseudo-random number engine with a non-deterministic
	// random number (entropy is 0 when not available).
	std::random_device randomDevice;
	UINT32 uiSeed2 = guiDistribution(randomDevice);

	std::seed_seq seed = { uiSeed1, uiSeed2 };
	gRandomEngine.seed(seed);

	// Pregenerate random numbers.
	for (guiPreRandomIndex = 0; guiPreRandomIndex < MAX_PREGENERATED_NUMS; ++guiPreRandomIndex)
	{
		guiPreRandomNums[ guiPreRandomIndex ] = guiDistribution(gRandomEngine);
	}
	guiPreRandomIndex = 0;
}

/// Returns a pseudo-random integer in the range [0,uiRange).
/// Returns 0 if no range is given (not an error).
UINT32 Random(UINT32 uiRange)
{
	if (!uiRange)
		return 0;
	std::uniform_int_distribution<UINT32> distribution(0, uiRange - 1);
	return distribution(gRandomEngine);
}

BOOLEAN Chance(UINT32 uiChance)
{
	return Random(100) < uiChance;
}

/// Returns a pregenerated pseudo-random integer in the range [0,uiRange).
/// Returns 0 if no range is given (not an error).
UINT32 PreRandom(UINT32 uiRange)
{
	if (!uiRange)
		return 0;
	std::uniform_int_distribution<UINT32> distribution(0, uiRange - 1);
	return distribution(gPreRandomEngine);
}

BOOLEAN PreChance(UINT32 uiChance)
{
	return PreRandom(100) < uiChance;
}
