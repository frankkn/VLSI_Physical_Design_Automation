#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#define TIME_LIMIT 300

int best_cutsize = 1e+9;

std::unordered_map<int,std::pair<int,int>> Cells;
std::unordered_map<int,std::unordered_set<int>> CellArray;
std::unordered_map<int,std::unordered_set<int>> NetArray;
std::unordered_map<int, bool> LockedCells;
std::unordered_map<int, int> GainTable;
std::unordered_set<int> result_A;
std::unordered_map<int, std::pair<int,int>> netGroupCnt;

bool CanMove(int cell_num);

class Set
{
  private:
    int area;
    std::unordered_set<int> cellSet;
  public:
    Set():area(0), cellSet{} {}
    
    void addCell(int cell_name, int size);
    void deleteCell(int cell_name, int size);
    int getArea() { return area; }
    int getSize() { return cellSet.size(); };
    bool isIn(int cell_num) { return cellSet.count(cell_num); }
    std::unordered_set<int>& getCellSet() { return cellSet; };
}A, B;

class Cluster
{
  private:
    // int bucket_size;
    std::map<int, std::unordered_set<int>, std::greater<int>> GainList;
  public:
    Cluster() { InitBucketList(); }
    // int getSize() { return bucket_size; };
    void insertCell(int cell_name);
    void removeCell(int cell_name);
    void moveToOtherSide();
    int getBaseCell();
    void InitBucketList();
    std::map<int, std::unordered_set<int>, std::greater<int>> getGainList() { return GainList; } ;
};

void Set::addCell(int cell_name, int size)
{
  cellSet.insert(cell_name);
  area += size;
} 

void Set::deleteCell(int cell_name, int size)
{
  cellSet.erase(cell_name);
  area -= size;
}

void Cluster::insertCell(int cell_name)
{
  int cur_gain = GainTable[cell_name];
  GainList[cur_gain].insert(cell_name);
  // ++bucket_size;
}

int Cluster::getBaseCell()
{
  for(auto& [gain, cells]: GainList)
  {
    if(cells.size() == 0)
    {
      continue;
    }
    else
    {
      for(auto& cell: cells)
      {
        if(!LockedCells[cell] && CanMove(cell))
        {
          return cell;
        }
      }
    }
  }
  return -1;
}

void Cluster::removeCell(int cell_name)
{
  int cur_gain = GainTable[cell_name];
  GainList[cur_gain].erase(cell_name);
  // --bucket_size;
}

void Cluster::InitBucketList()
{
  for(auto& [cell_name, cell_gain]: GainTable)
  {
    GainList[cell_gain].insert(cell_name);
  }
}

void calNetGroup()
{
  for(auto& [net_name, cells]: NetArray)
  {
    netGroupCnt[net_name].first = netGroupCnt[net_name].second = 0;
    for(auto& cell: cells)
    {
      if(A.isIn(cell))
      {
        netGroupCnt[net_name].first++;
      }
      else
      { 
        netGroupCnt[net_name].second++;
      }
    }
  }
}

int calCutSize()
{
  calNetGroup();
  int cut_cnt = 0;
  for(auto& [net_name, cells]:NetArray)
  {
    if(netGroupCnt[net_name].first != 0 && netGroupCnt[net_name].second != 0)
    {
      ++cut_cnt;
    }
  }
  return cut_cnt;
}

void InitGainTable()
{	
  calNetGroup();
  for(auto& [cell_name, nets]:CellArray)
  {
    GainTable[cell_name] = 0;
    if(A.isIn(cell_name))
    {
      for(auto& net:nets)
      {
        if(netGroupCnt[net].first == 1) ++GainTable[cell_name];
        if(netGroupCnt[net].second == 0) --GainTable[cell_name];
      }
    }
    else 
    {
      for(auto& net:nets)
      {
        if(netGroupCnt[net].second == 1) ++GainTable[cell_name];
        if(netGroupCnt[net].first == 0) --GainTable[cell_name];
      }
    }
  }
}

void updateGain(Cluster& BucketList, int base)
{
  LockedCells[base] = true;
  // Base is in Set 0
  if(A.isIn(base))
  {
    A.deleteCell(base, Cells[base].first);
    B.addCell(base, Cells[base].second);
    for(auto& net: CellArray[base])
    {
      // To == 0
      if(netGroupCnt[net].second == 0)
      {
        for(auto& cell:NetArray[net])
        {
          if(cell != base && !LockedCells[cell])
          {
            BucketList.removeCell(cell);
            ++GainTable[cell];
            BucketList.insertCell(cell);
          }
        }
      }
      // To == 1
      else if(netGroupCnt[net].second == 1)
      {
        for(auto& cell: NetArray[net])
        {
          if(cell != base && !LockedCells[cell] && B.isIn(cell))
          {
            BucketList.removeCell(cell);
            --GainTable[cell];
            BucketList.insertCell(cell);
          }
        }
      }

      // From--, To++
      --netGroupCnt[net].first;
      ++netGroupCnt[net].second;

      // From == 0
      if(netGroupCnt[net].first == 0)
      {
        for(auto& cell: NetArray[net])
        {
          if(cell != base && !LockedCells[cell] && B.isIn(cell))
          {
            BucketList.removeCell(cell);
            --GainTable[cell];
            BucketList.insertCell(cell);
          }
        }
      }
      // From == 1
      else if(netGroupCnt[net].first == 1)
      {
        for(auto& cell: NetArray[net])
        {
          if(cell != base && !LockedCells[cell] && A.isIn(cell))
          {
            BucketList.removeCell(cell);
            ++GainTable[cell];
            BucketList.insertCell(cell);
          }
        }
      }
    }
  }
  // Base is in Set 1
  else
  {
    B.deleteCell(base, Cells[base].second);
    A.addCell(base, Cells[base].first);
    for(auto& net: CellArray[base])
    {
      // To == 0
      if(netGroupCnt[net].first == 0)
      {
        for(auto& cell:NetArray[net])
        {
          if(cell != base && !LockedCells[cell])
          {
            BucketList.removeCell(cell);
            ++GainTable[cell];
            BucketList.insertCell(cell);
          }
        }
      }
      // To == 1
      else if(netGroupCnt[net].first == 1)
      {
        for(auto& cell: NetArray[net])
        {
          if(cell != base && !LockedCells[cell] && A.isIn(cell))
          {
            BucketList.removeCell(cell);
            --GainTable[cell];
            BucketList.insertCell(cell);
          }
        }
      }

      // From--, To++
      --netGroupCnt[net].second;
      ++netGroupCnt[net].first;

      // From == 0
      if(netGroupCnt[net].second == 0)
      {
        for(auto& cell: NetArray[net])
        {
          if(cell != base && !LockedCells[cell] && A.isIn(cell))
          {
            BucketList.removeCell(cell);
            --GainTable[cell];
            BucketList.insertCell(cell);
          }
        }
      }
      // From == 1
      else if(netGroupCnt[net].second == 1)
      {
        for(auto& cell: NetArray[net])
        {
          if(cell != base && !LockedCells[cell] && B.isIn(cell))
          {
            BucketList.removeCell(cell);
            ++GainTable[cell];
            BucketList.insertCell(cell);
          }
        }
      }
    }
  }
}

bool CanMove(int cell_num)
{
  int tmp_A_area = A.getArea(), tmp_B_area = B.getArea();
  if(A.isIn(cell_num))
  {  
    tmp_A_area -= Cells[cell_num].first;
    tmp_B_area += Cells[cell_num].second;
  }
  else
  {
    tmp_B_area -= Cells[cell_num].second;
    tmp_A_area += Cells[cell_num].first;
  }
  long long diff_area = llabs(tmp_A_area - tmp_B_area);
  long long total_area = (tmp_A_area + tmp_B_area) / 10;
  return diff_area - total_area < 0? true: false;
}

void UnlockAllCells()
{
  for(auto& [cell, state]: LockedCells)
  {
    state = false;
  }
}

int fmProcess(Cluster& BucketList, std::chrono::high_resolution_clock::time_point begin, int pass)
{
  if(pass != 1) InitGainTable();
  int lock_num = 0;  
  int cur_cutsize = calCutSize();
  int partialSum = 0;
  while(lock_num < Cells.size())
  {
    auto end = std::chrono::high_resolution_clock::now();
	  auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

    if(elapsed.count() * 1e-9 > (TIME_LIMIT - (Cells.size()/100000)*1 ))
    {
      return partialSum;
    }

    int base = BucketList.getBaseCell();
    if(pass != 1 && GainTable[base] <= 0) break;
    if(base != -1)
    {
      partialSum += GainTable[base];
      cur_cutsize -= GainTable[base];
      
      updateGain(BucketList, base);

      if(cur_cutsize < best_cutsize)
      {
        best_cutsize = cur_cutsize;
        result_A = A.getCellSet();
      }
    }
    else
    {
      UnlockAllCells();
      break;
    }
    lock_num++;
  }
  return partialSum;
}

void WriteResult(std::string filename, int best_cutsize)
{
  std::ofstream output(filename);

  int size_of_set_A = result_A.size();
  output << "cut_size " << best_cutsize << std::endl;
  output << "A " << size_of_set_A <<std::endl;
  for(auto &cell : result_A)
  {
    output << "c" << cell << std::endl;
  }

  output << "B " << Cells.size() - size_of_set_A << std::endl;
  for(auto &[cell_name, nets]:CellArray)
  {
    if(!result_A.count(cell_name))
    {
      output << "c" << cell_name << std::endl;
    }
  }
}

int main(int argc , char *argv[])
{
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  std::ios::sync_with_stdio(false);
  auto begin = std::chrono::high_resolution_clock::now();
  std::ifstream fin_cell(argv[1]);
  std::ifstream fin_nets(argv[2]);

  // Step 1: Read Cell Area (size in Set A/Set B)
  std::string cell_name;
  int size_a, size_b;
  while(fin_cell >> cell_name >> size_a >> size_b)
  {
    cell_name.erase(0,1);
    int cell_num = stoi(cell_name);
    Cells.emplace(cell_num, std::make_pair(size_a,size_b));
    GainTable.emplace(cell_num, 0);
  }

  // Step 2: Construct NetArray and CellArray
  std::string tmp, net_name;
  while(fin_nets >> tmp >> net_name >> tmp)
  {
    int net_num = stoi(net_name.erase(0,1));
    while(fin_nets >> cell_name && cell_name[0] != '}')
    {
      int cell_num = stoi(cell_name.erase(0,1));
      NetArray[net_num].insert(cell_num);
      CellArray[cell_num].insert(net_num);
      LockedCells.emplace(cell_num, false);
    }
  }

  auto Input_end = std::chrono::high_resolution_clock::now();
	auto Input_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(Input_end - begin);
	std::cout<< "Input Time = "<<  Input_elapsed.count() * 1e-9 << "seconds" << "\n";

  // Step 3: Init Partition
  // First move all the cell into A, 
  // Then pick one from A to B until meet balance condition

  //int cur_seed = std::stoi(argv[3]);
  std::vector<int> random_cells; 
  for(auto& [cell_name, size]:Cells)
  {
    random_cells.emplace_back(cell_name);
  }

  switch(random_cells.size())
  {
    case 1000:
      std::srand(1006103);
      break;
    case 10000:
      std::srand(14083);
      break;
    case 100000:
      std::srand(19154);
      break;
    // case 200000:
    //   std::srand(28723);
    //   break;
    // case 400000:
    //   std::srand(18017);
    //   break;
    default:
      break;
  }

  //srand(cur_seed);
  if(random_cells.size() <= 100000)
  {
    std::random_shuffle(random_cells.begin(), random_cells.end());
  }
  for(auto& cell_name:random_cells)
  {
    int size_a = Cells[cell_name].first;
    A.addCell(cell_name, size_a);
  }
  for(auto& cell_name:random_cells)
  {
    int size_a = Cells[cell_name].first;
    int size_b = Cells[cell_name].second;
    long long diff_area = llabs(A.getArea() - B.getArea());
    long long total_area = (A.getArea() + B.getArea()) / 10;
    bool balance = diff_area - total_area < 0? true: false;
    if(!balance)
    {
      A.deleteCell(cell_name, size_a);
      B.addCell(cell_name, size_b);
    }
  }


  // for(auto& [cell_name, size]:Cells)
  // {
  //   A.addCell(cell_name, size.first);
  // }
  // for(auto& [cell_name, size]:Cells)
  // {
  //   long long diff_area = llabs(A.getArea() - B.getArea());
  //   long long total_area = (A.getArea() + B.getArea()) / 10;
  //   bool balance = diff_area - total_area < 0? true: false;
  //   if(!balance)
  //   {
  //     A.deleteCell(cell_name, size.first);
  //     B.addCell(cell_name, size.second);
  //   }
  // }

  // // Step 4: Buildup GainTable and Bucketlist
  InitGainTable();
  Cluster BucketList;

  // Step 5: FM process 
  // Move one cell with max gain from A to B 
  // int best_cutsize = fmProcess(BucketList);
  int pass = 0;
  while(true)
  {
    ++pass;
    int partialSum = fmProcess(BucketList, begin, pass);
    if(partialSum <= 0)
    {
      // std::cout << "After FM, Size of cut size = " << best_cutsize << "\n";
      break;
    }
  }

  auto FM_end = std::chrono::high_resolution_clock::now();
	auto FM_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(FM_end - Input_end);
	std::cout << "FM Time = "<<  FM_elapsed.count() * 1e-9 << "seconds" << "\n";
  std::cout << "Best cut size = " << best_cutsize << "\n";

  WriteResult(argv[3], best_cutsize);

  auto Output_end = std::chrono::high_resolution_clock::now();
	auto Output_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(Output_end - FM_end);
	std::cout<< "Output Time = "<<  Output_elapsed.count() * 1e-9 << "seconds" << "\n";

  std::cout << "I/O Time = " << (Input_elapsed + Output_elapsed).count() * 1e-9 << "seconds" << "\n";
  auto total_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(Output_end - begin);
  std::cout << "Total Time = " << total_elapsed.count() * 1e-9 << "seconds" << "\n";

  return 0;
}