#pragma once
#include "precomp.h"
#include <vector>
class CollisionTile
{
public:

    vector<CollisionTile*> neighbours;
    

    vector<Collidable*> getObjects();
    /// <summary>
    /// Get all Collidables in tiles around the tile
    /// </summary>
    /// <returns>All near collidables</returns>
    vector<Collidable*> getPossibleCollidables();

    vector<Collidable*> objects;
    vector<Collidable*> beams;
    /// <summary>
    /// Add a collidable to the tile
    /// </summary>
    /// <param name="c">Collidable to add</param>
    void addCollidable(Collidable* c);
    /// <summary>
    /// Add a neighbour tile to the current tile. Should only be used in initialization.
    /// </summary>
    /// <param name="neighbour">The new neighbour</param>
    void addNeighbour(CollisionTile* neighbour);


};
class CollisionGrid
{
public:
    /// <summary>
    /// Calculate all neighbour tiles. Should only be run once.
    /// Needs the tiles array to be filled.
    /// </summary>
    void initializeTilesNeighbours();
    /// <summary>
    /// Get tile at index
    /// </summary>
    /// <param name="x">X index</param>
    /// <param name="y">Y index</param>
    /// <returns>Tile at x,y in the tile array</returns>
    CollisionTile* getTile(int x, int y);
    /// <summary>
    /// Put the collidable in the correct tile according to its position.
    /// </summary>
    /// <param name="col">Collidable to add</param>
    void updateTile(Collidable* col);
    /// <summary>
    /// Get a tile from a world-position.
    /// </summary>
    /// <param name="pos">World position</param>
    /// <returns>Tile where the worldposition lies in</returns>
    CollisionTile* getTileFor(const vec2& pos);
    /// <summary>
    /// Clear all objects from the grid except for the particle beams.
    /// </summary>
    void clearGrid();


private:
    vec2 getTileIndex(const vec2 pos);

    // ---- Grid tile width and height ----
    //Screen: 1280 x 720
    //Original width: 80
    //Original height: 45
    //Scale them proportionally, width + 16 and height + 9
    static constexpr size_t width = 128;
    static constexpr size_t height = 72;

    //Divider used to find tile index by world position.
    static constexpr size_t divider = 1280/width;

    //2D Array holding all tiles
    std::array<std::array<CollisionTile, width>, height> tiles;

};


