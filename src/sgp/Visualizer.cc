#include "Visualizer.h"

#include "Animation_Control.h"
#include "Cheats.h"
#include "Handle_UI.h"
#include "Interface.h"
#include "JAScreens.h"
#include "SDL_helpers.h"
#include "SDL3/SDL.h"
#include "Soldier_Control.h"
#include "Structure_Internals.h"
#include "WorldDef.h"

#include <memory>

namespace Visualizer
{

bool toggledOn{ };

std::unique_ptr<SDL_Texture, SDLDeleter> bgTexture;
std::unique_ptr<SDL_Texture, SDLDeleter> finalTexture;

constexpr SDL_FRect bgRectDest{ 11, 47, 240, 320 };
constexpr float isoToActualLengthRatio = 0.816f;
constexpr float actualEdgeLength = 120;
constexpr float isoEdgeLength = actualEdgeLength * isoToActualLengthRatio;
constexpr float isoBoxEdgeLength = isoEdgeLength / PROFILE_X_SIZE;
// lengths for an opposite and an adjacent leg of the triangle's isometric angle to the X axis
constexpr float oppLegLength = isoEdgeLength * 0.500f; // 0.500 is sin(30)
constexpr float adjLegLength = isoEdgeLength * 0.866f; // 0.866 is cos(30)
constexpr float oppBoxLegLength = isoBoxEdgeLength * 0.500f;
constexpr float adjBoxLegLength = isoBoxEdgeLength * 0.866f;

constexpr float boxHeight = actualEdgeLength / PROFILE_Z_SIZE;

constexpr SDL_FColor tileColorLvl0   { 0,      1,               0,          SDL_ALPHA_OPAQUE }; // green
constexpr SDL_FColor tileColorLvl1   { 0.254f, 0.141f,          0.94f,      SDL_ALPHA_OPAQUE }; // purple
constexpr SDL_FColor boxTopColor     { 0,      0.647f,          1,          SDL_ALPHA_OPAQUE }; // blue
constexpr SDL_FColor boxRightColor   { 0,      0.647f - 0.118f, 1 - 0.118f, SDL_ALPHA_OPAQUE };
constexpr SDL_FColor boxLeftColor    { 0,      0.647f - 0.059f, 1 - 0.059f, SDL_ALPHA_OPAQUE };
// Alternatives for the checkered pattern 
constexpr SDL_FColor boxTopColorAlt  { 0,      0.565f,          1,          SDL_ALPHA_OPAQUE };
constexpr SDL_FColor boxRightColorAlt{ 0,      0.565f - 0.118f, 1 - 0.118f, SDL_ALPHA_OPAQUE };
constexpr SDL_FColor boxLeftColorAlt { 0,      0.565f - 0.059f, 1 - 0.059f, SDL_ALPHA_OPAQUE };

// The coordinate that every other coordinate is calculated from
constexpr SDL_Vertex baseCoord{ { bgRectDest.w / 2, bgRectDest.h - 5 - oppLegLength * 2}, tileColorLvl0 };

// Green ground rhombus (isometric square)
constexpr SDL_Vertex verticesLvl0[]
{
	   baseCoord,																						 // farthest from the viewer
	{ {baseCoord.position.x + adjLegLength, baseCoord.position.y + oppLegLength},     baseCoord.color }, // right
	{ {baseCoord.position.x,                baseCoord.position.y + oppLegLength * 2}, baseCoord.color }, // nearest
	{ {baseCoord.position.x - adjLegLength, baseCoord.position.y + oppLegLength},     baseCoord.color }, // left
};

// Triangle rendering sequence
const int indices[]
{
   0,1,2,
   2,3,0,
};

// Texture update triggers
GridNo lastCell{ 0 };
int16_t lastInterfaceLvl{ I_GROUND_LEVEL };
AnimationStates lastAnimState{ NOTUSEDANIM1 };

static void DrawHitbox(SDL_Renderer * const renderer, float const x, float const y, float const z, bool const drawRoofOnly = false)
{
	float const offsetX = baseCoord.position.x + (x - y) * adjBoxLegLength;
	float const offsetY = baseCoord.position.y + (x + y) * oppBoxLegLength - boxHeight * (z + 1);

	// ensure checkered color pattern horizontally ...
	bool isAlt{ (bool)fmod(x + y, 2) };
	// ... and vertically
	if (fmod(z, 2))
	{
		isAlt = !isAlt;
	}

	SDL_Vertex topFace[4];
	SDL_Vertex rightFace[4];
	SDL_Vertex leftFace[4];

	SDL_FColor currTopColor;
	if (drawRoofOnly)
	{
		currTopColor = tileColorLvl1;
	}
	else
	{
		currTopColor = isAlt ? boxTopColorAlt : boxTopColor;
	}
	topFace[0] =   { { offsetX,                   offsetY },                       currTopColor };
	topFace[1] =   { { offsetX + adjBoxLegLength, offsetY + oppBoxLegLength },     currTopColor };
	topFace[2] =   { { offsetX,                   offsetY + oppBoxLegLength * 2 }, currTopColor };
	topFace[3] =   { { offsetX - adjBoxLegLength, offsetY + oppBoxLegLength },     currTopColor };
	SDL_RenderGeometry(renderer, nullptr, &topFace[0], 4, indices, 6);

	if (drawRoofOnly)
		return;

	SDL_FColor currRightColor{ isAlt ? boxRightColorAlt : boxRightColor };
	rightFace[0] = { topFace[1].position,                                                     currRightColor };
	rightFace[1] = { { offsetX + adjBoxLegLength, offsetY + oppBoxLegLength + boxHeight },    currRightColor };
	rightFace[2] = { { offsetX,                   offsetY + oppBoxLegLength * 2 + boxHeight}, currRightColor };
	rightFace[3] = { topFace[2].position,                                                     currRightColor };
	SDL_RenderGeometry(renderer, nullptr, &rightFace[0], 4, indices, 6);

	SDL_FColor currLeftColor { isAlt ? boxLeftColorAlt  : boxLeftColor };
	leftFace[0] =  { topFace[3].position,                                                  currLeftColor };
	leftFace[1] =  { topFace[2].position,                                                  currLeftColor };
	leftFace[2] =  { rightFace[2].position,                                                currLeftColor };
	leftFace[3] =  { { offsetX - adjBoxLegLength, offsetY + oppBoxLegLength + boxHeight }, currLeftColor };
	SDL_RenderGeometry(renderer, nullptr, &leftFace[0], 4, indices, 6);
}

static void UpdateTexture(SDL_Renderer * const renderer)
{
	SDL_SetRenderTarget(renderer, finalTexture.get());

	SDL_RenderTexture( renderer, bgTexture.get(), nullptr, nullptr);

	uint8_t combinedShapeLvl0[PROFILE_X_SIZE][PROFILE_Y_SIZE]{};
	uint8_t combinedShapeLvl1[PROFILE_X_SIZE][PROFILE_Y_SIZE]{};
	bool isWalkableRoofPresent{ (bool)gpWorldLevelData[lastCell].pRoofHead };
	for (uint8_t loopX = 0; loopX < PROFILE_X_SIZE; loopX++)
	{
		for (uint8_t loopY = 0; loopY < PROFILE_Y_SIZE; loopY++)
		{
			STRUCTURE* structure{ gpWorldLevelData[lastCell].pStructureHead };
			bool isAnyRoofPresent{ };
			while (structure)
			{
				PROFILE* shape{ structure->pShape };
				// Coalesce all the structures' shapes in the tile into a single shape
				if (structure->sCubeOffset == 0)
				{
					combinedShapeLvl0[loopX][loopY] |= (*shape)[loopX][loopY];
				}
				else
				{
					isAnyRoofPresent = structure->fFlags & STRUCTURE_ROOF;
					combinedShapeLvl1[loopX][loopY] |= (*shape)[loopX][loopY];
				}
				structure = structure->pNext;
			}

			for (uint8_t loopZ = 0; loopZ < PROFILE_Z_SIZE; loopZ++)
			{
				if (combinedShapeLvl0[loopX][loopY] & AtHeight[loopZ])
				{
					DrawHitbox(renderer, loopX, loopY, loopZ);
				}
				if (loopZ == PROFILE_Z_SIZE - 1 && isWalkableRoofPresent && lastInterfaceLvl == I_ROOF_LEVEL)
				{
					DrawHitbox(renderer, loopX, loopY, loopZ, true);
				}
			}

			if (isAnyRoofPresent && lastInterfaceLvl == I_GROUND_LEVEL)
				continue;

			for (uint8_t loopZ = 0; loopZ < PROFILE_Z_SIZE; loopZ++)
			{
				if (combinedShapeLvl1[loopX][loopY] & AtHeight[loopZ])
				{
					DrawHitbox(renderer, loopX, loopY, loopZ + PROFILE_Z_SIZE);
				}
			}
		}
	}

	SDL_SetRenderTarget(renderer, nullptr);
}

static void InitTextures(SDL_Renderer* const renderer)
{
	finalTexture.reset(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bgRectDest.w, bgRectDest.h));

	bgTexture.reset(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bgRectDest.w, bgRectDest.h));
	SDL_SetRenderTarget(renderer, bgTexture.get());

	// Black background
	SDL_SetRenderDrawColor(renderer, 10, 10, 10, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, nullptr);
	// Green ground tile
	SDL_RenderGeometry(renderer, nullptr, verticesLvl0, 4, indices, 6);

	SDL_SetRenderTarget(renderer, nullptr);
}

void Render(SDL_Renderer * const renderer)
{
	if (!toggledOn)
	{
		return;
	}

	if (guiCurrentScreen != GAME_SCREEN)
	{
		ToggleOnOff();
	}
	if (!finalTexture)
	{
		InitTextures(renderer);
	}

	AnimationStates currAnimState{ };
	if (gpWorldLevelData[lastCell].pMercHead)
	{
		currAnimState = static_cast<AnimationStates>( gpWorldLevelData[lastCell].pMercHead->pSoldier->usAnimState);
	}
	else
	{
		lastAnimState = NOTUSEDANIM1;
	}

	if (lastCell != guiCurrentCursorGridNo || lastInterfaceLvl != gsInterfaceLevel || currAnimState != lastAnimState )
	{
		lastCell = guiCurrentCursorGridNo;
		lastInterfaceLvl = gsInterfaceLevel;
		UpdateTexture(renderer);
	}

	SDL_RenderTexture(renderer, finalTexture.get(), nullptr, &bgRectDest);
	// set this back to total black for subsequent SDL_RenderClear
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
}

void ToggleOnOff()
{
	if (guiCurrentScreen == GAME_SCREEN && !toggledOn && CHEATER_CHEAT_LEVEL())
	{
		lastCell = 0;
		toggledOn = true;
	}
	else if (toggledOn)
	{
		toggledOn = false;
		bgTexture.reset();
		finalTexture.reset();
	}
}

}
