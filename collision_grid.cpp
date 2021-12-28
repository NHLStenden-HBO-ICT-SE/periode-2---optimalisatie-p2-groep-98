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

            target->addNeighbour(this->getTile(x, y + 1));        //up
            target->addNeighbour(this->getTile(x, y - 1));        //down
            target->addNeighbour(this->getTile(x - 1, y));        //left
            target->addNeighbour(this->getTile(x, y));            //right
            target->addNeighbour(this->getTile(x + 1, y + 1));    //r_up
            target->addNeighbour(this->getTile(x - 1, y + 1));    //l_up
            target->addNeighbour(this->getTile(x + 1, y - 1));    //r_down
            target->addNeighbour(this->getTile(x - 1, y - 1));    //l_down


            

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

void CollisionGrid::updateTile(Collidable* col, vec2& pos)
{
    CollisionTile* tile = getTileFor(col, pos);

    tile->objects.push_back(col);
}

CollisionTile* CollisionGrid::getTileFor(Collidable* col, const vec2& pos)
{
    size_t pos_x = pos.x / divider;
    size_t pos_y = pos.y / divider;
    TerrainTile* tile;
    
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

            
            item.objects.clear();

        }
    }
}



vector<Collidable*> CollisionTile::getObjects()
{
	return vector<Collidable*>();
}

vector<Collidable*> CollisionTile::getPossibleCollidables()
{
    vector<Collidable*> results = this->objects;

    
    for (CollisionTile* tile : this->neighbours) {
    results.insert(results.end(), tile->objects.begin(), tile->objects.end());

    }
    return results;
}

void CollisionTile::addNeighbour(CollisionTile* neighbour)
{
    if (neighbour) {
        this->neighbours.push_back(neighbour);
    }

}


