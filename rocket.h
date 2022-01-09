#pragma once

namespace Tmpl8
{

class Rocket : public Collidable
{
  public:
    Rocket(vec2 position, vec2 direction, float collision_radius, allignments allignment, Sprite* rocket_sprite);
    ~Rocket();

    void tick();
    void draw(Surface* screen);

    bool intersects(vec2 position_other, float radius_other) const;

    float col_get_collision_radius() { return this->collision_radius; };

    vec2 col_get_current_position();
    vec2 position;
    vec2 speed;

    float collision_radius;

    bool active;

    allignments allignment;

    int current_frame;
    Sprite* rocket_sprite;
};

} // namespace Tmpl8