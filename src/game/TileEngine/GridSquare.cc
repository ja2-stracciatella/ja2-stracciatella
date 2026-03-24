#include "GridSquare.h"
#include "JA2Types.h"
#include "WorldDef.h"

void ClearReachableFlags(GridSquare const& square)
{
	for (auto const gridNo : square)
	{
		gpWorldLevelData[gridNo].uiFlags &= ~MAPELEMENT_REACHABLE;
	}
}


GridSquare::GridSquare(GridNo const center, int const apothem)
{
	if (center < 0 || center >= WORLD_MAX) return;
	if (apothem < 0) return;

	int const centerRow = center / WORLD_COLS;
	int const centerColumn = center % WORLD_COLS;

	int const leftEdge = std::max(0, centerColumn - apothem);
	int const rightEdge = std::min(WORLD_COLS - 1, centerColumn + apothem);

	int const topEdge = std::max(0, centerRow - apothem);
	int const bottomEdge = std::min(WORLD_ROWS - 1, centerRow + apothem);

	c.reserve((rightEdge - leftEdge + 1) * (bottomEdge - topEdge + 1));

	for (int row = topEdge; row <= bottomEdge; ++row)
	{
		for (int column = leftEdge; column <= rightEdge; ++column)
		{
			c.push_back(static_cast<GridNo>(row * WORLD_COLS + column));
		}
	}
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(GridSquare, NegativeApothem)
{
	GridSquare square{ 7000, -1 };
	EXPECT_EQ(square.c.size(), 0);
}

TEST(GridSquare, ZeroApothem)
{
	GridSquare square{ 7000, 0 };
	EXPECT_EQ(square.c.size(), 1);
	EXPECT_EQ(square.c.front(), 7000);
}

TEST(GridSquare, PositiveApothem)
{
	GridSquare square{ 7000, 1 };
	EXPECT_EQ(square.c.size(), 9);
	EXPECT_EQ(square.c.front(), 7000 - WORLD_COLS - 1);
	EXPECT_EQ(square.c.back(), 7000 + WORLD_COLS + 1);
}

TEST(GridSquare, ApothemEqualsMapSize)
{
	GridSquare square{ 7000, WORLD_COLS };
	EXPECT_EQ(square.c.size(), WORLD_MAX);
	EXPECT_EQ(square.c.front(), 0);
	EXPECT_EQ(square.c.back(), WORLD_MAX - 1);
}

TEST(GridSquare, LeftEdgeClipped)
{
	constexpr GridNo center = 30 * WORLD_COLS + 3;
	GridSquare square{ center, 5 };
	EXPECT_EQ(square.c.size(), 9 * 11);
	EXPECT_EQ(square.c.front(), center - 5 * WORLD_COLS - 3);
	EXPECT_EQ(square.c.back(), center + 5 * WORLD_COLS + 5);
}

TEST(GridSquare, RightEdgeClipped)
{
	constexpr GridNo center = 30 * WORLD_COLS + 158;
	GridSquare square{ center, 5 };
	EXPECT_EQ(square.c.size(), 7 * 11);
	EXPECT_EQ(square.c.front(), center - 5 * WORLD_COLS - 5);
	EXPECT_EQ(square.c.back(), center + 5 * WORLD_COLS + 1);
}

TEST(GridSquare, TopEdgeClipped)
{
	constexpr GridNo center = 4 * WORLD_COLS + 20;
	GridSquare square{ center, 6 };
	EXPECT_EQ(square.c.size(), 11 * 13);
	EXPECT_EQ(square.c.front(), center - 4 * WORLD_COLS - 6);
	EXPECT_EQ(square.c.back(), center + 6 * WORLD_COLS + 6);
}

TEST(GridSquare, BottomEdgeClipped)
{
	constexpr GridNo center = 159 * WORLD_COLS + 8;
	GridSquare square{ center, 2 };
	EXPECT_EQ(square.c.size(), 3 * 5);
	EXPECT_EQ(square.c.front(), center - WORLD_COLS * 2 - 2);
	EXPECT_EQ(square.c.back(), center + 2);
}

TEST(GridSquare, InvalidCenterGridNo)
{
	{
		GridSquare square{ -1, 1 };
		EXPECT_EQ(square.c.size(), 0);
	}
	{
		GridSquare square{ WORLD_MAX, 5 };
		EXPECT_EQ(square.c.size(), 0);
	}
}

#endif
