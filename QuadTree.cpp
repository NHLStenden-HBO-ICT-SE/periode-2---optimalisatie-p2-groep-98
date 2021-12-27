#include "precomp.h"
#include "QuadTree.h"
#include "vector"
#include <iostream>
#include <string>

using namespace std;
QuadTree::QuadTree(int level_given, int startx, int starty, int width, int height, string formatname, QuadTree* parent)
{

	this->level = int(level_given);
	this->start_x = startx;
	this->start_y = starty;
	this->width = width;
	this->height = height;
	this->formatname = formatname;
	this->parent = parent;
	this->has_child_nodes = false;
	if (parent) {
		this->has_parent = true;
	}
}

void QuadTree::pushAllNodesTo(vector<QuadTree*>& result)
{
	if (!this->has_child_nodes) {
		result.push_back(this);
	}
	else {
		for (QuadTree* node : this->child_nodes) {
			node->pushAllNodesTo(result);
		}
	}

}

std::vector<std::vector<Collidable*>> QuadTree::getCollisionReadyNodes() {

	//Getting all the lowest nodes from the tree
	vector<QuadTree*> results;
	this->pushAllNodesTo(results);


	std::vector<std::vector<Collidable*>> collision_ready_vector;

	//Adding all objects from the parents
	for (QuadTree* node : results) {
		vector<Collidable*> Collidables;
		QuadTree* usenode = node;
		Collidables.insert(Collidables.end(), node->objects.begin(), node->objects.end());
		while (usenode->has_parent) {
			Collidables.insert(Collidables.end(), usenode->parent->objects.begin(), usenode->parent->objects.end());
			usenode = usenode->parent;
		}

		collision_ready_vector.push_back(Collidables);
	}
	return collision_ready_vector;
}








void QuadTree::clear() {
	this->objects.clear();
	for (QuadTree* node : this->child_nodes) {
		node->clear();
	}
	this->has_child_nodes = false;

	this->child_nodes.clear();
}




void QuadTree::split() {
	this->has_child_nodes = true;
	this->child_nodes.resize(4);
	int subWidth = this->width / 2;
	int subHeight = this->height / 2;

	int x = this->start_x;
	int y = this->start_y;

	int subLevel = level + 1;
	//cout << "NEW LEVEL : " << subLevel << endl;
	this->child_nodes.at(0) = new QuadTree(subLevel, x + subWidth, y, subWidth, subHeight, "bottomright", this);
	this->child_nodes.at(1) = new QuadTree(subLevel, x, y, subWidth, subHeight, "bottomleft", this);
	this->child_nodes.at(2) = new QuadTree(subLevel, x, y + subHeight, subWidth, subHeight, "topleft", this);
	this->child_nodes.at(3) = new QuadTree(subLevel, x + subWidth, y + subHeight, subWidth, subHeight, "topright", this);
	//cout << "Children summoned on level " << subLevel << endl;

}



bool QuadTree::doesFit(Collidable& t) {
	int endx = start_x + width;
	int endy = start_y + height;
	float MIN_DISTANCE = 1.5;
	float px = t.getCurrentPosition().x;
	float py = t.getCurrentPosition().y;

	return endx - px >= MIN_DISTANCE && endy - py >= MIN_DISTANCE && px - start_x >= MIN_DISTANCE && py - start_y >= MIN_DISTANCE;

}


QuadTree& QuadTree::addObject(Collidable &t)
{
	if (!this->has_child_nodes && this->objects.size() > MAX_OBJECTS && level < MAX_LEVELS) {
		//If the list is to crowded, split the current node into 4 new ones and replace every object from the current list
			//Create the 4 sub squares
		this->split();
		
		//Copy the Collidables in current node to a temp list
		vector<Collidable*> templist = this->objects;
		//Clear the current node objects since everything will be moved into sub nodes
		this->objects.clear();


		//cout << "TESTING ONE " << this->child_nodes.at(0)->level << endl;

		for (Collidable* tt : templist) {
			this->addObject(*tt);
		}
		return this->addObject(t);



	}
	for (QuadTree* node : this->child_nodes) {
		if (node->doesFit(t)) {
			//cout << "adding to " << node->formatname << endl;
			return node->addObject(t);
		}

	}
	this->objects.push_back(&t);

	return *this;



}

