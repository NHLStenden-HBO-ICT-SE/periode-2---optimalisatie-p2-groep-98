#include "precomp.h"
#include "collision_grid.h"

void CollisionGrid::initialize_tiles_neighbours()
{
    for (int y = 0; y < tiles.size(); y++)
    {
        array< CollisionTile, width> horizontal = tiles.at(y);
        for (int x = 0; x < horizontal.size(); x++)
        {
            CollisionTile* target = this->get_tile(x, y);
            for (int x_dif = -1; x_dif <= 1; x_dif++)
            {
                
                for (int y_dif = -1; y_dif <= 1; y_dif++) {
                    int use_x = x + x_dif;
                    int use_y = y + y_dif;
                    //Don't add itself as neighbour
                    if (x_dif == 0 && y_dif == 0) continue;
                    if (tile_exists(use_x, use_y)) {
                        target->add_neighbour(get_tile(use_x,use_y));
                    }
                }
            }

        }
    }
}

CollisionTile* CollisionGrid::get_tile(int x, int y)
{
    CollisionTile* tile;
        try {
            tile = &tiles.at(y).at(x);

        }
        catch (const std::out_of_range& oor) {
            return nullptr;
        }
        return tile;
}

void CollisionGrid::update_tile(Collidable* col)
{
    //If the collidable is a particle beam, generate all the tiles it covers
    if (col->collider_type == Collider::BEAM) {
        Particle_beam* beam = dynamic_cast<Particle_beam*>(col);
        
        //All corners
        vec2 lo = beam->min_position;
        vec2 lb = vec2(beam->min_position.x, beam->max_position.y);
        vec2 ro = vec2(beam->max_position.x, beam->min_position.y);
        vec2 rb = beam->max_position;


        //Get the tile indexes for each corner
        vec2 p_lo = get_tile_index(lo);
        vec2 p_lb = get_tile_index(lb);
        vec2 p_ro = get_tile_index(ro);
        vec2 p_rb = get_tile_index(rb);
        
        //Get all the tiles in between the corners
        for (int x = p_lo.x; x < p_ro.x + 1; x++) {
            for (int y = p_lo.y; y < p_lb.y + 1; y++) {
                mlock->lock();
                get_tile(x, y)->add_collidable(col);
                mlock->unlock();
            }
        } 
        return;
    }


    vec2& pos = col->col_get_current_position();
    CollisionTile* tile = get_tile_for(pos);
    mlock->lock();
    tile->objects.push_back(col);
    mlock->unlock();
}


bool CollisionGrid::tile_exists(int x, int y)
{
    return x >= 0 && x < width&& y >= 0 && y < height;
}

vec2 CollisionGrid::get_tile_index(vec2 pos) {
    size_t pos_x = pos.x / divider;
    size_t pos_y = pos.y / divider;

    pos_x = min(pos_x, width - 1);
    pos_y = min(pos_y, height - 1);
    return vec2(pos_x, pos_y);
}

CollisionTile* CollisionGrid::get_tile_for(vec2 pos)
{
    
    size_t pos_x = pos.x / divider;
    size_t pos_y = pos.y / divider;
    
    pos_x = min(pos_x, width-1);
    pos_y = min(pos_y, height-1);

    return &tiles.at(pos_y).at(pos_x);
}

void CollisionGrid::clear_grid()
{
    for (int y = 0; y < tiles.size(); y++)
    {
        array< CollisionTile, width>& horizontal = tiles.at(y);
        for (int x = 0; x < horizontal.size(); x++)
        {
            CollisionTile& item = horizontal.at(x);

            //Remove everything except for the BEAMS
            item.objects.clear();
        }
    }
}


vector<Collidable*> CollisionTile::get_possible_collidables()
{
    vector<Collidable*> results = this->objects;

    
    for (CollisionTile* tile : this->neighbours) {
        results.insert(results.end(), tile->objects.begin(), tile->objects.end());
    }
    results.insert(results.end(), this->beams.begin(), this->beams.end());

    return results;
}

void CollisionTile::add_collidable(Collidable* c)
{
    if (c->collider_type == Collider::BEAM) {
        this->beams.push_back(c);
        return;
    }
    this->objects.push_back(c);
}

void CollisionTile::add_neighbour(CollisionTile* neighbour)
{
    if (neighbour) {
        this->neighbours.push_back(neighbour);
    }

}




