#include "precomp.h" // include (only) this in every .cpp file
#include <chrono>
#include <iostream>
#include <stack>
#include <stdlib.h>
#include <map>

constexpr auto num_tanks_blue = 2048;
constexpr auto num_tanks_red = 2048;

constexpr auto tank_max_health = 1000;
constexpr auto rocket_hit_value = 60;
constexpr auto particle_beam_hit_value = 50;

constexpr auto tank_max_speed = 1.0;

constexpr auto health_bar_width = 70;

constexpr auto max_frames = 2000;

//Global performance timer
constexpr auto REF_PERFORMANCE = 99619; //UPDATE THIS WITH YOUR REFERENCE PERFORMANCE (see console after 2k frames)
static timer perf_timer;
static float duration;

//Load sprite files and initialize sprites
static Surface* tank_red_img = new Surface("assets/Tank_Proj2.png");
static Surface* tank_blue_img = new Surface("assets/Tank_Blue_Proj2.png");
static Surface* rocket_red_img = new Surface("assets/Rocket_Proj2.png");
static Surface* rocket_blue_img = new Surface("assets/Rocket_Blue_Proj2.png");
static Surface* particle_beam_img = new Surface("assets/Particle_Beam.png");
static Surface* smoke_img = new Surface("assets/Smoke.png");
static Surface* explosion_img = new Surface("assets/Explosion.png");

static Sprite tank_red(tank_red_img, 12);
static Sprite tank_blue(tank_blue_img, 12);
static Sprite rocket_red(rocket_red_img, 12);
static Sprite rocket_blue(rocket_blue_img, 12);
static Sprite smoke(smoke_img, 4);
static Sprite explosion(explosion_img, 9);
static Sprite particle_beam_sprite(particle_beam_img, 3);

const static vec2 tank_size(7, 9);
const static vec2 rocket_size(6, 6);

const static float tank_radius = 3.f;
const static float rocket_radius = 5.f;

const int num_of_threads = std::thread::hardware_concurrency() * 2;
ThreadPool* pool = new ThreadPool(num_of_threads);


// -----------------------------------------------------------
// Initialize the simulation state
// This function does not count for the performance multiplier
// (Feel free to optimize anyway though ;) )
// -----------------------------------------------------------
void Game::init()
{
    frame_count_font = new Font("assets/digital_small.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ:?!=-0123456789.");

    tanks.reserve(num_tanks_blue + num_tanks_red);

    uint max_rows = 24;

    float start_blue_x = tank_size.x + 40.0f;
    float start_blue_y = tank_size.y + 30.0f;

    float start_red_x = 1088.0f;
    float start_red_y = tank_size.y + 30.0f;

    float spacing = 7.5f;

    //Spawn blue tanks
    for (int i = 0; i < num_tanks_blue; i++)
    {
        vec2 position{ start_blue_x + ((i % max_rows) * spacing), start_blue_y + ((i / max_rows) * spacing) };
        tanks.push_back(Tank(position.x, position.y, BLUE, &tank_blue, &smoke, 1100.f, position.y + 16, tank_radius, tank_max_health, tank_max_speed));
    }
    //Spawn red tanks
    for (int i = 0; i < num_tanks_red; i++)
    {
        vec2 position{ start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing) };
        tanks.push_back(Tank(position.x, position.y, RED, &tank_red, &smoke, 100.f, position.y + 16, tank_radius, tank_max_health, tank_max_speed));
    }

    particle_beams.push_back(Particle_beam(vec2(590, 327), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
    particle_beams.push_back(Particle_beam(vec2(64, 64), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
    particle_beams.push_back(Particle_beam(vec2(1200, 600), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
    
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::shutdown()
{
}

// -----------------------------------------------------------
// Iterates through all tanks and returns the closest enemy tank for the given tank
// -----------------------------------------------------------
Tank& Game::find_closest_enemy(Tank& current_tank)
{
    float closest_distance = numeric_limits<float>::infinity();
    int closest_index = 0;

    for (int i = 0; i < tanks.size(); i++)
    {
        if (tanks.at(i).allignment != current_tank.allignment && tanks.at(i).active)
        {
            float sqr_dist = fabsf((tanks.at(i).get_position() - current_tank.get_position()).sqr_length());
            if (sqr_dist < closest_distance)
            {
                closest_distance = sqr_dist;
                closest_index = i;
            }
        }
    }

    return tanks.at(closest_index);
}

//Checks if a point lies on the left of an arbitrary angled line
bool Tmpl8::Game::left_of_line(vec2 line_start, vec2 line_end, vec2 point)
{
    return ((line_end.x - line_start.x) * (point.y - line_start.y) - (line_end.y - line_start.y) * (point.x - line_start.x)) < 0;
}


vec2 p0;
vector<vec2> points_on_hull;

// Find next to top in a stack
vec2 nextToTop(stack<vec2>& S)
{
    vec2 p = S.top();
    S.pop();
    vec2 res = S.top();
    S.push(p);
    return res;
}

// Swap two points
void swap(vec2& p1, vec2& p2)
{
    vec2 temp = p1;
    p1 = p2;
    p2 = temp;
}

// Square of distance between p1 and p2
int distSq(vec2 p1, vec2 p2)
{
    return (p1.x - p2.x) * (p1.x - p2.x) +
        (p1.y - p2.y) * (p1.y - p2.y);
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are collinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(vec2 p, vec2 q, vec2 r)
{
    int val = (q.y - p.y) * (r.x - q.x) -
        (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0;  // collinear
    return (val > 0) ? 1 : 2; // clock or counterclock wise
}

// A function used by library function qsort() to sort an array of points with respect to the first point
int compare(const void* vp1, const void* vp2)
{
    vec2* p1 = (vec2*)vp1;
    vec2* p2 = (vec2*)vp2;

    // Find orientation
    int o = orientation(p0, *p1, *p2);
    if (o == 0)
        return (distSq(p0, *p2) >= distSq(p0, *p1)) ? -1 : 1;

    return (o == 2) ? -1 : 1;
}

// Prints convex hull of a set of n points.
void convexHull(vector<vec2> points)
{
    int i = 0;
    // Find the bottommost point
    int ymin = points.at(0).y, min = 0;
    for (int i = 1; i < points.size(); i++)
    {
        int y = points.at(i).y;

        // Pick the bottom-most or chose the left most point in case of tie
        if ((y < ymin) || (ymin == y &&
            points.at(i).x < points.at(min).x))
            ymin = points.at(i).y, min = i;
    }

    // Place the bottom-most point at first position
    swap(points.at(0), points.at(min));

    // Sort n-1 points with respect to the first point.
    // A point p1 comes before p2 in sorted output if p2 has larger polar angle (in counterclockwise direction) than p1
    p0 = points.at(0);
    qsort(&points.at(1), points.size() - 1, sizeof(vec2), compare);

    // If two or more points make same angle with p0,
    // Remove all but the one that is farthest from p0
    int m = 1;
    for (int i = 1; i < points.size(); i++)
    {
        // Keep removing i while angle of i and i+1 is same with respect to p0
        while (i < points.size() - 1 && orientation(p0, points.at(i),
            points.at(i + 1)) == 0)
            i++;


        points.at(m) = points.at(i);
        m++;
    }

    // If modified array of points has less than 3 points, convex hull is not possible
    if (m < 3) return;

    stack<vec2> S;
    S.push(points.at(0));
    S.push(points.at(1));
    S.push(points.at(2));

    // Process remaining n-3 points
    for (int i = 3; i < m; i++)
    {
        // Keep removing top while the angle formed by points next-to-top, top, and points[i] makes a non-left turn
        while (S.size() > 1 && orientation(nextToTop(S), S.top(), points.at(i)) != 2)
            S.pop();
        S.push(points.at(i));
    }

    // Result stack has the output points
    while (!S.empty())
    {
        vec2 p = S.top();
        //cout << "(" << p.x << ", " << p.y << ")" << endl;

        points_on_hull.push_back(p);
        S.pop();

        i++;
    }
}
// -----------------------------------------------------------
// Update the game state:
// Move all objects
// Update sprite frames
// Collision detection
// Targeting etc..
// -----------------------------------------------------------
void Game::update(float deltaTime)
{
    //Calculate the route to the destination for each tank using BFS
    //Initializing routes here so it gets counted for performance..
    if (frame_count == 0)
    {
        background_terrain.initializeTilesNeighbours();
        for (Tank& t : tanks) {
            t.set_route(background_terrain.get_route(t, t.target));
        }
    }





   
    //Check tank collision and nudge tanks away from each other    //Optimize, create a list with active tanks instead of checking in the tanks list
    

    background_terrain.clearGrid();
    for (Tank& t : tanks) {
        background_terrain.updateTile(&t, t.getCurrentPosition());
        

    }
    
    int collisions = 0;
    for (Tank& tank : tanks) {
        //cout << "POS " << tank.get_position().x << endl;
        TerrainTile* til = background_terrain.getTileFor(&tank, tank.get_position());
        vector<Collidable*> possible_collisions = til->getPossibleCollidables();
        for (Collidable* t2 : possible_collisions) {
            if (&tank == t2) continue;

            vec2 dir = tank.get_position() - t2->getCurrentPosition();
            float dir_squared_len = dir.sqr_length();

            float col_squared_len = (tank.get_collision_radius() + t2->getCollisionRadius());
            col_squared_len *= col_squared_len;

            if (dir_squared_len < col_squared_len)
            {
                collisions++;
                tank.push(dir.normalized(), 1.f);
            }
        }


    }
    //cout << "COL COUNT : " << collisions << endl;




    //for (Tank& tank : tanks)
    //{
    //    if (tank.active)
    //    {
    //        //use the active tank list
    //        //What about a grid system where the  other_tank 's are only the tanks in the same grid block.
    //        //Name of the algorithm: The separate axis theorem
    //        for (Tank& other_tank : tanks)
    //        {
    //            if (&tank == &other_tank || !other_tank.active) continue;

    //            vec2 dir = tank.get_position() - other_tank.get_position();
    //            float dir_squared_len = dir.sqr_length();

    //            float col_squared_len = (tank.get_collision_radius() + other_tank.get_collision_radius());
    //            col_squared_len *= col_squared_len;

    //            if (dir_squared_len < col_squared_len)
    //            {
    //                tank.push(dir.normalized(), 1.f);
    //            }
    //        }
    //    }
    //}


    vector<vec2> points;

    //Calculate "forcefield" around active tanks, clears forcefield here
    forcefield_hull.clear();
    points_on_hull.clear();

    //Calculate convex hull for 'rocket barrier'
    auto begin = chrono::high_resolution_clock::now();
    //Update tanks
    for (Tank& tank : tanks)
    {
        if (tank.active)
        {
            //Pushes points for use in convex hull
            points.push_back({ tank.get_position().x, tank.get_position().y });

            //Move tanks according to speed and nudges (see above) also reload
            tank.tick(background_terrain);

            //Shoot at closest target if reloaded
            if (tank.rocket_reloaded())
            {
                Tank& target = find_closest_enemy(tank);

                rockets.push_back(Rocket(tank.position, (target.get_position() - tank.position).normalized() * 3, rocket_radius, tank.allignment, ((tank.allignment == RED) ? &rocket_red : &rocket_blue)));

                tank.reload_rocket();
            }
        }
    }
    tanks.erase(std::remove_if(tanks.begin(), tanks.end(), [](const Tank& tank) { return !tank.active; }), tanks.end());

    //Calculates points_on_hull
    convexHull(points);
    //Draws all points of the forcefield
    for (vec2& point_on_hull : points_on_hull)
        forcefield_hull.push_back(point_on_hull);

    auto end = chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    //cout << ms << endl;

    //Update smoke plumes
    for (Smoke& smoke : smokes)
    {
        smoke.tick();
    }

    //Update rockets
    for (Rocket& rocket : rockets)
    {
        rocket.tick();

        //Check if rocket collides with enemy tank, spawn explosion, and if tank is destroyed spawn a smoke plume
        for (Tank& tank : tanks)
        {
            if (tank.active && (tank.allignment != rocket.allignment) && rocket.intersects(tank.position, tank.collision_radius))
            {
                explosions.push_back(Explosion(&explosion, tank.position));

                if (tank.hit(rocket_hit_value))
                {
                    smokes.push_back(Smoke(smoke, tank.position - vec2(7, 24)));
                }

                rocket.active = false;
                break;
            }
        }
    }

    //Disable rockets if they collide with the "forcefield"
    //Hint: A point to convex hull intersection test might be better here? :) (Disable if outside)

    //Optimize: active_rocket list 

    for (Rocket& rocket : rockets)
    {
        if (rocket.active)
        {
            for (size_t i = 0; i < forcefield_hull.size(); i++)
            {
                if (circle_segment_intersect(forcefield_hull.at(i), forcefield_hull.at((i + 1) % forcefield_hull.size()), rocket.position, rocket.collision_radius))
                {
                    explosions.push_back(Explosion(&explosion, rocket.position));
                    rocket.active = false;
                }
            }
        }
    }



    //Remove exploded rockets with remove erase idiom
    rockets.erase(std::remove_if(rockets.begin(), rockets.end(), [](const Rocket& rocket) { return !rocket.active; }), rockets.end());

    //Update particle beams
    for (Particle_beam& particle_beam : particle_beams)
    {
        particle_beam.tick(tanks);

        //Damage all tanks within the damage window of the beam (the window is an axis-aligned bounding box)
        for (Tank& tank : tanks)
        {
            if (tank.active && particle_beam.rectangle.intersects_circle(tank.get_position(), tank.get_collision_radius()))
            {
                if (tank.hit(particle_beam.damage))
                {
                    smokes.push_back(Smoke(smoke, tank.position - vec2(0, 48)));
                }
            }
        }
    }

    //Update explosion sprites and remove when done with remove erase idiom
    for (Explosion& explosion : explosions)
    {
        explosion.tick();
    }

    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& explosion) { return explosion.done(); }), explosions.end());
}
void merge(vector<Tank>& list, int const left, int const mid, int const right, string sortOn)
{
    const int subList1 = mid - left + 1;
    const int subList2 = right - mid;


    //list.begin() + left, list.begin() + left + subArrayOne
    //list.begin() + mid, list.begin() + mid + subArrayTwo
    vector<Tank> leftArray = {  };
    vector<Tank> rightArray = {  };



    //De list splitten in de 2 sublijsten
    for (int i = 0; i < subList1; i++) {
        leftArray.push_back(list.at(left + i));

    }
    for (int j = 0; j < subList2; j++) {
        rightArray.push_back(list.at(mid + 1 + j));

    }


    int indexList1 = 0;
    int indexList2 = 0;
    int indexMerged = left;


    while (indexList1 < subList1 && indexList2 < subList2) {
        if (leftArray[indexList1].getProperty(sortOn) <= rightArray[indexList2].getProperty(sortOn)) {
            list[indexMerged] = leftArray[indexList1];
            indexList1++;
        }
        else {
            list[indexMerged] = rightArray[indexList2];
            indexList2++;
        }
        indexMerged++;
    }

    while (indexList1 < subList1) {
        list[indexMerged] = leftArray[indexList1];
        indexList1++;
        indexMerged++;
    }

    while (indexList2 < subList2) {
        list[indexMerged] = rightArray[indexList2];
        indexList2++;
        indexMerged++;
    }

}

void mergeSort(vector<Tank>& list, int const begin, int const end, string sortOn)
{
    if (begin >= end) {
        return;
    }
    int mid = begin + (end - begin) / 2;
    mergeSort(list, begin, mid, sortOn);
    mergeSort(list, mid + 1, end, sortOn);
    merge(list, begin, mid, end, sortOn);

}


// -----------------------------------------------------------
// Draw all sprites to the screen
// (It is not recommended to multi-thread this function)
// -----------------------------------------------------------
void Game::draw()
{
    // clear the graphics window
    screen->clear(0);

    //Draw background
    background_terrain.draw(screen);

    //Draw sprites
    for (Tank t : tanks) {
        t.draw(screen);

    }
    /*
    for (int i = 0; i < num_tanks_blue + num_tanks_red; i++)
    {
        tanks.at(i).draw(screen);

        vec2 tank_pos = tanks.at(i).get_position();
    }*/

    for (Rocket& rocket : rockets)
    {
        rocket.draw(screen);
    }

    for (Smoke& smoke : smokes)
    {
        smoke.draw(screen);
    }

    for (Particle_beam& particle_beam : particle_beams)
    {
        particle_beam.draw(screen);
    }

    for (Explosion& explosion : explosions)
    {
        explosion.draw(screen);
    }

    //Draw forcefield (mostly for debugging, its kinda ugly..)
    for (size_t i = 0; i < forcefield_hull.size(); i++)
    {
        vec2 line_start = forcefield_hull.at(i);
        vec2 line_end = forcefield_hull.at((i + 1) % forcefield_hull.size());
        line_start.x += HEALTHBAR_OFFSET;
        line_end.x += HEALTHBAR_OFFSET;
        screen->line(line_start, line_end, 0x0000ff);
    }

    //Draw sorted health bars

    const int NUM_TANKS = tanks.size();


    vector<Tank> toSort;
    for (Tank t : tanks) {
        toSort.push_back(t);
    }
    
    //mergeSort(toSort, 0, NUM_TANKS - 1, "health");
    //toSort.erase(std::remove_if(toSort.begin(), toSort.end(), [](Tank* tank) { return !tank->active; }), toSort.end());

    std::vector<Tank> sorted_tanks_blue;
    std::vector<Tank> sorted_tanks_red;
    for (int i = 0; i < toSort.size(); i++) {
        Tank current_tank = toSort.at(i);

        if (current_tank.allignment == BLUE) {
            sorted_tanks_blue.emplace_back(current_tank);
        }
        else {
            sorted_tanks_red.emplace_back(current_tank);

        }
    }
    draw_health_bars(sorted_tanks_blue, 0);

    draw_health_bars(sorted_tanks_red, 1);



}








// -----------------------------------------------------------
// Draw the health bars based on the given tanks health values
// -----------------------------------------------------------
void Tmpl8::Game::draw_health_bars(const std::vector<Tank>& sorted_tanks, const int team)
{
    int health_bar_start_x = (team < 1) ? 0 : (SCRWIDTH - HEALTHBAR_OFFSET) - 1;
    int health_bar_end_x = (team < 1) ? health_bar_width : health_bar_start_x + health_bar_width - 1;

    for (int i = 0; i < SCRHEIGHT - 1; i++)
    {
        //Health bars are 1 pixel each
        int health_bar_start_y = i * 1;
        int health_bar_end_y = health_bar_start_y + 1;

        screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
    }

    //Draw the <SCRHEIGHT> least healthy tank health bars
    int draw_count = std::min(SCRHEIGHT, (int)sorted_tanks.size());

    //Draw height is the screenheight : 720 or the amount of thanks left
    for (int i = 0; i < draw_count - 1; i++)
    {
        //Health bars are 1 pixel each
        int health_bar_start_y = i * 1;
        int health_bar_end_y = health_bar_start_y + 1;

        float health_fraction = (1 - ((double)sorted_tanks.at(i).health / (double)tank_max_health));

        if (team == 0) {
            screen->bar(health_bar_start_x + (int)((double)health_bar_width * health_fraction), health_bar_start_y, health_bar_end_x, health_bar_end_y, GREENMASK);
        }
        else { screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x - (int)((double)health_bar_width * health_fraction), health_bar_end_y, GREENMASK); }
    }
}

// -----------------------------------------------------------
// When we reach max_frames print the duration and speedup multiplier
// Updating REF_PERFORMANCE at the top of this file with the value
// on your machine gives you an idea of the speedup your optimizations give
// -----------------------------------------------------------
void Tmpl8::Game::measure_performance()
{
    char buffer[128];
    if (frame_count >= max_frames)
    {
        if (!lock_update)
        {
            duration = perf_timer.elapsed();
            cout << "Duration was: " << duration << " (Replace REF_PERFORMANCE with this value)" << endl;
            lock_update = true;
        }

        frame_count--;
    }

    if (lock_update)
    {
        screen->bar(420 + HEALTHBAR_OFFSET, 170, 870 + HEALTHBAR_OFFSET, 430, 0x030000);
        int ms = (int)duration % 1000, sec = ((int)duration / 1000) % 60, min = ((int)duration / 60000);
        sprintf(buffer, "%02i:%02i:%03i", min, sec, ms);
        frame_count_font->centre(screen, buffer, 200);
        sprintf(buffer, "SPEEDUP: %4.1f", REF_PERFORMANCE / duration);
        frame_count_font->centre(screen, buffer, 340);
    }
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::tick(float deltaTime)
{
    if (!lock_update)
    {
        update(deltaTime);
    }
    draw();

    measure_performance();

    // print something in the graphics window
    //screen->Print("hello world", 2, 2, 0xffffff);

    // print something to the text window
    //cout << "This goes to the console window." << std::endl;

    //Print frame count
    frame_count++;
    string frame_count_string = "FRAME: " + std::to_string(frame_count);
    frame_count_font->print(screen, frame_count_string.c_str(), 350, 580);
}