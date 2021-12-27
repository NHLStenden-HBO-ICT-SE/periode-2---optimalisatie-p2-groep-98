#pragma once


namespace Tmpl8
{
    enum TileType
    {
        GRASS,
        FORREST,
        ROCKS,
        MOUNTAINS,
        WATER
    };

    class TerrainTile
    {
    public:
        TerrainTile *up, *down, *left, *right, *r_up, *r_down, *l_up, *l_down;
        vector<TerrainTile*> exits;
        bool visited = false;

        size_t position_x;
        size_t position_y;

        TileType tile_type;
        
        void defineTilesAround(TerrainTile* up, TerrainTile* down, TerrainTile* left, TerrainTile* right, TerrainTile* r_up, TerrainTile* r_down, TerrainTile* l_up, TerrainTile* l_down);
        
        vector<Collidable*> getObjects();
        vector<Collidable*> getPossibleCollidables();
        vector<Collidable*> objects;
        

    private:
        

    };

    class Terrain
    {
    public:

        Terrain();

        void update();
        void draw(Surface* target) const;

        //Use Breadth-first search to find shortest route to the destination
        vector<vec2> get_route(const Tank& tank, const vec2& target);

        float get_speed_modifier(const vec2& position) const;
        void initializeTilesNeighbours();
        TerrainTile* getTile(int x, int y);
        void updateTile(Collidable* col, vec2& pos);
        TerrainTile* getTileFor(Collidable* col, const vec2& pos);
        void clearGrid();

    private:

        bool is_accessible(int y, int x);

        static constexpr int sprite_size = 16;
        static constexpr size_t terrain_width = 80;
        static constexpr size_t terrain_height = 45;

        std::unique_ptr<Surface> grass_img;
        std::unique_ptr<Surface> forest_img;
        std::unique_ptr<Surface> rocks_img;
        std::unique_ptr<Surface> mountains_img;
        std::unique_ptr<Surface> water_img;

        std::unique_ptr<Sprite> tile_grass;
        std::unique_ptr<Sprite> tile_forest;
        std::unique_ptr<Sprite> tile_rocks;
        std::unique_ptr<Sprite> tile_mountains;
        std::unique_ptr<Sprite> tile_water;
        
        std::array<std::array<TerrainTile, terrain_width>, terrain_height> tiles;
    };
}