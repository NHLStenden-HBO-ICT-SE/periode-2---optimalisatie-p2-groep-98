#pragma once
#include "precomp.h"

enum class Collider {
	TANK, ROCKET
};

class Collidable
{

public:

	Collider collider_type;
	virtual vec2 getCurrentPosition() = 0;
	virtual float getCollisionRadius() = 0;
};




