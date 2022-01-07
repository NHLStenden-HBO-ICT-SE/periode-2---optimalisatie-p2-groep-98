#pragma once

namespace Tmpl8
{

    using uint = unsigned int;
    using HeuristicFunction = std::function<uint(vec2, vec2)>;

    struct Node
    {
        uint G, H;
        vec2 coordinates;
        Node* parent;

        Node(vec2 coord_, Node* parent_ = nullptr);
        uint getScore();
    };

    using NodeSet = std::vector<Node*>;

    class Heuristic
    {
        static vec2 getDelta(vec2 source_, vec2 target_);

    public:
        static uint manhattan(vec2 source_, vec2 target_);
        static uint euclidean(vec2 source_, vec2 target_);
        static uint octagonal(vec2 source_, vec2 target_);
    };

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
        //TerrainTile *up, *down, *left, *right;
        vector<TerrainTile*> exits;
        bool visited = false;

        size_t position_x;
        size_t position_y;

        TileType tile_type;

    private:
    };

    class Terrain
    {
        bool detectCollision(vec2 coordinates_);
        Node* findNodeOnList(NodeSet& nodes_, vec2 coordinates_);
        void releaseNodes(NodeSet& nodes_);
    public:

        Terrain();

        void update();
        void draw(Surface* target) const;
        static void test(Tank src, vec2 dest);
        //Use Breadth-first search to find shortest route to the destination
        vector<vec2> get_route(const Tank& tank, const vec2& target);

        float get_speed_modifier(const vec2& position) const;
        void setWorldSize(vec2 worldSize_);
        void setDiagonalMovement(bool enable_);
        void setHeuristic(HeuristicFunction heuristic_);
        vector<vec2> findPath(vec2 source_, vec2 target_);
        void addCollision(vec2 coordinates_);
        void removeCollision(vec2 coordinates_);
        void clearCollisions();

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

        inline static std::array<std::array<TerrainTile, terrain_width>, terrain_height> tiles;

        HeuristicFunction heuristic;
        vector<vec2> direction, walls;
        vec2 worldSize;
        uint directions;
    };

}