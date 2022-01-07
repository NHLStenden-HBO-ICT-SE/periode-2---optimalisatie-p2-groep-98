#include "terrain.h"
#include "precomp.h"
#include <stack>
#include <set>

namespace fs = std::filesystem;
namespace Tmpl8
{

    static constexpr size_t terrain_width = 80;
    static constexpr size_t terrain_height = 45;
    std::array<std::array<TerrainTile, terrain_width>, terrain_height> tiles;

    Terrain::Terrain()
    {
        setWorldSize(vec2(terrain_width, terrain_height));
        setDiagonalMovement(false);
        setHeuristic(&Heuristic::manhattan);
        direction = {
            { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 },
            { -1, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 }
        };

        //Load in terrain sprites
        grass_img = std::make_unique<Surface>("assets/tile_grass.png");
        forest_img = std::make_unique<Surface>("assets/tile_forest.png");
        rocks_img = std::make_unique<Surface>("assets/tile_rocks.png");
        mountains_img = std::make_unique<Surface>("assets/tile_mountains.png");
        water_img = std::make_unique<Surface>("assets/tile_water.png");


        tile_grass = std::make_unique<Sprite>(grass_img.get(), 1);
        tile_forest = std::make_unique<Sprite>(forest_img.get(), 1);
        tile_rocks = std::make_unique<Sprite>(rocks_img.get(), 1);
        tile_water = std::make_unique<Sprite>(water_img.get(), 1);
        tile_mountains = std::make_unique<Sprite>(mountains_img.get(), 1);


        //Load terrain layout file and fill grid based on tiletypes
        fs::path terrain_file_path{ "assets/terrain.txt" };
        std::ifstream terrain_file(terrain_file_path);

        if (terrain_file.is_open())
        {
            std::string terrain_line;

            std::getline(terrain_file, terrain_line);
            std::istringstream lineStream(terrain_line);

            int rows;

            lineStream >> rows;
            for (size_t row = 0; row < rows; row++)
            {
                std::getline(terrain_file, terrain_line);

                for (size_t collumn = 0; collumn < terrain_line.size(); collumn++)
                {
                    switch (std::toupper(terrain_line.at(collumn)))
                    {
                    case 'G':
                        //terrain_grid[row][collumn] = 1;
                        Terrain::tiles.at(row).at(collumn).tile_type = TileType::GRASS;
                        break;
                    case 'F':
                        //terrain_grid[row][collumn] = 1;
                        Terrain::tiles.at(row).at(collumn).tile_type = TileType::FORREST;
                        break;
                    case 'R':
                        //terrain_grid[row][collumn] = 1;
                        Terrain::tiles.at(row).at(collumn).tile_type = TileType::ROCKS;
                        break;
                    case 'M':
                        //terrain_grid[row][collumn] = 0;
                        Terrain::tiles.at(row).at(collumn).tile_type = TileType::MOUNTAINS;
                        break;
                    case 'W':
                        //terrain_grid[row][collumn] = 0;
                        Terrain::tiles.at(row).at(collumn).tile_type = TileType::WATER;
                        break;
                    default:
                        //terrain_grid[row][collumn] = 1;
                        Terrain::tiles.at(row).at(collumn).tile_type = TileType::GRASS;
                        break;
                    }
                }
            }
        }
        else
        {
            std::cout << "Could not open terrain file! Is the path correct? Defaulting to grass.." << std::endl;
            std::cout << "Path was: " << terrain_file_path << std::endl;
        }

        //Instantiate tiles for path planning
        for (size_t y = 0; y < Terrain::tiles.size(); y++)
        {
            for (size_t x = 0; x < Terrain::tiles.at(y).size(); x++)
            {
                Terrain::tiles.at(y).at(x).position_x = x;
                Terrain::tiles.at(y).at(x).position_y = y;

                if (is_accessible(y, x + 1)) { Terrain::tiles.at(y).at(x).exits.push_back(&Terrain::tiles.at(y).at(x + 1)); }
                if (is_accessible(y, x - 1)) { Terrain::tiles.at(y).at(x).exits.push_back(&Terrain::tiles.at(y).at(x - 1)); }
                if (is_accessible(y + 1, x)) { Terrain::tiles.at(y).at(x).exits.push_back(&Terrain::tiles.at(y + 1).at(x)); }
                if (is_accessible(y - 1, x)) { Terrain::tiles.at(y).at(x).exits.push_back(&Terrain::tiles.at(y - 1).at(x)); }
            }
        }
    }

    void Terrain::update()
    {
        //Pretend there is animation code here.. next year :)
    }




    //bool Terrain::vec2::operator == (const vec2& coordinates_)
    //{
    //    return (x == coordinates_.x && y == coordinates_.y);
    //}

    //Terrain::vec2 operator + (const AStar::vec2& left_, const AStar::vec2& right_)
    //{
    //    return{ left_.x + right_.x, left_.y + right_.y };
    //}

    Node::Node(vec2 coordinates_, Node* parent_)
    {
        parent = parent_;
        coordinates = coordinates_;
        G = H = 0;
    }

    uint Node::getScore()
    {
        return G + H;
    }

    void Terrain::setWorldSize(vec2 worldSize_)
    {
        worldSize = worldSize_;
    }

    void Terrain::setDiagonalMovement(bool enable_)
    {
        directions = (enable_ ? 8 : 4);
    }

    void Terrain::setHeuristic(HeuristicFunction heuristic_)
    {
        heuristic = std::bind(heuristic_, placeholders::_1, placeholders::_2);
    }

    void Terrain::addCollision(vec2 coordinates_)
    {
        walls.push_back(coordinates_);
    }

    void Terrain::removeCollision(vec2 coordinates_)
    {
        auto it = std::find(walls.begin(), walls.end(), coordinates_);
        if (it != walls.end()) {
            walls.erase(it);
        }
    }

    void Terrain::clearCollisions()
    {
        walls.clear();
    }

    vector<vec2> Terrain::findPath(vec2 source_, vec2 target_)
    {
        Node* current = nullptr;
        NodeSet openSet, closedSet;
        openSet.reserve(100);
        closedSet.reserve(100);
        openSet.push_back(new Node(source_));

        while (!openSet.empty()) {
            auto current_it = openSet.begin();
            current = *current_it;

            for (auto it = openSet.begin(); it != openSet.end(); it++) {
                auto node = *it;
                if (node->getScore() <= current->getScore()) {
                    current = node;
                    current_it = it;
                }
            }

            if (current->coordinates == target_) {
                break;
            }

            closedSet.push_back(current);
            openSet.erase(current_it);

            for (uint i = 0; i < directions; ++i) {
                vec2 newCoordinates(current->coordinates + direction[i]);
                if (detectCollision(newCoordinates) || findNodeOnList(closedSet, newCoordinates))
                {
                    if (!is_accessible(newCoordinates.x, newCoordinates.y))
                        addCollision(newCoordinates);
                    continue;
                }

                uint totalCost = current->G + ((i < 4) ? 10 : 14);

                Node* successor = findNodeOnList(openSet, newCoordinates);
                if (successor == nullptr) {
                    successor = new Node(newCoordinates, current);
                    successor->G = totalCost;
                    successor->H = heuristic(successor->coordinates, target_);
                    openSet.push_back(successor);
                }
                else if (totalCost < successor->G) {
                    successor->parent = current;
                    successor->G = totalCost;
                }
            }
        }

        vector<vec2> path;
        while (current != nullptr) {
            path.push_back(current->coordinates);
            current = current->parent;
        }

        releaseNodes(openSet);
        releaseNodes(closedSet);

        return path;
    }

    Node* Terrain::findNodeOnList(NodeSet& nodes_, vec2 coordinates_)
    {
        for (auto node : nodes_) {
            if (node->coordinates == coordinates_) {
                return node;
            }
        }
        return nullptr;
    }

    void Terrain::releaseNodes(NodeSet& nodes_)
    {
        for (auto it = nodes_.begin(); it != nodes_.end();) {
            delete* it;
            it = nodes_.erase(it);
        }
    }

    bool Terrain::detectCollision(vec2 coordinates_)
    {
        if (coordinates_.x < 0 || coordinates_.x >= worldSize.x ||
            coordinates_.y < 0 || coordinates_.y >= worldSize.y ||
            std::find(walls.begin(), walls.end(), coordinates_) != walls.end()) {
            return true;
        }
        return false;
    }

    vec2 Heuristic::getDelta(vec2 source_, vec2 target_)
    {
        return{ abs(source_.x - target_.x),  abs(source_.y - target_.y) };
    }

    uint Heuristic::manhattan(vec2 source_, vec2 target_)
    {
        auto delta = std::move(getDelta(source_, target_));
        return static_cast<uint>(10 * (delta.x + delta.y));
    }

    uint Heuristic::euclidean(vec2 source_, vec2 target_)
    {
        auto delta = std::move(getDelta(source_, target_));
        return static_cast<uint>(10 * sqrt(pow(delta.x, 2) + pow(delta.y, 2)));
    }

    uint Heuristic::octagonal(vec2 source_, vec2 target_)
    {
        auto delta = std::move(getDelta(source_, target_));
        return 10 * (delta.x + delta.y) + (-6) * std::min(delta.x, delta.y);
    }



    void Terrain::draw(Surface* target) const
    {

        for (size_t y = 0; y < Terrain::tiles.size(); y++)
        {
            for (size_t x = 0; x < Terrain::tiles.at(y).size(); x++)
            {
                int posX = (x * sprite_size) + HEALTHBAR_OFFSET;
                int posY = y * sprite_size;

                switch (Terrain::tiles.at(y).at(x).tile_type)
                {
                case TileType::GRASS:
                    tile_grass->draw(target, posX, posY);
                    break;
                case TileType::FORREST:
                    tile_forest->draw(target, posX, posY);
                    break;
                case TileType::ROCKS:
                    tile_rocks->draw(target, posX, posY);
                    break;
                case TileType::MOUNTAINS:
                    tile_mountains->draw(target, posX, posY);
                    break;
                case TileType::WATER:
                    tile_water->draw(target, posX, posY);
                    break;
                default:
                    tile_grass->draw(target, posX, posY);
                    break;
                }
            }
        }
    }

    //Use Breadth-first search to find shortest route to the destination
    vector<vec2> Terrain::get_route(const Tank& tank, const vec2& target)
    {
        //Find start and target tile
        const size_t pos_x = tank.position.x / sprite_size;
        const size_t pos_y = tank.position.y / sprite_size;

        const size_t target_x = target.x / sprite_size;
        const size_t target_y = target.y / sprite_size;

        //Init queue with start tile
        std::queue<vector<TerrainTile*>> queue;
        queue.emplace();
        queue.back().push_back(&Terrain::tiles.at(pos_y).at(pos_x));

        std::vector<TerrainTile*> visited;

        bool route_found = false;
        vector<TerrainTile*> current_route;
        while (!queue.empty() && !route_found)
        {
            current_route = queue.front();
            queue.pop();
            TerrainTile* current_tile = current_route.back();

            //Check all exits, if target then done, else if unvisited push a new partial route
            for each (TerrainTile * exit in current_tile->exits)
            {
                if (exit->position_x == target_x && exit->position_y == target_y)
                {
                    current_route.push_back(exit);
                    route_found = true;
                    break;
                }
                else if (!exit->visited)
                {
                    exit->visited = true;
                    visited.push_back(exit);
                    queue.push(current_route);
                    queue.back().push_back(exit);
                }
            }
        }

        //Reset tiles
        for each (TerrainTile * tile in visited)
        {
            tile->visited = false;
        }

        if (route_found)
        {
            //Convert route to vec2 to prevent dangling pointers
            std::vector<vec2> route;
            for (TerrainTile* tile : current_route)
            {
                route.push_back(vec2((float)tile->position_x * sprite_size, (float)tile->position_y * sprite_size));
            }

            return route;
        }
        else
        {
            return  std::vector<vec2>();
        }

    }

    //TODO: Function not used, convert BFS to dijkstra and take speed into account next year :)
    float Terrain::get_speed_modifier(const vec2& position) const
    {
        const size_t pos_x = position.x / sprite_size;
        const size_t pos_y = position.y / sprite_size;

        switch (Terrain::tiles.at(pos_y).at(pos_x).tile_type)
        {
        case TileType::GRASS:
            return 1.0f;
            break;
        case TileType::FORREST:
            return 0.5f;
            break;
        case TileType::ROCKS:
            return 0.75f;
            break;
        case TileType::MOUNTAINS:
            return 0.0f;
            break;
        case TileType::WATER:
            return 0.0f;
            break;
        default:
            return 1.0f;
            break;
        }
    }

    bool Terrain::is_accessible(int y, int x)
    {
        //Bounds check
        if ((x >= 0 && x < terrain_width) && (y >= 0 && y < terrain_height))
        {
            //Inaccessible terrain check
            if (Terrain::tiles.at(y).at(x).tile_type != TileType::MOUNTAINS && Terrain::tiles.at(y).at(x).tile_type != TileType::WATER)
            {
                return true;
            }
        }

        return false;
    }
}