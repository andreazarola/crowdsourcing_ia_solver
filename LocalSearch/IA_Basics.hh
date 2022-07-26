// File IA_Basics.hh
#ifndef IA_BASICS_HH
#define IA_BASICS_HH

#include <algorithm>
#include "Utils.hh"
#include "IA_Data.hh"
#include <utils/random.hh>

using namespace EasyLocal::Core;

class IA_State
{  
  friend ostream& operator<<(ostream&, const IA_State&);
  friend bool operator==(const IA_State& st1, const IA_State& st2);
  friend class IA_OutputManager;
public:
  IA_State(const IA_Input &in);
  IA_State& operator=(const IA_State& s);

  int GetAssignment(int w, int k) const { return assignments[w][k]; }
  void SetAssignment(int w, int k, int item, bool full_update);

  int GetItemAssignments(int i) const { return static_cast<int>(item_assignments[i].size()); }
  int GetItemAssignment(int i, int j) const { return static_cast<int>(item_assignments[i][j]); }

  bool WorkerItemAssignment(int w, int i) const { return static_cast<int>(worker_item_assignments[w][i]); }

  bool HasExceedingAssignments(int i) const { return  GetItemAssignments(i) > in.MinItemRepetitions(); }
  int WorkersDistanceSets(int w1, int w2) const {return workers_distance_sets[w1][w2];}
  int WorkersCategoryLevels(int w, int c, int l) const {return workers_category_levels[w][c][l];}

  int ItemPropertyLevels(int i, int p, int l) const {return items_property_levels[i][p][l];}

  int ItemQualityLevel(int i) const {return item_quality_level[i];}
  
  void SortAssignments();
  int ComputeDeltaDistanceSets(int w1, int w2, int i1, int i2) const;
  int ComputeDeltaCategoryWorkerAssignments(int w, int i1, int i2, int c) const;
  int ComputeDeltaPropertyItemAssignments(int i1, int i2, int p, int l) const;
  int ComputeDeltaChangeMinimumItemQualityLevel(int i1, int i2, int w) const;
  int ComputeDeltaSwapMinimumItemQualityLevel(int i1, int i2, int w1, int w2) const;
  int ComputeQualityForItem(int i, int w) const;
  int ComputeGreedySoftScore(int i, int w) const;


  bool IsConsistent(bool full_update) const;
  void Reset();

  int ComputeItemCategoryLevelsForWorker(int i, int w) const;
  int ComputeWorkerPropertiesLevelsForItem(int i, int w) const;
  int GetSurplusItemAssignments(int k) const {return surplus_item_assignments[k];}
  int NumSurplusItemAssignments() const {return static_cast<int>(surplus_item_assignments.size());}
  // int IndexSurplusItemAssignments(int i) const {return index_surplus_item_assignments[i];}

  int ComputeNumItemsLowQuality() const; // num items with a quality level lower than MinItemQualityLevel
  double ComputeFairnessIndex(int fairness_cost) const; // total fairness cost / (num_items * sum property levels)
  const IA_Input & in;
protected:
  vector<vector<int>> assignments; // for each worker: list of items, Workers X TotalWorkerAssignments

  // Redundant data
  vector<vector<int>> item_assignments; // for each item, the list of workers the item is assigned to 
  vector<vector<bool>> worker_item_assignments; // workers X items, 1 if item has been assigned to worker
  vector<vector<int>> workers_distance_sets; // workers X workers, Hamming distance (0 if they have the same item sets)
  vector<vector<vector<int>>> workers_category_levels; // workers X (num_categories X max_category_levels), some levels for some categories could be empty
  vector<vector<vector<int>>> items_property_levels; // items X num_properties X max_property_levels, some levels for some categories could be empty
  vector<int> surplus_item_assignments; //list of items exceeding the min repetitions
  // vector<int> index_surplus_item_assignments; // index of an item in the surplus_item_assignments
  vector<int> item_quality_level; // for each item, the current cumulative quality level gained from workers

};

class IA_Change // Change item
{
  friend bool operator==(const IA_Change& m1, const IA_Change& m2);
  friend bool operator!=(const IA_Change& m1, const IA_Change& m2);
  friend bool operator<(const IA_Change& m1, const IA_Change& m2);
  friend ostream& operator<<(ostream& os, const IA_Change& c);
  friend istream& operator>>(istream& is, IA_Change& c);
 public:
  IA_Change() {old_item = -1; new_item = -1; worker = -1;}
  int old_item, new_item;
  int position;
  int worker;
};

class IA_Swap
{
  friend bool operator==(const IA_Swap& m1, const IA_Swap& m2);
  friend bool operator!=(const IA_Swap& m1, const IA_Swap& m2);
  friend bool operator<(const IA_Swap& m1, const IA_Swap& m2);
  friend ostream& operator<<(ostream& os, const IA_Swap& c);
  friend istream& operator>>(istream& is, IA_Swap& c);
 public:
  IA_Swap() {item1 = -1; item2 = -1; worker1 = -1; worker2 = -1; position1 = -1; position2 = -1;}
  int item1, item2;
  int worker1, worker2;
  int position1, position2;
};
#endif

