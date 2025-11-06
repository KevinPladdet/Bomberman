#pragma once
#include "map.h"

struct Tile
{
	Tile(int2 _pos, bool _hasPowerUp) { pos = _pos; hasPowerUp = _hasPowerUp;}
	int2 pos;
	bool hasPowerUp;
	PowerUpType powerUpType;
};