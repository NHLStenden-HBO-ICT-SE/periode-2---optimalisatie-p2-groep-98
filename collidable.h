#pragma once
#include "precomp.h"
class Collidable
{

public:


	virtual vec2 getCurrentPosition() = 0;
	virtual float getCollisionRadius() = 0;
};

