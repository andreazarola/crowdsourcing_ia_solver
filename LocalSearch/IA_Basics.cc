// File IA_Basics.cc
#include "IA_Basics.hh"

IA_State::IA_State(const IA_Input &my_in) 
  : in(my_in), assignments(in.Workers(), vector<int>(in.TotalWorkerAssignments(), -1)),
  item_assignments(in.Items()), 
  worker_item_assignments(in.Workers(), vector<bool>(in.Items(), false)), 
  workers_distance_sets(in.Workers(), vector<int> (in.Workers(), 0)), 
    workers_category_levels(in.Workers(), vector<vector<int>>(in.ItemCategories(),vector<int>(in.MaxItemCategoryLevels(),0))),
    items_property_levels(in.Items(), vector<vector<int>>(in.WorkerProperties(),vector<int>(in.MaxWorkerPropertyLevels(),0))), surplus_item_assignments(0), item_quality_level(in.Items(), 0)//, index_surplus_item_assignments(in.Items(), -1)
{}

void IA_State::Reset()
{
  int w, k, i;

  for(w=0; w < in.Workers(); w++)
    {
      fill(assignments[w].begin(), assignments[w].end(),-1);
      fill(worker_item_assignments[w].begin(), worker_item_assignments[w].end(), false);
      fill(workers_distance_sets[w].begin(), workers_distance_sets[w].end(), 0);
      for (k = 0; k < in.ItemCategories(); k++)
	fill(workers_category_levels[w][k].begin(),  workers_category_levels[w][k].end(), 0);
    }

  for(i=0; i < in.Items(); i++)
    {
      for (k = 0; k < in.WorkerProperties(); k++)
	    fill(items_property_levels[i][k].begin(),  items_property_levels[i][k].end(), 0);
      item_assignments[i].clear();
      item_quality_level[i] = 0;
    }
  
  surplus_item_assignments.clear();
}

IA_State& IA_State::operator=(const IA_State& s)
{
  assignments = s.assignments;
  item_assignments = s.item_assignments;
  worker_item_assignments = s.worker_item_assignments;
  workers_distance_sets = s.workers_distance_sets;
  workers_category_levels = s.workers_category_levels;
  items_property_levels = s.items_property_levels;
  surplus_item_assignments = s.surplus_item_assignments;
  // index_surplus_item_assignments = s.index_surplus_item_assignments;
  item_quality_level = s.item_quality_level;
  return *this;
}
  
bool operator==(const IA_State& st1, const IA_State& st2)
{
  vector<vector<int>> assignments_1 = st1.assignments, assignments_2 = st2.assignments; 
  
  // sort assignments and then compare // TODO: not if soft constraints!!!
  SortMatrix(assignments_1);
  SortMatrix(assignments_2);
  
  return assignments_1 == assignments_2;
}

ostream& operator<<(ostream& os, const IA_State& st)
{
  int i, w, w2, k, c, l, p;

  os << "WORKER ASSIGNMENTS:" << endl;
  for(w=0; w<st.in.Workers(); w++)
    {
      os << w << ": ";
      if(!st.assignments[w].empty())
        {
          for(k=0; k<st.in.TotalWorkerAssignments(); k++)
            os << st.assignments[w][k] << " ";
       //   os << st.in.GetItem(st.assignments[w][k]).Id() << " ";
        }
      os << endl;
    }
  os << endl;

   os << "ITEM ASSIGNMENTS:" << endl;
   for(i=0; i<st.in.Items(); i++)
     {
       os << i << "/" << st.in.MinItemRepetitions() << ": ";
       for(k=0; k<st.GetItemAssignments(i); k++)
	 os << st.item_assignments[i][k] << ' ';
					    os << endl;		 	    
     }
	   os << endl;

   os << "WORKER X ITEM ASSIGNMENT:" << endl;
   for(w=0; w<st.in.Workers(); w++)
     {
       os << w << ": ";
       for(i=0; i<st.in.Items(); i++)
         os << st.worker_item_assignments[w][i] << " ";
       os << endl;
     }
   os << endl;

   os << "WORKER X WORKER DISTANCE SET:" << endl;
   for(w=0; w<st.in.Workers(); w++)
     {
       os << w << ": ";
       for(w2=0; w2<st.in.Workers(); w2++)
         os << st.workers_distance_sets[w][w2] << " ";
       os << endl;
     }
   os << endl;

   os << "WORKERS X (NUM_CATEGORIES X CATEGORY_LEVELS):" << endl;
   for(w=0; w<st.in.Workers(); w++)
     {
       os << w << ": ";
       for(c=0; c < st.in.ItemCategories(); c++)
         {
	   os << "| ";
	   for (l = 0; l < st.in.GetItemCategory(c).Levels(); l++) 
	     os << st.workers_category_levels[w][c][l] << " ";             
         }
       os << endl;
     }
   os << endl;

   os << "ITEMS X (NUM_PROPERTIES X PROPERTY_LEVELS):" << endl;
   for(i=0; i<st.in.Items(); i++)
     {
       os << i << ": ";
       for(p=0; p < st.in.WorkerProperties(); p++)
         {
	   os << "| ";
	   for (l = 0; l < st.in.GetWorkerProperty(p).Levels(); l++) 
	     os << st.items_property_levels[i][p][l] << " ";             
         }
       os << endl;
     }
   os << endl;

   os << "SURPLUS ITEM ASSIGNMENTS:" << endl;
   for (i=0; i<st.NumSurplusItemAssignments(); i++)
     os << st.surplus_item_assignments[i] << " ";
    os << endl << endl;

   // os << "INDEX SURPLUS ITEM ASSIGNMENTS:" << endl;
   // for(i=0; i<st.in.Items(); i++)
     // if(st.index_surplus_item_assignments[i] != -1)
       // os << i << ": " << st.index_surplus_item_assignments[i] << ", ";
   // os << endl << endl;

    os << "ITEM QUALITY LEVELS:" <<  endl;
    for (i=0; i<st.in.Items(); i++)
      os << st.item_quality_level[i] << " ";
    os << endl << endl;
  
  return os;
}

void IA_State::SortAssignments()
{
  SortMatrix(assignments);
}

bool IA_State::IsConsistent(bool full_update) const
{
  int i, c, k, w, w2, l, item, p;
  vector<vector<int>> item_assignments_test(in.Items());
  vector<vector<int>> worker_item_assignments_test(in.Workers(), vector<int>(in.Items(), 0));
  vector<vector<int>> workers_distance_sets_test(in.Workers(), vector<int>(in.Workers(), 0));
  vector<vector<vector<int>>> workers_category_levels_test(in.Workers(), vector<vector<int>>(in.ItemCategories(),vector<int>(in.MaxItemCategoryLevels(),0)));
  vector<vector<vector<int>>> items_property_levels_test(in.Items(), vector<vector<int>>(in.WorkerProperties(),vector<int>(in.MaxWorkerPropertyLevels(),0)));
  vector<int> surplus_item_assignments_test(0);
  vector<int> item_quality_level_test(in.Items(), 0);

  for(w=0; w<in.Workers(); w++)
    {
      for(k=0; k<in.TotalWorkerAssignments(); k++)
	{
	  item = assignments[w][k];
	  if(item == -1)
            cerr << "Unassignment for worker " << w << ", position " << k << endl;
	  else
	    {
	      item_assignments_test[item].push_back(w);
	      worker_item_assignments_test[w][item]++;
	      for(c = 0; c < in.ItemCategories(); c++)
		{
		  l = in.GetItem(item).GetCategoryLevel(c);
		  workers_category_levels_test[w][c][l]++;
		}
	      for(p = 0; p < in.WorkerProperties(); p++)
		{
		  l = in.GetWorker(w).GetPropertyLevel(p);
		  items_property_levels_test[item][p][l]++;
		}
	      item_quality_level_test[item] += in.GetWorker(w).Expertise();
	    }
	}
    }

  for(i=0; i<in.Items(); i++)
    if(HasExceedingAssignments(i))
      surplus_item_assignments_test.push_back(i);

  for(i=0; i<in.Items(); i++)
    {
      // if(item_assignments_test[i].size() != item_assignments[i].size())
      // 	{
      // 	  cerr << "Error in item assignments for item " << i << " (size computed: " << item_assignments_test[i].size() 
      // 	       << ", stored " << item_assignments[i].size() << ")" << endl;
      // 	  return false;
      // 	}
      for(k=0; k<static_cast<int>(item_assignments_test[i].size()); k++)
	{
	  if(find(item_assignments[i].begin(), item_assignments[i].end(), item_assignments_test[i][k]) == item_assignments[i].end())
	    cerr << "Error in item assignments for item " << i << " (element computed: " << item_assignments_test[i][k] 
	       << ", not found)" << endl;
	}
      
      if(item_quality_level_test[i] != item_quality_level[i])
	{
	  cerr << "Error in item quality level for item " << i << " (computed: " << item_quality_level_test[i] << ", stored " <<  item_quality_level[i] << ")" << endl;
	  return false;
	}
    }

  for(k=0; k<static_cast<int>(surplus_item_assignments_test.size()); k++)
    if(find(surplus_item_assignments.begin(), surplus_item_assignments.end(), surplus_item_assignments_test[k]) == surplus_item_assignments.end())
      cerr << "Error in surplus item assignments for item element computed: " << surplus_item_assignments_test[k] 
	       << ", not found)" << endl;
  
  for(w=0; w<in.Workers(); w++)
    for(i=0; i<in.Items(); i++)
      {
        if(worker_item_assignments_test[w][i] > 1)
          {
            cerr << "Error for worker " << w << ", item " << i << " has been assigned " << worker_item_assignments_test[w][i] << " times." << endl;
          return false;
          }
        if(worker_item_assignments[w][i] != worker_item_assignments_test[w][i])
                  {
                  cerr << "Error in worker item assignments for worker " << w << " item " << i << " (computed: " << worker_item_assignments_test[w][i] 
               << ", stored " <<  worker_item_assignments[w][i] << ")" << endl;
          return false;
        }
      }

  if(full_update)
    {
      for(w=0; w<in.Workers()-1; w++)
	{
	  for(w2=w+1; w2<in.Workers(); w2++)
	    {
	      for(i=0; i<in.Items(); i++)
		{
		  if (worker_item_assignments[w][i] != worker_item_assignments[w2][i])
		    {
		      workers_distance_sets_test[w][w2]++;
		      workers_distance_sets_test[w2][w]++;
		    }
		}
	      if(workers_distance_sets_test[w][w2] !=  workers_distance_sets[w][w2])
		{
		  cerr << "Error in workers equal sets for worker " << w << " and worker " << w2 << " (computed: " << workers_distance_sets_test[w][w2] 
		       << ", stored " <<  workers_distance_sets[w][w2] << ")" << endl;
		  return false;
		}

	    }
	}
    }

  for(w=0; w<in.Workers(); w++)
    {
      for(c=0; c < in.ItemCategories(); c++)
         {
	   for (l = 0; l < in.GetItemCategory(c).Levels(); l++) 
	     {
               if(workers_category_levels_test[w][c][l] != workers_category_levels[w][c][l])
		 {
		   cerr << "Error in workers category levels for worker " << w << ", category " << c << ", level " << l << " (computed: " << workers_category_levels_test[w][c][l] 
			<< ", stored " <<  workers_category_levels[w][c][l] << ")" << endl;
		   return false;
		 }
	     }
	 }
    }
  
  for(i=0; i<in.Items(); i++)
    {
      for(p = 0; p < in.WorkerProperties(); p++)
	{
	  for (l = 0; l < in.GetWorkerProperty(p).Levels(); l++)
	    {
	      if(items_property_levels_test[i][p][l] != items_property_levels[i][p][l])
		{
		  cerr << "Error in item property levels for item " << i << ", property " << p << ", level " << l << " (computed: " << items_property_levels_test[i][p][l] 
			<< ", stored " <<  items_property_levels[i][p][l] << ")" << endl;
		   return false;
		}
	    }
	}
    }
  return true;
}

void IA_State::SetAssignment(int w, int k, int new_item, bool full_update) 
{
  //   static int i = 0;
  //   i++;
  //   if (i % 1000 == 0) cerr << i << " ";
  int c, w2, old_level, new_level, p;
  int old_item;
  
  old_item = assignments[w][k];
  assignments[w][k] = new_item;
  if(GetItemAssignments(new_item) == in.MinItemRepetitions())
    {
      surplus_item_assignments.push_back(new_item);
    }
  item_assignments[new_item].push_back(w);
  worker_item_assignments[w][new_item] = true;
  item_quality_level[new_item] += in.GetWorker(w).Expertise();

  if(old_item != -1) // not from RandomState
    {
      RemoveElement(item_assignments[old_item],w);
      item_quality_level[old_item] -= in.GetWorker(w).Expertise();
      if(GetItemAssignments(old_item) == in.MinItemRepetitions())
	{
	  RemoveElement(surplus_item_assignments,old_item);
	}
      worker_item_assignments[w][old_item] = false;
      for(c = 0; c < in.ItemCategories(); c++)
	{
	  old_level = in.GetItem(old_item).GetCategoryLevel(c);
	  workers_category_levels[w][c][old_level]--;
	}

      for(p = 0; p < in.WorkerProperties(); p++)
	{
	  old_level = in.GetWorker(w).GetPropertyLevel(p);
	  items_property_levels[old_item][p][old_level]--;
	}
    
      if (full_update)
	{
	  for(w2=0; w2<in.Workers(); w2++)
	    {
	      if(w != w2)
		{
		  if(worker_item_assignments[w2][old_item])
		    {
		      workers_distance_sets[w][w2]++;
		      workers_distance_sets[w2][w]++;
		    }
		  else
		    {
		      workers_distance_sets[w][w2]--;
		      workers_distance_sets[w2][w]--;
		    }
		  if(worker_item_assignments[w2][new_item])
		    {
		      workers_distance_sets[w][w2]--;
		      workers_distance_sets[w2][w]--;
		    }
		  else
		    {
		      workers_distance_sets[w][w2]++;
		      workers_distance_sets[w2][w]++;
		    }
		}
	    }
	}
    }

  for(c = 0; c < in.ItemCategories(); c++)
    {
      new_level = in.GetItem(new_item).GetCategoryLevel(c);
      workers_category_levels[w][c][new_level]++;          
    } 

  for(p = 0; p < in.WorkerProperties(); p++)
    {
      new_level = in.GetWorker(w).GetPropertyLevel(p);
      items_property_levels[new_item][p][new_level]++;
    }
     
  if (full_update)
    {
      if(old_item == -1) // From random state
	{
	  for(w2=0; w2<in.Workers(); w2++)
	    {
	      if(w != w2)
		{
		  if(worker_item_assignments[w2][new_item])
		    {
		      workers_distance_sets[w][w2]--;
		      workers_distance_sets[w2][w]--;
		    }
		  else
		    {
		      workers_distance_sets[w][w2]++;
		      workers_distance_sets[w2][w]++;
		    }
		}
	    }
	}
    }
}


int IA_State::ComputeItemCategoryLevelsForWorker(int i, int w) const
{
  int c, l, count = 0;

  for(c = 0; c < in.ItemCategories(); c++)
  {
    l = in.GetItem(i).GetCategoryLevel(c);
    count += workers_category_levels[w][c][l];	 
  }

  return count;		
}

int IA_State::ComputeWorkerPropertiesLevelsForItem(int i, int w) const
{
  int p, l, count = 0;

  for(p = 0; p < in.WorkerProperties(); p++)
  {
    l = in.GetWorker(w).GetPropertyLevel(p);
    count += items_property_levels[i][p][l];
  }
  return count;		
}

int IA_State::ComputeQualityForItem(int i, int w) const
{
  const int waste_weight = 1, need_weight = 2;
  if(ItemQualityLevel(i) >= in.MinItemQualityLevel())
    return waste_weight * in.GetWorker(w).Expertise(); // the expertize is wasted, and so it is added as penalty
  else if (ItemQualityLevel(i) + in.GetWorker(w).Expertise() < in.MinItemQualityLevel())
    return - need_weight * in.GetWorker(w).Expertise();
  else
    return (ItemQualityLevel(i) - in.MinItemQualityLevel()) * need_weight
      + waste_weight * (ItemQualityLevel(i) + in.GetWorker(w).Expertise() - in.MinItemQualityLevel());
}

int IA_State::ComputeGreedySoftScore(int i, int w) const
{
  const int weight_properties = 5;
  const int weight_expertize = 1;

  return weight_properties * ComputeWorkerPropertiesLevelsForItem(i, w) +
    weight_expertize * ComputeQualityForItem(i, w);
}

int IA_State::ComputeDeltaDistanceSets(int w1, int w2, int i1, int i2) const
{
  int delta_distance = 0, distance, cost = 0;
  
  distance = workers_distance_sets[w1][w2];
  if(distance == 0)
    cost--;
  else if(distance == 2)
    {              
      if(worker_item_assignments[w2][i1])
        delta_distance++;
      else
        delta_distance--;
      
      if(worker_item_assignments[w2][i2])
        delta_distance--;
      else
        delta_distance++;
      if(distance + delta_distance == 0)
        cost++;
    }
  return cost;
}

int IA_State::ComputeDeltaCategoryWorkerAssignments(int w, int i1, int i2, int c) const
{
  int cost = 0, level1, level2;
  
  level1 = in.GetItem(i1).GetCategoryLevel(c);
  level2 = in.GetItem(i2).GetCategoryLevel(c);
  if(level1 != level2)
    {
      if(WorkersCategoryLevels(w, c, level1) > in.GetItemCategory(c).Assignments())
        cost--;
      else
        cost++;        

      if(WorkersCategoryLevels(w, c, level2) >= in.GetItemCategory(c).Assignments())
        cost++;
      else
        cost--;
    }  
  return cost;
}

int IA_State::ComputeDeltaPropertyItemAssignments(int i1, int i2, int p, int level) const
{
  int cost = 0;

  if(ItemPropertyLevels(i1, p, level) <= in.GetWorkerProperty(p).Assignments())
    cost++;        
  
  if(ItemPropertyLevels(i2, p, level) < in.GetWorkerProperty(p).Assignments())
    cost--;

  return cost;
}


int IA_State::ComputeDeltaChangeMinimumItemQualityLevel(int i1, int i2, int w) const
{
  int cost = 0;

    if(ItemQualityLevel(i1) - in.GetWorker(w).Expertise() < in.MinItemQualityLevel())
    cost += min(in.GetWorker(w).Expertise(), in.MinItemQualityLevel() - ItemQualityLevel(i1) + in.GetWorker(w).Expertise());

  if(ItemQualityLevel(i2) < in.MinItemQualityLevel())
    cost -= min(in.GetWorker(w).Expertise(), in.MinItemQualityLevel() - ItemQualityLevel(i2));

  return cost;
}
  

int IA_State::ComputeDeltaSwapMinimumItemQualityLevel(int i1, int i2, int w1, int w2) const
{
  int old_cost = 0, new_cost = 0, new_quality_level;

  if(ItemQualityLevel(i1) < in.MinItemQualityLevel())
      old_cost = in.MinItemQualityLevel() - ItemQualityLevel(i1);

  new_quality_level = ItemQualityLevel(i1) - in.GetWorker(w1).Expertise() + in.GetWorker(w2).Expertise();
  
  if(new_quality_level < in.MinItemQualityLevel())
    new_cost = in.MinItemQualityLevel() - new_quality_level;
    
  return new_cost - old_cost;
}

int IA_State::ComputeNumItemsLowQuality() const
{
  int num_items_low_quality = 0;
  
  for(int i=0; i<in.Items(); i++)
    {
      if(item_quality_level[i] < in.MinItemQualityLevel())
	num_items_low_quality++;
    }
  return num_items_low_quality;
}

double IA_State::ComputeFairnessIndex(int cost) const
{
  int sum_levels = 0;
  for (int p=0; p<in.WorkerProperties(); p++)
    sum_levels += in.GetWorkerProperty(p).Levels();
  return cost/double(in.Items() * sum_levels);
}

bool operator==(const IA_Change& mv1, const IA_Change& mv2)
{
  return mv1.worker == mv2.worker && mv1.position == mv2.position && mv1.new_item == mv2.new_item;
}

bool operator!=(const IA_Change& mv1, const IA_Change& mv2)
{
  return mv1.worker != mv2.worker || mv1.position != mv2.position || mv1.new_item != mv2.new_item;
}

bool operator<(const IA_Change& mv1, const IA_Change& mv2)
{
 return (mv1.worker < mv2.worker)
   || (mv1.worker == mv2.worker && mv1.position < mv2.position)
   || (mv1.worker == mv2.worker && mv1.position == mv2.position && mv1.new_item < mv2.new_item);
}

istream& operator>>(istream& is, IA_Change& c)
{
  throw NotImplemented(__FILE__, __LINE__);        
  return is;
}

ostream& operator<<(ostream& os, const IA_Change& c)
{
  os << c.worker << "@"  << c.position << ": " << c.old_item << "->" << c.new_item;
  return os;
}

bool operator==(const IA_Swap& mv1, const IA_Swap& mv2)
{
  return mv1.worker1 == mv2.worker1 && mv1.worker2 == mv2.worker2 && 
    mv1.item1 == mv2.item1 && mv1.item2 == mv2.item2;
}

bool operator!=(const IA_Swap& mv1, const IA_Swap& mv2)
{
  return mv1.worker1 != mv2.worker1 || mv1.worker2 != mv2.worker2 || 
    mv1.item1 != mv2.item1 || mv1.item2 == mv2.item2;
}

bool operator<(const IA_Swap& mv1, const IA_Swap& mv2)
{
  return mv1.worker1 < mv2.worker1
    || (mv1.worker1 == mv2.worker1 && mv1.worker2 < mv2.worker2)
    || (mv1.worker1 == mv2.worker1 && mv1.worker2 == mv2.worker2 &&  mv1.item1 < mv2.item1)
    || (mv1.worker1 == mv2.worker1 && mv1.worker2 == mv2.worker2 &&  mv1.item1 == mv2.item1 && mv1.item2 < mv2.item2);
}

istream& operator>>(istream& is, IA_Swap& c)
{
  // Insert the code that read a move
        throw NotImplemented(__FILE__, __LINE__);        
  return is;
}

ostream& operator<<(ostream& os, const IA_Swap& mv)
{
  os << mv.worker1 << ":" << mv.item1 << "/" << mv.position1 << " <--> "
     << mv.worker2 << ":" << mv.item2 << "/" << mv.position2;
  return os;
}
