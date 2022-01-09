#pragma once
#include "precomp.h"

enum class Collider {
	TANK, ROCKET, BEAM
};

class Collidable
{

public:
	/// <summary>
	/// Used to get the type of object and cast it.
	/// </summary>
	Collider collider_type;

	virtual vec2 col_get_current_position() = 0;
	virtual float col_get_collision_radius() = 0;
};




