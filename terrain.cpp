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

    // Creating a shortcut for int, int pair type
    typedef pair<int, int> Pair;

    // Creating a shortcut for pair<int, pair<int, int>> type
    typedef pair<double, pair<int, int> > pPair;

    // A structure to hold the necessary parameters
    struct cell {
        // Row and Column index of its parent
        // Note that 0 <= i <= terrain_height-1 & 0 <= j <= terrain_width-1
        int parent_i, parent_j;
        // f = g + h
        double f, g, h;
    };

    // A Utility Function to check whether given cell (row, col)
    // is a valid cell or not.
    bool is_valid(int row, int col)
    {
        // Returns true if row number and column number
        // is in range
        return (row >= 0) && (row < terrain_height) && (col >= 0)
            && (col < terrain_width);
    }

    // A Utility Function to check whether the given cell is
    // blocked or not
    bool is_unblocked(std::array<std::array<TerrainTile, terrain_width>, terrain_height> tiles, int row, int col)
    {
        // Returns true if the cell is not blocked else false
        if (tiles.at(row).at(col).tile_type != TileType::MOUNTAINS || tiles.at(row).at(col).tile_type != TileType::WATER)
            return (true);
        else
            return (false);
    }

    // A Utility Function to check whether destination cell has
    // been reached or not
    bool is_destination(int row, int col, vec2 dest)
    {
        if (row == dest.x && col == dest.y)
            return (true);
        else
            return (false);
    }

    // A Utility Function to calculate the 'h' heuristics.
    double calculate_h_value(int row, int col, vec2 dest)
    {
        // Return using the distance formula
        return ((double)sqrt(
            (row - dest.x) * (row - dest.x)
            + (col - dest.y) * (col - dest.y)));
    }

    // A Utility Function to trace the path from the source
    // to destination
    void trace_path(cell cell_details[][terrain_width], vec2 dest)
    {
        cout << "\nThe Path is ";
        int row = dest.x;
        int col = dest.y;

        stack<Pair> Path;

        while (!(cell_details[row][col].parent_i == row
            && cell_details[row][col].parent_j == col)) {
            Path.push(make_pair(row, col));
            int temp_row = cell_details[row][col].parent_i;
            int temp_col = cell_details[row][col].parent_j;
            row = temp_row;
            col = temp_col;
        }

        Path.push(make_pair(row, col));
        while (!Path.empty()) {
            pair<int, int> p = Path.top();
            Path.pop();
            printf("-> (%d,%d) ", p.first, p.second);
        }

        return;
    }

    // A Function to find the shortest path between
    // a given source cell to a destination cell according
    // to A* Search Algorithm
    void Terrain::a_star_search(Tank src, vec2 dest)
    {
        // If the source is out of range
        if (is_valid(src.get_position().x, src.get_position().y) == false) {
            cout << "Source is invalid" << endl;
            return;
        }

        // If the destination is out of range
        if (is_valid(dest.x, dest.y) == false) {
            cout << "Destination is invalid" << endl;
            return;
        }

        // Either the source or the destination is blocked
        if (is_unblocked(tiles, src.get_position().x, src.get_position().y) == false
            || is_unblocked(tiles, dest.x, dest.y)
            == false) {
            cout << "Source or the destination is blocked" << endl;
            return;
        }

        // If the destination cell is the same as source cell
        if (is_destination(src.get_position().x, src.get_position().y, dest)
            == true) {
            cout << "We are already at the destination" << endl;
            return;
        }

        // Create a closed list and initialise it to false which
        // means that no cell has been included yet This closed
        // list is implemented as a boolean 2D array
        bool closed_list[terrain_height][terrain_width];
        memset(closed_list, false, sizeof(closed_list));

        // Declare a 2D array of structure to hold the details
        // of that cell
        cell cell_details[terrain_height][terrain_width];

        int i, j;

        for (i = 0; i < terrain_height; i++) {
            for (j = 0; j < terrain_width; j++) {
                cell_details[i][j].f = FLT_MAX;
                cell_details[i][j].g = FLT_MAX;
                cell_details[i][j].h = FLT_MAX;
                cell_details[i][j].parent_i = -1;
                cell_details[i][j].parent_j = -1;
            }
        }

        // Initialising the parameters of the starting node
        i = src.get_position().x, j = src.get_position().y;
        cell_details[i][j].f = 0.0;
        cell_details[i][j].g = 0.0;
        cell_details[i][j].h = 0.0;
        cell_details[i][j].parent_i = i;
        cell_details[i][j].parent_j = j;

        /*
         Create an open list having information as-
         <f, <i, j>>
         where f = g + h,
         and i, j are the row and column index of that cell
         Note that 0 <= i <= terrain_height-1 & 0 <= j <= terrain_width-1
         This open list is implemented as a set of pair of
         pair.*/
        set<pPair> open_list;

        // Put the starting cell on the open list and set its
        // 'f' as 0
        open_list.insert(make_pair(0.0, make_pair(i, j)));

        // We set this boolean value as false as initially
        // the destination is not reached.
        bool found_dest = false;

        while (!open_list.empty()) {
            pPair p = *open_list.begin();

            // Remove this vertex from the open list
            open_list.erase(open_list.begin());

            // Add this vertex to the closed list
            i = p.second.first;
            j = p.second.second;
            closed_list[i][j] = true;

            /*
             Generating all the 8 successor of this cell

                 N.W   N   N.E
                   \   |   /
                    \  |  /
                 W----Cell----E
                      / | \
                    /   |  \
                 S.W    S   S.E

             Cell-->Popped Cell (i, j)
             N -->  North       (i-1, j)
             S -->  South       (i+1, j)
             E -->  East        (i, j+1)
             W -->  West           (i, j-1)
             N.E--> North-East  (i-1, j+1)
             N.W--> North-West  (i-1, j-1)
             S.E--> South-East  (i+1, j+1)
             S.W--> South-West  (i+1, j-1)*/

             // To store the 'g', 'h' and 'f' of the 8 successors
            double gNew, hNew, fNew;

            //----------- 1st Successor (North) ------------

            // Only process this cell if this is a valid one
            if (is_valid(i - 1, j) == true) {
                // If the destination cell is the same as the
                // current successor
                if (is_destination(i - 1, j, dest) == true) {
                    // Set the Parent of the destination cell
                    cell_details[i - 1][j].parent_i = i;
                    cell_details[i - 1][j].parent_j = j;
                    cout << "The destination cell is found" << endl;
                    trace_path(cell_details, dest);
                    found_dest = true;
                    return;
                }
                // If the successor is already on the closed
                // list or if it is blocked, then ignore it.
                // Else do the following
                else if (closed_list[i - 1][j] == false
                    && is_unblocked(tiles, i - 1, j)
                    == true) {
                    gNew = cell_details[i][j].g + 1.0;
                    hNew = calculate_h_value(i - 1, j, dest);
                    fNew = gNew + hNew;

                    // If it isn’t on the open list, add it to
                    // the open list. Make the current square
                    // the parent of this square. Record the
                    // f, g, and h costs of the square cell
                    //                OR
                    // If it is on the open list already, check
                    // to see if this path to that square is
                    // better, using 'f' cost as the measure.
                    if (cell_details[i - 1][j].f == FLT_MAX
                        || cell_details[i - 1][j].f > fNew) {
                        open_list.insert(make_pair(
                            fNew, make_pair(i - 1, j)));

                        // Update the details of this cell
                        cell_details[i - 1][j].f = fNew;
                        cell_details[i - 1][j].g = gNew;
                        cell_details[i - 1][j].h = hNew;
                        cell_details[i - 1][j].parent_i = i;
                        cell_details[i - 1][j].parent_j = j;
                    }
                }
            }

            //----------- 2nd Successor (South) ------------

            // Only process this cell if this is a valid one
            if (is_valid(i + 1, j) == true) {
                // If the destination cell is the same as the
                // current successor
                if (is_destination(i + 1, j, dest) == true) {
                    // Set the Parent of the destination cell
                    cell_details[i + 1][j].parent_i = i;
                    cell_details[i + 1][j].parent_j = j;
                    cout << "The destination cell is found" << endl;
                    trace_path(cell_details, dest);
                    found_dest = true;
                    return;
                }
                // If the successor is already on the closed
                // list or if it is blocked, then ignore it.
                // Else do the following
                else if (closed_list[i + 1][j] == false
                    && is_unblocked(tiles, i + 1, j)
                    == true) {
                    gNew = cell_details[i][j].g + 1.0;
                    hNew = calculate_h_value(i + 1, j, dest);
                    fNew = gNew + hNew;

                    // If it isn’t on the open list, add it to
                    // the open list. Make the current square
                    // the parent of this square. Record the
                    // f, g, and h costs of the square cell
                    //                OR
                    // If it is on the open list already, check
                    // to see if this path to that square is
                    // better, using 'f' cost as the measure.
                    if (cell_details[i + 1][j].f == FLT_MAX
                        || cell_details[i + 1][j].f > fNew) {
                        open_list.insert(make_pair(
                            fNew, make_pair(i + 1, j)));
                        // Update the details of this cell
                        cell_details[i + 1][j].f = fNew;
                        cell_details[i + 1][j].g = gNew;
                        cell_details[i + 1][j].h = hNew;
                        cell_details[i + 1][j].parent_i = i;
                        cell_details[i + 1][j].parent_j = j;
                    }
                }
            }

            //----------- 3rd Successor (East) ------------

            // Only process this cell if this is a valid one
            if (is_valid(i, j + 1) == true) {
                // If the destination cell is the same as the
                // current successor
                if (is_destination(i, j + 1, dest) == true) {
                    // Set the Parent of the destination cell
                    cell_details[i][j + 1].parent_i = i;
                    cell_details[i][j + 1].parent_j = j;
                    cout << "The destination cell is found" << endl;
                    trace_path(cell_details, dest);
                    found_dest = true;
                    return;
                }

                // If the successor is already on the closed
                // list or if it is blocked, then ignore it.
                // Else do the following
                else if (closed_list[i][j + 1] == false
                    && is_unblocked(tiles, i, j + 1)
                    == true) {
                    gNew = cell_details[i][j].g + 1.0;
                    hNew = calculate_h_value(i, j + 1, dest);
                    fNew = gNew + hNew;

                    // If it isn’t on the open list, add it to
                    // the open list. Make the current square
                    // the parent of this square. Record the
                    // f, g, and h costs of the square cell
                    //                OR
                    // If it is on the open list already, check
                    // to see if this path to that square is
                    // better, using 'f' cost as the measure.
                    if (cell_details[i][j + 1].f == FLT_MAX
                        || cell_details[i][j + 1].f > fNew) {
                        open_list.insert(make_pair(
                            fNew, make_pair(i, j + 1)));

                        // Update the details of this cell
                        cell_details[i][j + 1].f = fNew;
                        cell_details[i][j + 1].g = gNew;
                        cell_details[i][j + 1].h = hNew;
                        cell_details[i][j + 1].parent_i = i;
                        cell_details[i][j + 1].parent_j = j;
                    }
                }
            }

            //----------- 4th Successor (West) ------------

            // Only process this cell if this is a valid one
            if (is_valid(i, j - 1) == true) {
                // If the destination cell is the same as the
                // current successor
                if (is_destination(i, j - 1, dest) == true) {
                    // Set the Parent of the destination cell
                    cell_details[i][j - 1].parent_i = i;
                    cell_details[i][j - 1].parent_j = j;
                    cout << "The destination cell is found" << endl;
                    trace_path(cell_details, dest);
                    found_dest = true;
                    return;
                }

                // If the successor is already on the closed
                // list or if it is blocked, then ignore it.
                // Else do the following
                else if (closed_list[i][j - 1] == false
                    && is_unblocked(tiles, i, j - 1)
                    == true) {
                    gNew = cell_details[i][j].g + 1.0;
                    hNew = calculate_h_value(i, j - 1, dest);
                    fNew = gNew + hNew;

                    // If it isn’t on the open list, add it to
                    // the open list. Make the current square
                    // the parent of this square. Record the
                    // f, g, and h costs of the square cell
                    //                OR
                    // If it is on the open list already, check
                    // to see if this path to that square is
                    // better, using 'f' cost as the measure.
                    if (cell_details[i][j - 1].f == FLT_MAX
                        || cell_details[i][j - 1].f > fNew) {
                        open_list.insert(make_pair(
                            fNew, make_pair(i, j - 1)));

                        // Update the details of this cell
                        cell_details[i][j - 1].f = fNew;
                        cell_details[i][j - 1].g = gNew;
                        cell_details[i][j - 1].h = hNew;
                        cell_details[i][j - 1].parent_i = i;
                        cell_details[i][j - 1].parent_j = j;
                    }
                }
            }

            //----------- 5th Successor (North-East)
            //------------

            // Only process this cell if this is a valid one
            if (is_valid(i - 1, j + 1) == true) {
                // If the destination cell is the same as the
                // current successor
                if (is_destination(i - 1, j + 1, dest) == true) {
                    // Set the Parent of the destination cell
                    cell_details[i - 1][j + 1].parent_i = i;
                    cell_details[i - 1][j + 1].parent_j = j;
                    cout << "The destination cell is found" << endl;
                    trace_path(cell_details, dest);
                    found_dest = true;
                    return;
                }

                // If the successor is already on the closed
                // list or if it is blocked, then ignore it.
                // Else do the following
                else if (closed_list[i - 1][j + 1] == false
                    && is_unblocked(tiles, i - 1, j + 1)
                    == true) {
                    gNew = cell_details[i][j].g + 1.414;
                    hNew = calculate_h_value(i - 1, j + 1, dest);
                    fNew = gNew + hNew;

                    // If it isn’t on the open list, add it to
                    // the open list. Make the current square
                    // the parent of this square. Record the
                    // f, g, and h costs of the square cell
                    //                OR
                    // If it is on the open list already, check
                    // to see if this path to that square is
                    // better, using 'f' cost as the measure.
                    if (cell_details[i - 1][j + 1].f == FLT_MAX
                        || cell_details[i - 1][j + 1].f > fNew) {
                        open_list.insert(make_pair(
                            fNew, make_pair(i - 1, j + 1)));

                        // Update the details of this cell
                        cell_details[i - 1][j + 1].f = fNew;
                        cell_details[i - 1][j + 1].g = gNew;
                        cell_details[i - 1][j + 1].h = hNew;
                        cell_details[i - 1][j + 1].parent_i = i;
                        cell_details[i - 1][j + 1].parent_j = j;
                    }
                }
            }

            //----------- 6th Successor (North-West)
            //------------

            // Only process this cell if this is a valid one
            if (is_valid(i - 1, j - 1) == true) {
                // If the destination cell is the same as the
                // current successor
                if (is_destination(i - 1, j - 1, dest) == true) {
                    // Set the Parent of the destination cell
                    cell_details[i - 1][j - 1].parent_i = i;
                    cell_details[i - 1][j - 1].parent_j = j;
                    cout << "The destination cell is found" << endl;
                    trace_path(cell_details, dest);
                    found_dest = true;
                    return;
                }

                // If the successor is already on the closed
                // list or if it is blocked, then ignore it.
                // Else do the following
                else if (closed_list[i - 1][j - 1] == false
                    && is_unblocked(tiles, i - 1, j - 1)
                    == true) {
                    gNew = cell_details[i][j].g + 1.414;
                    hNew = calculate_h_value(i - 1, j - 1, dest);
                    fNew = gNew + hNew;

                    // If it isn’t on the open list, add it to
                    // the open list. Make the current square
                    // the parent of this square. Record the
                    // f, g, and h costs of the square cell
                    //                OR
                    // If it is on the open list already, check
                    // to see if this path to that square is
                    // better, using 'f' cost as the measure.
                    if (cell_details[i - 1][j - 1].f == FLT_MAX
                        || cell_details[i - 1][j - 1].f > fNew) {
                        open_list.insert(make_pair(
                            fNew, make_pair(i - 1, j - 1)));
                        // Update the details of this cell
                        cell_details[i - 1][j - 1].f = fNew;
                        cell_details[i - 1][j - 1].g = gNew;
                        cell_details[i - 1][j - 1].h = hNew;
                        cell_details[i - 1][j - 1].parent_i = i;
                        cell_details[i - 1][j - 1].parent_j = j;
                    }
                }
            }

            //----------- 7th Successor (South-East)
            //------------

            // Only process this cell if this is a valid one
            if (is_valid(i + 1, j + 1) == true) {
                // If the destination cell is the same as the
                // current successor
                if (is_destination(i + 1, j + 1, dest) == true) {
                    // Set the Parent of the destination cell
                    cell_details[i + 1][j + 1].parent_i = i;
                    cell_details[i + 1][j + 1].parent_j = j;
                    cout << "The destination cell is found" << endl;
                    trace_path(cell_details, dest);
                    found_dest = true;
                    return;
                }

                // If the successor is already on the closed
                // list or if it is blocked, then ignore it.
                // Else do the following
                else if (closed_list[i + 1][j + 1] == false
                    && is_unblocked(tiles, i + 1, j + 1)
                    == true) {
                    gNew = cell_details[i][j].g + 1.414;
                    hNew = calculate_h_value(i + 1, j + 1, dest);
                    fNew = gNew + hNew;

                    // If it isn’t on the open list, add it to
                    // the open list. Make the current square
                    // the parent of this square. Record the
                    // f, g, and h costs of the square cell
                    //                OR
                    // If it is on the open list already, check
                    // to see if this path to that square is
                    // better, using 'f' cost as the measure.
                    if (cell_details[i + 1][j + 1].f == FLT_MAX
                        || cell_details[i + 1][j + 1].f > fNew) {
                        open_list.insert(make_pair(
                            fNew, make_pair(i + 1, j + 1)));

                        // Update the details of this cell
                        cell_details[i + 1][j + 1].f = fNew;
                        cell_details[i + 1][j + 1].g = gNew;
                        cell_details[i + 1][j + 1].h = hNew;
                        cell_details[i + 1][j + 1].parent_i = i;
                        cell_details[i + 1][j + 1].parent_j = j;
                    }
                }
            }

            //----------- 8th Successor (South-West)
            //------------

            // Only process this cell if this is a valid one
            if (is_valid(i + 1, j - 1) == true) {
                // If the destination cell is the same as the
                // current successor
                if (is_destination(i + 1, j - 1, dest) == true) {
                    // Set the Parent of the destination cell
                    cell_details[i + 1][j - 1].parent_i = i;
                    cell_details[i + 1][j - 1].parent_j = j;
                    cout << "The destination cell is found" << endl;
                    trace_path(cell_details, dest);
                    found_dest = true;
                    return;
                }

                // If the successor is already on the closed
                // list or if it is blocked, then ignore it.
                // Else do the following
                else if (closed_list[i + 1][j - 1] == false
                    && is_unblocked(tiles, i + 1, j - 1)
                    == true) {
                    gNew = cell_details[i][j].g + 1.414;
                    hNew = calculate_h_value(i + 1, j - 1, dest);
                    fNew = gNew + hNew;

                    // If it isn’t on the open list, add it to
                    // the open list. Make the current square
                    // the parent of this square. Record the
                    // f, g, and h costs of the square cell
                    //                OR
                    // If it is on the open list already, check
                    // to see if this path to that square is
                    // better, using 'f' cost as the measure.
                    if (cell_details[i + 1][j - 1].f == FLT_MAX
                        || cell_details[i + 1][j - 1].f > fNew) {
                        open_list.insert(make_pair(
                            fNew, make_pair(i + 1, j - 1)));

                        // Update the details of this cell
                        cell_details[i + 1][j - 1].f = fNew;
                        cell_details[i + 1][j - 1].g = gNew;
                        cell_details[i + 1][j - 1].h = hNew;
                        cell_details[i + 1][j - 1].parent_i = i;
                        cell_details[i + 1][j - 1].parent_j = j;
                    }
                }
            }
        }

        // When the destination cell is not found and the open
        // list is empty, then we conclude that we failed to
        // reach the destination cell. This may happen when the
        // there is no way to destination cell (due to
        // blockages)
        if (found_dest == false)
            cout << "Failed to find the Destination Cell" << endl;

        return;
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