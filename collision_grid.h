#pragma once
#include "precomp.h"
#include <vector>
class CollisionTile
{
public:

    vector<CollisionTile*> neighbours;
    

    vector<Collidable*> getObjects();
    vector<Collidable*> getPossibleCollidables();
    vector<Collidable*> objects;
    void addNeighbour(CollisionTile* neighbour);


};
class CollisionGrid
{
public:
    void initializeTilesNeighbours();
    CollisionTile* getTile(int x, int y);
    void updateTile(Collidable* col);
    CollisionTile* getTileFor(const vec2& pos);
    void clearGrid();


private:
    vec2 getTileIndex(const vec2 pos);
    //Screen: 1280 x 720
    //Original width: 80
    //Original height: 45
    //Scale them proportionally, width + 16 and height + 9
    static constexpr size_t width = 128;
    static constexpr size_t height = 72;

    static constexpr size_t divider = 1280/width;


    std::array<std::array<CollisionTile, width>, height> tiles;

};


