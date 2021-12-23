#pragma once
#include "precomp.h"


#pragma once
#include "vector";
#include "Tank.h"
#include <string>
class QuadTree
{
public:
	int MAX_OBJECTS = 10;
	int MAX_LEVELS = 3;
	int level = 0;
	int start_x;
	int start_y;
	int width;
	int height;
	bool has_child_nodes = false;
	bool has_parent = false;
	std::string formatname;
	std::vector<Tank*> objects;
	std::vector<QuadTree*> child_nodes;

	QuadTree* parent;

	QuadTree(int level, int startx, int starty, int width, int height, std::string name, QuadTree* parent);
	void pushAllNodesTo(std::vector<QuadTree*>& result);
	std::vector<std::vector<Tank*>> getCollisionReadyNodes();
	void clear();
	void split();
	QuadTree& addObject(Tank& t);
	bool doesFit(Tank& t);
};

