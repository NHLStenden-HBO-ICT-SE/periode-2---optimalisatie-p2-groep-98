#pragma once
#include "precomp.h"

enum class Collider {
	TANK, ROCKET, BEAM
};

class Collidable
{

public:
	/// <summary>
	/// Used to get the type of object and cast it
	/// </summary>
	Collider collider_type;
	/// <summary>
	/// Get the current position
	/// </summary>
	virtual vec2 col_get_current_position() = 0;
	/// <summary>
	/// Get the collision radius
	/// </summary>
	virtual float col_get_collision_radius() = 0;
};




