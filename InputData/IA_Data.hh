// File IA_Data.hh
#ifndef IA_DATA_HH
#define IA_DATA_HH

#include <iostream>
#include <vector>
#include <exception>
#include <algorithm>
#include <cmath>

#include "json.hpp"

const int HARD_WEIGHT = 1;
const int W1 = 1;
const int W2 = 1;
#define HARD_WEIGHT_SET


class NotImplemented : public std::logic_error 
{
public:
	NotImplemented(const char* file, int line) : std::logic_error("Not Implemented method at " + std::string(file) + ":" + std::to_string(line)) {}
};


using json = nlohmann::json;
using namespace std;

class Feature
{
  friend ostream& operator<<(ostream& os, const Feature& ca);
public:
  Feature(string c_id, int a, int l);
  Feature& operator=(Feature c);
  string Id() const {return id;}
  int Assignments() const {return assignments;}
  int Levels() const {return levels;}
  void SetLevel(int i, string l) {level_vector[i] = l;}
  string GetLevel(int i) const {return level_vector[i];}
protected:
  string id;
  int levels;
  int assignments; // no of items of each level to be assigned to each worker / no of workers of each level to be assigned to each item
  vector<string> level_vector;  
};

class Item
{
  friend ostream& operator<<(ostream& os, const Item& i);
public:
  Item(string i_id, size_t nc) {id = i_id; categories.resize(nc);}
  string Id() const {return id;}
  void SetCategoryLevel(int c, int l) {categories[c] = l;}
  int GetCategoryLevel(int c) const {return categories[c];} //level of category c
protected:
  string id;
  vector<int> categories;
};

class Worker
{
  friend ostream& operator<<(ostream& os, const Worker& w);
public:
  Worker(string w_id, int ex, size_t np) {id = w_id; expertise = ex; properties.resize(np);}
  string Id() const {return id;}
  void SetPropertyLevel(int p, int l) {properties[p] = l;}
  int GetPropertyLevel(int p) const {return properties[p];} //level of property p
  int Expertise() const {return expertise;}
protected:
  string id;
  int expertise;
  vector<int> properties;
};


class IA_Input 
{
  friend ostream& operator<<(ostream& os, const IA_Input& ia);
public:
  IA_Input(json content);

  string Name() const {return name;}
  int ItemCategories() const {return item_categories;}
  int Items() const {return items;}
  int TotalWorkerAssignments() const {return total_worker_assignments;} // no items to assign to each worker
  int MinItemRepetitions() const {return min_item_repetitions;} // min no repetitions for each item
  int MinItemQualityLevel() const {return min_item_quality_level;} // min item quality level as the sum of expertise level of the workers assigned to the item
  int MaxItemCategoryLevels() const {return max_item_category_levels;} // max num of levels for each category
  int WorkerProperties() const { return worker_properties; }
  int Workers() const {return workers;}
  int MaxWorkerPropertyLevels() const {return max_worker_property_levels;} // max num of levels for each property

  const Item& GetItem(int i) const {return item_vector[i];}
  const Feature& GetItemCategory(int c) const {return item_category_vector[c];}

  const Worker& GetWorker(int w) const {return worker_vector[w];}
  const Feature& GetWorkerProperty(int p) const {return worker_property_vector[p];}

  int FindItemId(string id) const;

  int ComputeLowerBound() const { return ceil(items * min_item_repetitions)/total_worker_assignments;}
  
protected:

  int FindItemCategory(string id) const;
  int FindItemCategoryLevel(int c, string id) const;

  int FindWorkerProperty(string id) const;
  int FindWorkerPropertyLevel(int p, string id) const;
  
  string name;

  int item_categories;
  int items;
  int worker_properties;
  int workers; 

  vector<Feature> item_category_vector;
  vector<Item> item_vector;

  vector<Feature> worker_property_vector;
  vector<Worker> worker_vector;

  int total_worker_assignments;
  int min_item_repetitions;
  int min_item_quality_level;
  int max_item_category_levels;
  int max_worker_property_levels;
};

class IA_Output 
{
  friend ostream& operator<<(ostream& os, const IA_Output& out);
  friend istream& operator>>(istream& is, IA_Output& out);
public:
  IA_Output(const IA_Input& i);
  IA_Output& operator=(const IA_Output& out);
  int GetAssignment(unsigned w, unsigned k) const { return assignments[w][k]; }
  void AddAssignment(unsigned w, int i) { return assignments[w].push_back(i);}
  // void SetAssignment(unsigned w, unsigned k, int item) { assignments[w][k] = i;}

  void ReadSolution(string sol_file);
  json ConvertToJSON() const;
  void PrintJsonFormat(ostream& os) const;
  
  bool IsEmployed(unsigned w) const {return !assignments[w].empty();}
  void SetUsedWorkers(int u) {used_workers = u;}
  int UsedWorkers() const {return used_workers;}

  void SortAssignments(); // Sort the assignments[w] vectors

  void Reset();
protected:
  const IA_Input& in;
  int used_workers;
  vector<vector<int>> assignments; // for each worker: list of items, Workers X TotalWorkerAssignments (for used workers, assignments[w] is empty for unused workers)
};
#endif
