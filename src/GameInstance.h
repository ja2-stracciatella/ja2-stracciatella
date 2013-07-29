#pragma once

class GamePolicy;

/**
 * Global instance of the game policy.
 *
 * It should be used if the application code doesn't already have an
 * instance of the game policy. */
extern const GamePolicy *GGP;
