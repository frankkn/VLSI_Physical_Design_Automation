#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
using namespace std;

class Cell
{
  public:
    string name;
    vector<int> cellArea;
    int set;
    int gain;
    bool locked;
    vector<Net*> nets;

    Cell(string name, vector<int> cellArea):name(name), cellArea(cellArea), set(-1), locked(false) {}
};

class Net
{
  public:
    string name;
    vector<Cell*> cells; 
    vector<int> setCnt;

    Net(string name):name(name), setCnt({0,0}) {}

};


class ListNode
{
  Cell* cell;
  ListNode* next, *before;
  ListNode(Cell* cell):cell(cell), next(nullptr), before(nullptr) {}
};

class BucketList
{
  public:
    int size;
    unordered_map<int, ListNode*> bucketlist;

    BucketList():size(0) {}
};

class Set
{
  public:
    int area;
    unordered_set<Cell*> cells;

    Set() {}
    void addCell(Cell* cell, int num); 
    void deleteCell(Cell* cell, int num);
};
