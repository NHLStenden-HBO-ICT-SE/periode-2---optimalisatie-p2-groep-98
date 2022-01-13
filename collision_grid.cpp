#include "precomp.h"
#include "collision_grid.h"

void CollisionGrid::initializeTilesNeighbours()
{
    for (size_t y = 0; y < tiles.size(); y++)
    {
        array< CollisionTile, width> horizontal = tiles.at(y);
        for (size_t x = 0; x < horizontal.size(); x++)
        {
            CollisionTile* target = this->getTile(x, y);
            
            CollisionTile* neighbours[8] = {
                this->getTile(x, y + 1),        //up
                this->getTile(x, y - 1),        //down
                this->getTile(x - 1, y),        //left
                this->getTile(x + 1, y),            //right
                this->getTile(x + 1, y + 1),    //r_up
                this->getTile(x - 1, y + 1),    //l_up
                this->getTile(x + 1, y - 1),    //r_down
                this->getTile(x - 1, y - 1)     //l_down

            };

            //Looping over array to check if it is not a nullptr.
            for (size_t i = 0; i < 8; i++)
            {
                CollisionTile* t = neighbours[i];
                if (t) {
                    target->addNeighbour(t);
                }
            }
            

        }
    }
}

CollisionTile* CollisionGrid::getTile(int x, int y)
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
        vec2 p_lo = getTileIndex(lo);
        vec2 p_lb = getTileIndex(lb);
        vec2 p_ro = getTileIndex(ro);
        vec2 p_rb = getTileIndex(rb);
        
        //Get all the tiles in between the corners
        for (int x = p_lo.x; x < p_ro.x + 1; x++) {
            for (int y = p_lo.y; y < p_lb.y + 1; y++) {
                mlock->lock();
                getTile(x, y)->addCollidable(col);
                mlock->unlock();
            }
        }
        return;
    }


    vec2& pos = col->col_get_current_position();
    CollisionTile* tile = getTileFor(pos);
    mlock->lock();
    tile->objects.push_back(col);
    mlock->unlock();
}


vec2 CollisionGrid::getTileIndex(const vec2 pos) {
    size_t pos_x = pos.x / divider;
    size_t pos_y = pos.y / divider;

    pos_x = min(pos_x, width - 1);
    pos_y = min(pos_y, height - 1);
    return vec2(pos_x, pos_y);
}

CollisionTile* CollisionGrid::getTileFor(const vec2& pos)
{
    
    size_t pos_x = pos.x / divider;
    size_t pos_y = pos.y / divider;
    
    pos_x = min(pos_x, width-1);
    pos_y = min(pos_y, height-1);

    return &tiles.at(pos_y).at(pos_x);
}

void CollisionGrid::clearGrid()
{
    for (size_t y = 0; y < tiles.size(); y++)
    {
        array< CollisionTile, width>& horizontal = tiles.at(y);
        for (size_t x = 0; x < horizontal.size(); x++)
        {
            CollisionTile& item = horizontal.at(x);

            //Remove everything except for the BEAMS
            item.objects.clear();
        }
    }
}


vector<Collidable*> CollisionTile::getPossibleCollidables()
{
    vector<Collidable*> results = this->objects;

    
    for (CollisionTile* tile : this->neighbours) {
        results.insert(results.end(), tile->objects.begin(), tile->objects.end());
    }
    results.insert(results.end(), this->beams.begin(), this->beams.end());

    return results;
}

void CollisionTile::addCollidable(Collidable* c)
{
    if (c->collider_type == Collider::BEAM) {
        this->beams.push_back(c);
        return;
    }
    this->objects.push_back(c);
}

void CollisionTile::addNeighbour(CollisionTile* neighbour)
{
    if (neighbour) {
        this->neighbours.push_back(neighbour);
    }

}


