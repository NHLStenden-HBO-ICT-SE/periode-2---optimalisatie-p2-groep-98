#pragma once
#include "precomp.h"


#pragma once
#include "vector";
#include "Tank.h"
#include <string>
class QuadTree
{
public:
	int MAX_OBJECTS = 5;
	int MAX_LEVELS = 4;
	int level = 0;
	int start_x;
	int start_y;
	int width;
	int height;
	bool has_child_nodes = false;
	bool has_parent = false;
	std::string formatname;
	std::vector<Collidable*> objects;
	std::vector<QuadTree*> child_nodes;

	QuadTree* parent;

	QuadTree(int level, int startx, int starty, int width, int height, std::string name, QuadTree* parent);
	void pushAllNodesTo(std::vector<QuadTree*>& result);
	std::vector<std::vector<Collidable*>> getCollisionReadyNodes();
	void clear();
	void split();
	QuadTree& addObject(Collidable& t);
	bool doesFit(Collidable& t);
};

