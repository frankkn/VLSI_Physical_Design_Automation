#pragma once
#include <string>
#include <vector>
using namespace std;

struct Cell
{
	string name;
	int width, height, weight;
	double x_global, y_global, x_final, y_final;

	Cell(string const &name, int const &width, int const &height)
		:name(name), width(width), height(height), weight(width), x_global(0), y_global(0), x_final(0), y_final(0) {}
};

struct Cluster
{
	double x_c, q_c;
	int e_c, w_c; // weight/width of cluster
	double x_tmp, e_tmp, q_tmp, w_tmp;
	vector<Cell*> nodes;
	Cluster *prevCluster;

	Cluster(Cluster *prevCluster, double const &x_c, int const &e_c, double const &q_c, int const &w_c)
		:prevCluster(prevCluster), x_c(x_c), e_c(e_c), q_c(q_c), w_c(w_c) {}
};

struct Subrow
{
	int x_min, x_max, capacity;
	Cluster *lastCluster;

	void updateInfo(int const &new_x_min, int const &new_x_max);

	Subrow(int const &x_min, int const &x_max)
		:x_min(x_min), x_max(x_max), capacity(x_max - x_min), lastCluster(nullptr) {}
};

struct Row
{
	int width, height, y;
	vector<Subrow*>subrows;

	Row(int &width, int &height, int &y):width(width), height(height), y(y) {}
};

struct Input
{
	string name;
	int maxDisplacement;
	vector<Cell*>cellList, terminalList;
	vector<Row*>rowList;

	Input(int &maxDisplacement, vector<Cell*> &cellList, vector<Cell*> &terminalList, vector<Row*> &rowList, string &name)
		:maxDisplacement(maxDisplacement), cellList(cellList), terminalList(terminalList), rowList(rowList), name(name) {}
};
