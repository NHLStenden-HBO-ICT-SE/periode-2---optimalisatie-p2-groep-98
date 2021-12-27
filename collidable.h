#pragma once
enum class Collider {
	TANK, ROCKET
};

class Collidable
{
public: 
	virtual float getColRadius() = 0;
	virtual vec2& getCurrentPosition() = 0;
	Collider collider_type;
};


