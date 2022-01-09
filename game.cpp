#include "precomp.h" // include (only) this in every .cpp file
#include <chrono>
#include <iostream>
#include <stack>
#include <stdlib.h>
#include <map>
#include <algorithm>
#include <iterator>
#include <vector>
constexpr auto num_tanks_blue = 2048;
constexpr auto num_tanks_red = 2048;

constexpr auto tank_max_health = 1000;
constexpr auto rocket_hit_value = 60;
constexpr auto particle_beam_hit_value = 50;

constexpr auto tank_max_speed = 1.0;

constexpr auto health_bar_width = 70;

constexpr auto max_frames = 2000;

//Global performance timer
constexpr auto REF_PERFORMANCE = 29696.7; //UPDATE THIS WITH YOUR REFERENCE PERFORMANCE (see console after 2k frames)
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

const int NUM_OF_THREADS = std::thread::hardware_concurrency() * 2;
ThreadPool* pool = new ThreadPool(NUM_OF_THREADS);
std::mutex mlock;

vector<Tank> inactive_tanks;





CollisionGrid* grid = new CollisionGrid();


// -----------------------------------------------------------
// Initialize the simulation state
// This function does not count for the performance multiplier
// (Feel free to optimize anyway though ;) )
// -----------------------------------------------------------
void Game::init()
{
    frame_count_font = new Font("assets/digital_small.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ:?!=-0123456789.");

    active_tanks.reserve(num_tanks_blue + num_tanks_red);

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
        active_tanks.push_back(Tank(position.x, position.y, BLUE, &tank_blue, &smoke, 1100.f, position.y + 16, tank_radius, tank_max_health, tank_max_speed));
    }
    //Spawn red tanks
    for (int i = 0; i < num_tanks_red; i++)
    {
        vec2 position{ start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing) };
        active_tanks.push_back(Tank(position.x, position.y, RED, &tank_red, &smoke, 100.f, position.y + 16, tank_radius, tank_max_health, tank_max_speed));
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
    for (int i = 0; i < active_tanks.size(); i++)
    {
        if (active_tanks.at(i).allignment != current_tank.allignment && active_tanks.at(i).active)
        {
            float sqr_dist = fabsf((active_tanks.at(i).get_position() - current_tank.get_position()).sqr_length());
            if (sqr_dist < closest_distance)
            {
                closest_distance = sqr_dist;
                closest_index = i;
            }
        }
    }

    return active_tanks.at(closest_index);
}

vector<vec2> points_on_hull;

//Checks if a point lies on the left of an arbitrary angled line
bool Tmpl8::Game::left_of_line(vec2 line_start, vec2 line_end, vec2 point)
{
    return ((line_end.x - line_start.x) * (point.y - line_start.y) - (line_end.y - line_start.y) * (point.x - line_start.x)) < 0;
}






// Find next to top in a stack
vec2 next_to_top(stack<vec2>& S)
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

// Prints convex hull of a set of n points.
void convex_hull(vector<vec2> points)
{
    int i = 0;
    // Find the bottommost point
    float ymin = points.at(0).y, min = 0;
    for (int i = 1; i < points.size(); i++)
    {
        float y = points.at(i).y;

        // Pick the bottom-most or chose the left most point in case of tie
        if ((y < ymin) || (ymin == y && points.at(i).x < points.at(min).x))
            ymin = points.at(i).y, min = i;
    }

    // Place the bottom-most point at first position
    swap(points.at(0), points.at(min));

    // Sort n-1 points with respect to the first point.
    // A point p1 comes before p2 in sorted output if p2 has larger polar angle (in counterclockwise direction) than p1

    vec2* arr_points = new vec2[points.size() ];
    for (size_t i = 0; i < points.size(); i++)
    {
        arr_points[i] = points.at(i);
    }

    Sorting::convex_merge_sort(arr_points, 0, points.size() - 1);

    
    int size = points.size();



    // If two or more points make same angle with p0,
    // Remove all but the one that is farthest from p0
    int m = 1;
    for (int i = 1; i < size; i++)
    {
        // Keep removing i while angle of i and i+1 is same with respect to p0
        while (i < size - 1 && Sorting::orientation(arr_points[0], arr_points[i],
            arr_points[i + 1]) == 0)
            i++;


        points.at(m) = points.at(i);
        m++;
    }

    // If modified array of points has less than 3 points, convex hull is not possible
    if (m < 3)
        return;

    stack<vec2> S;
    S.push(arr_points[0]);
    S.push(arr_points[1]);
    S.push(arr_points[2]);

    // Process remaining n-3 points
    for (int i = 3; i < m; i++)
    {
        // Keep removing top while the angle formed by points next-to-top, top, and points[i] makes a non-left turn
        while (S.size() > 1 && Sorting::orientation(next_to_top(S), S.top(), arr_points[i]) != 2)
            S.pop();
        S.push(arr_points[i]);
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
    delete[] arr_points;
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
        std::vector<future<void>> route_threads;
        route_threads.reserve(NUM_OF_THREADS);
        int low = 0;
        int high;

        for (int i = 0; i < NUM_OF_THREADS; i++, low += active_tanks.size() / NUM_OF_THREADS)
        {
            high = low + active_tanks.size() / NUM_OF_THREADS;
            if (i == NUM_OF_THREADS - 1)
                high = active_tanks.size();
            route_threads.push_back(pool->enqueue([&, low, high]() {
                for (int j = low; j < high; j++)
                {
                    Tank& t = active_tanks.at(j);
                    t.set_route(background_terrain.get_route(t, t.target));
                }
                }));
        }
        for (size_t i = 0; i < route_threads.size(); i++)
        {
            route_threads.at(i).wait();
        }
        //background_terrain.initializeTilesNeighbours();
        grid->initializeTilesNeighbours();
     
        for (Particle_beam& particle_beam : particle_beams) {
            grid->updateTile(&particle_beam);


        }
    }

    grid->clearGrid();

    

    vector<Tank> tt;
    for (Tank& t : active_tanks) {
        tt.push_back(t);
        grid->updateTile(&t);
    }
    for (Rocket& r : rockets) {
        grid->updateTile(&r);
    }
    


    int collisions = 0;
    int TANK_COUNT = active_tanks.size();
    int LIST_SIZE = TANK_COUNT / NUM_OF_THREADS;
    int SPLIT_AMOUNT = NUM_OF_THREADS;



    vector<future<void>> threads;

    //COLLISION DETECTION
    //Split the list into SPLIT_AMOUNT parts and give each part to a thread
    for (size_t x = 0; x < SPLIT_AMOUNT; x++)
    {
        int startAt = 0;
        startAt = x * active_tanks.size() / SPLIT_AMOUNT;

        threads.push_back(pool->enqueue([&, startAt]() {
        for (size_t i = 0; i < active_tanks.size() / SPLIT_AMOUNT; i++)
        {
            Tank& tank = active_tanks.at(i + startAt);

                           
                
            CollisionTile* tank_tile = grid->getTileFor(tank.get_position());
            //Get a list of all collidables near the tile
            vector<Collidable*> possible_collisions = tank_tile->getPossibleCollidables();
            for (Collidable* other : possible_collisions) {
                if (&tank == other) continue;

                //Collidable for making sure the tank only hits the particle once in a frame
                // Because multiple tiles around the tank contain the same particle collidable
                Particle_beam *hitParticle = nullptr;

                //Since the beam does not have a position and round collision field, check it first.
                if (other->collider_type == Collider::BEAM && hitParticle != other) {
                    Particle_beam particle_beam = *dynamic_cast<Particle_beam*>(other);
                    vec2 pos = tank.col_get_current_position();
                    float radius = tank.col_get_collision_radius();
                    if (tank.active && particle_beam.rectangle.intersects_circle(pos, radius) )
                    {
                        if (tank.hit(particle_beam.damage))
                        {
                            hitParticle = &particle_beam;
                            smokes.push_back(Smoke(smoke, tank.position - vec2(0, 48)));
                        }
                    }
                }

                //Check if "other" collides with the tank
                vec2 dir = tank.get_position() - other->col_get_current_position();
                float dir_squared_len = dir.sqr_length();

                float col_squared_len = (tank.get_collision_radius() + other->col_get_collision_radius());
                col_squared_len *= col_squared_len;

                //No collision > continue in for loop
                if (dir_squared_len > col_squared_len)
                {
                    continue;

                }

                //Tank collision
                if (other->collider_type == Collider::TANK) {
                    tank.push(dir.normalized(), 1.f);
                }


                //Rocket collision
                if (other->collider_type == Collider::ROCKET) {
                    Rocket* rocket = dynamic_cast<Rocket*>(other);
                    if (tank.active && (tank.allignment != rocket->allignment) && rocket->intersects(tank.position, tank.collision_radius))
                    {   
                        mlock.lock();
                        explosions.push_back(Explosion(&explosion, tank.position));
                        mlock.unlock();
                      
                        if (tank.hit(rocket_hit_value))
                        {
                            mlock.lock();
                            smokes.push_back(Smoke(smoke, tank.position - vec2(7, 24)));
                            mlock.unlock();
                        }

                        rocket->active = false;
                        break;
                    }
                }




            }
            }
            }));

    }

    for (future<void>& th : threads) {
        th.wait();
    }




    vector<vec2> points;

    //Calculate "forcefield" around active tanks, clears forcefield here
    forcefield_hull.clear();
    points_on_hull.clear();

    //Calculate convex hull for 'rocket barrier'
    auto begin = chrono::high_resolution_clock::now();
    //Update tanks
    for (Tank t : active_tanks) {
        if (!t.active) {
            inactive_tanks.push_back(t);
        }
    }
    active_tanks.erase(std::remove_if(active_tanks.begin(), active_tanks.end(), [](const Tank& tank) { return !tank.active; }), active_tanks.end());


    for (Tank& tank : active_tanks)
    {
                   
        //Move tanks according to speed and nudges (see above) also reload
        tank.tick(background_terrain);

        //Shoot at closest target if reloaded
        if (tank.rocket_reloaded())
        {
            Tank& target = find_closest_enemy(tank);

            rockets.push_back(Rocket(tank.position, (target.get_position() - tank.position).normalized() * 3, rocket_radius, tank.allignment, ((tank.allignment == RED) ? &rocket_red : &rocket_blue)));

            tank.reload_rocket();
        }
        //Pushes points for use in convex hull
        points.push_back({ tank.get_position().x, tank.get_position().y });
    }

   

    //Calculates points_on_hull
    convex_hull(points);
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
    }

    //Disable rockets if they collide with the "forcefield"
    //Hint: A point to convex hull intersection test might be better here? :) (Disable if outside)


    for (Rocket& rocket : rockets)
    {
        if (rocket.active && frame_count > 1)
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
        particle_beam.tick(active_tanks);

        
    }

    //Update explosion sprites and remove when done with remove erase idiom
    for (Explosion& explosion : explosions)
    {
        explosion.tick();
    }

    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& explosion) { return explosion.done(); }), explosions.end());
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
    for (Tank t : active_tanks) {
        t.draw(screen);
    }
    for (Tank t : inactive_tanks) {
        t.draw(screen);
    }
    

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



    size_t blue_count = 0;
    size_t red_count = 0;
    for (Tank t : active_tanks) {
        if (!t.active) continue;
        if (t.allignment == BLUE) {
            blue_count++;
        }
        else {
            red_count++;
        }
    }

    int* blue_tanks = new int[blue_count];
    int* red_tanks = new int[red_count];
    
        
    size_t red = 0;
    size_t blue = 0;

    for (int i = 0; i < active_tanks.size(); i++) {
        Tank current_tank = active_tanks.at(i);

        if (current_tank.allignment == BLUE) {
            //sorted_tanks_blue.emplace_back(current_tank);
            blue_tanks[blue] = int(current_tank.health);
            blue++;
        }
        else {
            //sorted_tanks_red.emplace_back(current_tank);
            red_tanks[red] = int(current_tank.health);
            red++;

        }
    }
    auto begin = chrono::high_resolution_clock::now();

    Sorting::health_merge_sort(blue_tanks, 0, blue_count -1);
    Sorting::health_merge_sort(red_tanks, 0, red_count-1);
    auto end = chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    cout << "Merge sort time: " << ms << endl;
    
    draw_health_bars(blue_tanks, 0, blue_count);

    draw_health_bars(red_tanks, 1, red_count);

    

    delete[] blue_tanks;
    delete[] red_tanks;


}








// -----------------------------------------------------------
// Draw the health bars based on the given tanks health values
// -----------------------------------------------------------
void Tmpl8::Game::draw_health_bars(const int sorted_health[], const int team,const int team_size)
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
    int draw_count = std::min(SCRHEIGHT, team_size);

    //Draw height is the screenheight : 720 or the amount of thanks left
    for (int i = 0; i < draw_count - 1; i++)
    {
        //Health bars are 1 pixel each
        int health_bar_start_y = i * 1;
        int health_bar_end_y = health_bar_start_y + 1;

        float health_fraction = (1 - ((double)sorted_health[i] / (double)tank_max_health));

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