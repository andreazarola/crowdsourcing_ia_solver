// File IA_Helpers.cc
#include "IA_Helpers.hh"

int IA_EqualSets::ComputeCost(const IA_Input & in, const IA_State& st) const
{
  int w1, w2, cost = 0;//, i;
  
  for(w1=0; w1<in.Workers()-1; w1++)
  {
    for(w2=w1+1; w2<in.Workers(); w2++)
    {
      // i = 0;
      // while(i < in.Items() && st.WorkerItemAssignment(w1, i) == st.WorkerItemAssignment(w2, i))
      //  i++;
      // if(i == in.Items())
      //   cost++;
      
      if(st.WorkersDistanceSets(w1, w2) == 0)
        cost++;
    }
  }
  return cost;
}

void IA_EqualSets::PrintViolations(const IA_Input & in, const IA_State& st, ostream& os) const
{
  int w1, w2, i;
  
  os << "EQUAL WORKER SETS:" << endl;
  
  for(w1=0; w1<in.Workers()-1; w1++)
  {
    for(w2=w1+1; w2<in.Workers(); w2++)
    {
      i = 0;
      while(i < in.Items() && st.WorkerItemAssignment(w1, i) == st.WorkerItemAssignment(w2, i))
        i++;   
      if(i == in.Items())
        os << "Worker " << w1 << " and " << w2 << " have the same set." << endl;
    }
  }
}

int IA_CategoryWorkerAssignments::ComputeCost(const IA_Input & in, const IA_State& st) const
{ 
  int cost = 0;
  int w, c, l;
  
  for(w=0; w<in.Workers(); w++)
  {
    for(c=0; c<in.ItemCategories(); c++)
    {
      for(l=0; l<in.GetItemCategory(c).Levels(); l++)
      {                
        if(st.WorkersCategoryLevels(w, c, l) != in.GetItemCategory(c).Assignments())
          cost += abs(st.WorkersCategoryLevels(w, c, l) - in.GetItemCategory(c).Assignments());
      }
    }
  }
  return cost;
}

void IA_CategoryWorkerAssignments::PrintViolations(const IA_Input& in, const IA_State& st, ostream& os) const
{
  int w, c, l;
  os << "CATEGORY WORKER ASSIGNMENTS:" << endl;
  
  for(w=0; w<in.Workers(); w++)
  {
    for(c=0; c<in.ItemCategories(); c++)
    {
      for(l=0; l<in.GetItemCategory(c).Levels(); l++)
      {                
        if(st.WorkersCategoryLevels(w, c, l) != in.GetItemCategory(c).Assignments())
          os << "Worker " << w << ", category " << c << ", level " << l
          << ", worker assignments " <<  in.GetItemCategory(c).Assignments()
          << ", actual " << st.WorkersCategoryLevels(w, c, l)
          << ", cost " << abs(st.WorkersCategoryLevels(w, c, l) - in.GetItemCategory(c).Assignments()) << endl;
      }
    }
  }  
}

int IA_PropertyItemAssignments::ComputeCost(const IA_Input& in, const IA_State& st) const
{ 
  int cost = 0;
  int i, p, l;
  
  for(i=0; i<in.Items(); i++)
  {
    for(p=0; p<in.WorkerProperties(); p++)
    {
      for(l=0; l<in.GetWorkerProperty(p).Levels(); l++)
      {                
        if(st.ItemPropertyLevels(i, p, l) < in.GetWorkerProperty(p).Assignments())
          cost += in.GetWorkerProperty(p).Assignments() - st.ItemPropertyLevels(i, p, l);
      }
    }
  }
  return cost;
}

void IA_PropertyItemAssignments::PrintViolations(const IA_Input& in,const IA_State& st, ostream& os) const
{
  int i, p, l;
  os << "PROPERTY ITEM ASSIGNMENTS:" << endl;
  
  for(i=0; i<in.Items(); i++)
  {
    for(p=0; p<in.WorkerProperties(); p++)
    {
      for(l=0; l<in.GetWorkerProperty(p).Levels(); l++)
      {                
        if(st.ItemPropertyLevels(i, p, l) < in.GetWorkerProperty(p).Assignments())
          os << "Item " << i << ", property " << p << ", level " << l
          << ", item assignments " <<  in.GetWorkerProperty(p).Assignments()
          << ", actual " << st.ItemPropertyLevels(i, p, l)
          << ", cost " << in.GetWorkerProperty(p).Assignments() - st.ItemPropertyLevels(i, p, l) << endl;
      }
    }
  } 
}

int IA_MinimumItemQualityLevel::ComputeCost(const IA_Input& in,const IA_State& st) const
{
  int cost = 0, quality_level;
  int i, w; 
  
  for(i=0; i<in.Items(); i++)
  {
    quality_level = 0;
    for(w=0; w<st.GetItemAssignments(i); w++)
    quality_level += in.GetWorker(st.GetItemAssignment(i, w)).Expertise();
    if(in.MinItemQualityLevel() > quality_level)
      cost += in.MinItemQualityLevel() - quality_level;
  }
  return cost;
}

void IA_MinimumItemQualityLevel::PrintViolations(const IA_Input& in,const IA_State& st, ostream& os) const
{
  int quality_level;
  int i, w; 
  
  for(i=0; i<in.Items(); i++)
  {
    quality_level = 0;
    for(w=0; w<st.GetItemAssignments(i); w++)
    quality_level += in.GetWorker(st.GetItemAssignment(i, w)).Expertise();
    if(in.MinItemQualityLevel() > quality_level)
      os << "Item " << i << ", minimum quality level " <<  in.MinItemQualityLevel()
      << ", actual " << quality_level << ", cost " << in.MinItemQualityLevel() - quality_level << endl;
  }
}

// constructor
IA_StateManager::IA_StateManager()
: StateManager<IA_Input,IA_State>("IAStateManager")  { full_update = false; }

void IA_StateManager::RandomState(const IA_Input& in, IA_State& st)
{
  int count_greedy_trials = 0;
  auto start = std::chrono::high_resolution_clock::now();
  do
  {
    GreedyState(in, st);
    auto cost = CostFunctionComponents(in, st);
    initial_violations = cost.violations;
    initial_objective = cost.objective;
    count_greedy_trials++;
  }
  while (initial_violations != 0 && count_greedy_trials < 10);
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
  initial_time = duration.count() / 1000.0;
  return;
  
  int counter = 0;
  bool feasible;
  
  do
  {
    feasible = CreateRandomMinimalState(in, st);
    counter++;
  }
  while (counter < 1000 && !feasible);
  
  if(counter >= 1000)
    throw runtime_error("Impossible to generate random feasible state!");
}

bool IA_StateManager::CreateRandomMinimalState(const IA_Input& in, IA_State& st) const
{
  int i, w, k, index, j;
  int equal_best_number = 0, best_w = 0, best_feature_overlap, current_feature_overlap;  // best_w and equal_best_number assigned to 0 only to prevent the warning
  vector<int> available_items;  
  vector<int> num_workers_assignments(in.Workers(), 0);
  
  st.Reset();
  
  for (j = 0; j < in.Workers() * in.TotalWorkerAssignments(); j++)
  {
    // recreate the set of items to assign
    if (available_items.size() == 0)
    {
      available_items.resize(in.Items());
      iota(available_items.begin(), available_items.end(),0);
    }
    // select a random item (among those not already selected for that replicate)
    index = 0; //Random::Uniform<int>(0,available_items.size()-1);
    i = available_items[index];
    available_items.erase(available_items.begin() + index);
    // select the worker
    best_feature_overlap = -1;
    best_w = -1;
    for (w = 0; w < in.Workers(); w++)
    {  // if the worker is available for item i
      if (!st.WorkerItemAssignment(w,i) && num_workers_assignments[w] < in.TotalWorkerAssignments())
      {
        current_feature_overlap = st.ComputeItemCategoryLevelsForWorker(i,w);
        if (best_feature_overlap == -1 || current_feature_overlap < best_feature_overlap) 
        {
          best_w = w;
          best_feature_overlap = current_feature_overlap;
          equal_best_number = 1;
        }
        // secondary criterion: having more assignments
        else if (current_feature_overlap == best_feature_overlap && 
                 num_workers_assignments[w] > num_workers_assignments[best_w])
        {
          best_w = w;
          equal_best_number = 1;
        }
        else if (current_feature_overlap == best_feature_overlap &&
                 num_workers_assignments[w] == num_workers_assignments[best_w])
        {
          equal_best_number++;
          if (Random::Uniform<int>(1,equal_best_number) == 1) 
            best_w = w;
        }
      }
    }
    
    //       cerr << "Item " << i << ", worker " << best_w << ", feature_overlap " << best_feature_overlap << endl;
    if(best_w == -1)
      return false;	
    do
    {  // select a random position for the item i the schedule of the worker (at present the position is irrelevant)
      k = Random::Uniform<int>(0, in.TotalWorkerAssignments()-1);
    }
    while (st.GetAssignment(best_w,k) != -1);
    
    // cerr << "Assign item " << i << " to worker " << best_w << " in position " << k << endl; 
    st.SetAssignment(best_w, k, i, full_update);
    num_workers_assignments[best_w]++;
  }
  return true;
}

void IA_StateManager::GreedyState(const IA_Input& in, IA_State& st)
{  
  st.Reset();
  if (in.ItemCategories() == 1)
    SingleCategoryGreedyState(in, st);
  else if (in.ItemCategories() == 2)
    TwoCategoriesGreedyState(in, st);
  else if (in.ItemCategories() == 3)
    ThreeCategoriesGreedyState(in, st);
  else
    throw invalid_argument("Not implemented yet!");
}

void IA_StateManager::SingleCategoryGreedyState(const IA_Input& in, IA_State& st) const
{
  int w, k, i, j, item, val, best_i = 0, best_item = 0, best_val, equal_best_number = 0, v0;
  
  // for each tuple (singleton) of values of the categories, store the vector of items available with the given values (along with the multiplicy, henche a pair)
  vector<vector<pair<int,int>>> available_items(in.GetItemCategory(0).Levels());
  vector<int> current_items(in.GetItemCategory(0).Levels(),0);
  
  // the number of (available) repetitions of each item depends un the number of workers (not necessarily the minimum)
  int num_repetitions = ceil(static_cast<double>(in.Workers() * in.TotalWorkerAssignments()) / in.Items());  // in.MinItemRepetitions()
  int num_available_items = num_repetitions * in.Items();
  
  // initialize the data structure available_items, based on the number of levels of the 2 categories:
  // for each item, insert it in the proper cell based on its values (GetCategoryLevel)
  for (i = 0; i < in.Items(); i++)
  available_items[in.GetItem(i).GetCategoryLevel(0)].push_back(make_pair(i,num_repetitions));
  
  // p0 and p1 are the vectors of the values to assign to each worker: p0 is fixed to <0,1,..., n-1>, p1 is a changed by next_permutation
  vector<int> p0(in.TotalWorkerAssignments());
  
  // creates the vector <0,1,2,3,4,5>, <0,1,2,0,1,2>, <0,1,0,1,0,1> or ... depending on the levels of the category and the HIT size
  InitialPermutation(p0,in.GetItemCategory(0).Levels());
  
  // generate the assignment to each worker
  for(w = 0; w < in.Workers(); w++)
  {
    for (k = 0; k < in.TotalWorkerAssignments(); k++)
    {
      v0 = p0[k];
      // i is the index of the selected item in available_items[v0][v1] (item is the item itself)
      // i = Random::Uniform<int>(0, available_items[v0][v1].size()-1);
      j = current_items[v0];
      best_val = -1;
      for (i = j; static_cast<unsigned>(i) < available_items[v0].size(); i++)
      {
        item = available_items[v0][i].first;
        if (!st.WorkerItemAssignment(w, item))
        {
          val = st.ComputeGreedySoftScore(item, w);
          if (best_val == -1 || val < best_val)
          {
            best_i = i;
            best_item = item;
            best_val = val;
            equal_best_number = 1;
          }
          else if (val == best_val)
          {
            equal_best_number++;
            if (Random::Uniform<int>(1,equal_best_number) == 1) 
            {
              best_i = i;
              best_item = item;
            }
          }
        }
      }
      
      // remove the item from the structure
      num_available_items--;
      available_items[v0][best_i].second--;
      if (available_items[v0][best_i].second == 0)
        available_items[v0].erase(available_items[v0].begin() + best_i);
      else
      {
        swap(available_items[v0][best_i], available_items[v0][j]);
        current_items[v0]++;
        if (static_cast<unsigned>(current_items[v0]) >= available_items[v0].size())
          current_items[v0] = 0;
      }
      st.SetAssignment(w, k, best_item, full_update);
      //          cerr << "Assign item " << best_item << " to worker " << w << endl;
    }
  }
}

void IA_StateManager::TwoCategoriesGreedyState(const IA_Input& in, IA_State& st) const
{
  int w, k, i, j, item, val, best_i = 0, best_item = 0, best_val, equal_best_number = 0, v0, v1;
  
  // for each tuple (pair) of values of the categories, store the vector of items available with the given values (along with the multiplicy, henche a pair)
  vector<vector<vector<pair<int,int>>>> available_items(in.GetItemCategory(0).Levels(), 
                                                        vector<vector<pair<int,int>>>(in.GetItemCategory(1).Levels()));
  vector<vector<int>> current_items(in.GetItemCategory(0).Levels(),
                                    vector<int>(in.GetItemCategory(1).Levels(),0));
  
  // the number of (available) repetitions of each item depends un the number of workers (not necessarily the minimum)
  int num_repetitions = ceil(static_cast<double>(in.Workers() * in.TotalWorkerAssignments()) / in.Items());  // in.MinItemRepetitions()
  int num_available_items = num_repetitions * in.Items();
  
  // initialize the data structure available_items, based on the number of levels of the 2 categories:
  // for each item, insert it in the proper cell based on its values (GetCategoryLevel)
  for (i = 0; i < in.Items(); i++)
  available_items[in.GetItem(i).GetCategoryLevel(0)][in.GetItem(i).GetCategoryLevel(1)].push_back(make_pair(i,num_repetitions));
  
  // p0 and p1 are the vectors of the values to assign to each worker: p0 is fixed to <0,1,..., n-1>, p1 is a changed by next_permutation
  vector<int> p0(in.TotalWorkerAssignments()), p1(in.TotalWorkerAssignments());
  
  // creates the vector <0,1,2,3,4,5>, <0,1,2,0,1,2>, <0,1,0,1,0,1> or ... depending on the levels of the category and the HIT size
  InitialPermutation(p0,in.GetItemCategory(0).Levels());
  InitialPermutation(p1,in.GetItemCategory(1).Levels());
  
  // generate the assignment to each worker
  for(w = 0; w < in.Workers(); w++)
  {
    for (k = 0; k < in.TotalWorkerAssignments(); k++)
    {
      v0 = p0[k];
      v1 = p1[k];
      // i is the index of the selected item in available_items[v0][v1] (item is the item itself)
      
      // TODO: instead of a random item, assign the one the maximize the fairness 
      // (minimum repetition of property values, and maximum assignement as secondary criterion)
      // i = Random::Uniform<int>(0, available_items[v0][v1].size()-1);
      j = current_items[v0][v1];
      best_val = -1;
      for (i = j; static_cast<unsigned>(i) < available_items[v0][v1].size(); i++)
      {
        item = available_items[v0][v1][i].first;
        if (!st.WorkerItemAssignment(w, item))
        {
          val = st.ComputeGreedySoftScore(item, w);
          if (best_val == -1 || val < best_val)
          {
            best_i = i;
            best_item = item;
            best_val = val;
            equal_best_number = 1;
          }
          else if (val == best_val)
          {
            equal_best_number++;
            if (Random::Uniform<int>(1,equal_best_number) == 1) 
            {
              best_i = i;
              best_item = item;
            }
          }
        }
      }
      
      // remove the item from the structure
      num_available_items--;
      available_items[v0][v1][best_i].second--;
      if (available_items[v0][v1][best_i].second == 0)
        available_items[v0][v1].erase(available_items[v0][v1].begin() + best_i);
      else
      {
        swap(available_items[v0][v1][best_i], available_items[v0][v1][j]);
        current_items[v0][v1]++;
        if (static_cast<unsigned>(current_items[v0][v1]) >= available_items[v0][v1].size())
          current_items[v0][v1] = 0;
      }
      st.SetAssignment(w, k, best_item, full_update);
      //          cerr << "Assign item " << best_item << " to worker " << w << endl;
    }
    
    if (w < in.Workers() - 1) // if not the last worker, generate the next permutation (for the last worker loops forever)
      do 
      {
        if (!next_permutation(p1.begin(), p1.end())) 
          // go back the the first one
          FirstPermutation(p1,in.GetItemCategory(1).Levels());
        //             if (UnavailableItems(p0,p1,available_items))
        //               {
        //                 cerr << "Skipped permutation: ";
        //                 for (unsigned h = 0; h < p1.size(); h++)
        //                   cerr << p0[h] << "/" << p1[h] << " ";
        //                 cerr << endl;
        //               }
      }
    while (UnavailableItems(p0,p1,available_items));
    
    //       // print the permutation in p1
    //       for (unsigned h = 0; h < p1.size(); h++)
    //         cerr << p0[h] << "/" << p1[h] << " ";
    //       cerr << endl;
    //       // print the status of the structure available_items
    //       for (unsigned i = 0; i < available_items.size(); i++)
    //         for (unsigned j = 0; j < available_items[i].size(); j++)
    //           {
    //             cerr << i << " " << j << ": "; // << endl;
    //             for (unsigned k = 0; k < available_items[i][j].size(); k++)
    //               cerr << "(" << available_items[i][j][k].first << ", " << available_items[i][j][k].second << ") "; 
    //             cerr << endl;
    //           }
    //       cerr << endl;
  }
}

void IA_StateManager::ThreeCategoriesGreedyState(const IA_Input& in, IA_State& st) const
{
  int w, k, i, j, item, val, best_i = 0, best_item = 0, best_val, equal_best_number = 0, v0, v1, v2;
  
  // for each tuple (triple) of values of the categories, store the vector of items available with the given values (along with the multiplicy, henche a pair)
  vector<vector<vector<vector<pair<int,int>>>>> available_items(in.GetItemCategory(0).Levels(),
                                                                vector<vector<vector<pair<int,int>>>>(in.GetItemCategory(1).Levels(),
                                                                                                      vector<vector<pair<int,int>>>(in.GetItemCategory(2).Levels())));
  vector<vector<vector<int>>> current_item(in.GetItemCategory(0).Levels(),
                                           vector<vector<int>>(in.GetItemCategory(1).Levels(),
                                                               vector<int>(in.GetItemCategory(2).Levels(),0)));
  
  // the number of (available) repetitions of each item depends un the number of workers (not necessarily the minimum)
  int num_repetitions = ceil(static_cast<double>(in.Workers() * in.TotalWorkerAssignments()) / in.Items());  // in.MinItemRepetitions()
  int num_available_items = num_repetitions * in.Items();
  
  // initialize the data structure available_items, based on the number of levels of the 2 categories:
  // for each item, insert it in the proper cell based on its values (GetCategoryLevel)
  for (i = 0; i < in.Items(); i++)
  available_items[in.GetItem(i).GetCategoryLevel(0)][in.GetItem(i).GetCategoryLevel(1)][in.GetItem(i).GetCategoryLevel(2)].push_back(make_pair(i,num_repetitions));
  
  // p0 and p1 are the vectors of the values to assign to each worker: p0 is fixed to <0,1,..., n-1>, p1 is a changed by next_permutation
  vector<int> p0(in.TotalWorkerAssignments()), p1(in.TotalWorkerAssignments()), p2(in.TotalWorkerAssignments());
  
  // creates the vector <0,1,2,3,4,5>, <0,1,2,0,1,2>, <0,1,0,1,0,1> or ... depending on the levels of the category and the HIT size
  InitialPermutation(p0,in.GetItemCategory(0).Levels());
  InitialPermutation(p1,in.GetItemCategory(1).Levels());
  InitialPermutation(p2,in.GetItemCategory(2).Levels());
  
  // generate the assignment to each worker
  for(w = 0; w < in.Workers(); w++)
  {
    for (k = 0; k < in.TotalWorkerAssignments(); k++)
    {
      v0 = p0[k];
      v1 = p1[k];
      v2 = p2[k];
      
      // i is the index of the selected item in available_items[v0][v1][v2] (item is the item itself)
      // TODO: instead of a random item, assign the one the maximize the fairness 
      // (minimum repetition of property values, and maximum assignement as secondary criterion)
      
      j = current_item[v0][v1][v2];
      best_val = -1;
      for (i = j; static_cast<unsigned>(i) < available_items[v0][v1][v2].size(); i++)
      {
        item = available_items[v0][v1][v2][i].first;
        if (!st.WorkerItemAssignment(w, item))
        {
          val = st.ComputeGreedySoftScore(item, w);
          if (best_val == -1 || val < best_val)
          {
            best_i = i;
            best_item = item;
            best_val = val;
            equal_best_number = 1;
          }
          else if (val == best_val)
          {
            equal_best_number++;
            if (Random::Uniform<int>(1,equal_best_number) == 1) 
            {
              best_i = i;
              best_item = item;
            }
          }
        }
      }
      
      // remove the item best_item from the structure
      num_available_items--;
      available_items[v0][v1][v2][best_i].second--;
      if (available_items[v0][v1][v2][best_i].second == 0)
        available_items[v0][v1][v2].erase(available_items[v0][v1][v2].begin() + best_i);
      else
      {
        swap(available_items[v0][v1][v2][best_i], available_items[v0][v1][v2][j]);
        current_item[v0][v1][v2]++;
        if (static_cast<unsigned>(current_item[v0][v1][v2]) >= available_items[v0][v1][v2].size())
          current_item[v0][v1][v2] = 0;
      }
      st.SetAssignment(w, k, best_item, full_update);
      //          cerr << "Assign item " << best_item << " to worker " << w << endl;
    }
    
    if (w < in.Workers() - 1) // if not the last worker, generate the next permutation (for the last worker it might loops forever)
    {
      do 
      {
        if (!next_permutation(p2.begin(), p2.end()))
        { 
          // go back the the first one
          FirstPermutation(p2,in.GetItemCategory(2).Levels());
          if (!next_permutation(p1.begin(), p1.end()))
            FirstPermutation(p1,in.GetItemCategory(1).Levels());
        }
        //               if (UnavailableItems(p0,p1,p2,available_items))
        //                 {
        //                   cerr << "Skipped permutation: ";
        //                   for (unsigned h = 0; h < p0.size(); h++)
        //                     cerr << p0[h] << "/" << p1[h] << "/" << p2[h] << " ";
        //                   cerr << endl;
        //                 }
      }
      while (UnavailableItems(p0,p1,p2,available_items));
    }
    // print the permutation in p0/p1/p2
    //       for (unsigned h = 0; h < p0.size(); h++)
    //         cerr << p0[h] << "/" << p1[h] << "/" << p2[h] << " ";
    //       cerr << endl;
    //       // print the status of the structure available_items
    //       for (unsigned i = 0; i < available_items.size(); i++)
    //         for (unsigned j = 0; j < available_items[i].size(); j++)
    //           for (unsigned h = 0; h < available_items[i][j].size(); h++)
    //             {
    //               cerr << i << " " << j << " " << h << ": "; // << endl;
    //               for (unsigned k = 0; k < available_items[i][j][h].size(); k++)
    //                 cerr << "(" << available_items[i][j][h][k].first << ", " << available_items[i][j][h][k].second << ") "; 
    //               cerr << endl;
    //             }
    //       cerr << endl;
  }
}


bool IA_StateManager::UnavailableItems(vector<int>& p0, vector<int>& p1, vector<vector<vector<pair<int,int>>>> available_items) const
{
  int v0, v1;
  for (unsigned i = 0; i < p0.size(); i++)
  {
    v0 = p0[i];
    v1 = p1[i];
    if (available_items[v0][v1].size() == 0)
      return true;
    else  // remove an item (anyone, just for the count)
    {
      available_items[v0][v1][0].second--;
      if (available_items[v0][v1][0].second == 0)
        available_items[v0][v1].erase(available_items[v0][v1].begin());
    }
  }
  return false;
} 

bool IA_StateManager::UnavailableItems(vector<int>& p0, vector<int>& p1, vector<int>& p2, vector<vector<vector<vector<pair<int,int>>>>> available_items) const
{ // available items is passed by value, so that it can be dirtied harmlessly
  int v0, v1, v2;
  for (unsigned i = 0; i < p0.size(); i++)
  {
    v0 = p0[i];
    v1 = p1[i];
    v2 = p2[i];
    if (available_items[v0][v1][v2].size() == 0)
      return true;
    else          // remove an item (anyone, just for the count)
    {
      available_items[v0][v1][v2][0].second--;
      if (available_items[v0][v1][v2][0].second == 0)
        available_items[v0][v1][v2].erase(available_items[v0][v1][v2].begin());
    }
  }
  return false;
} 

bool IA_StateManager::CheckConsistency(const IA_Input& in, const IA_State& st) const
{
  return st.IsConsistent(full_update);
}

/*****************************************************************************
 * Output Manager Methods
 *****************************************************************************/

void IA_OutputManager::InputState(const IA_Input& in, IA_State& st, const IA_Output& out) const
{
  int w, k;
  st.Reset();
  
  for(w=0; w<in.Workers(); w++)
  {
    if(out.IsEmployed(w))
    {
      for(k=0; k<in.TotalWorkerAssignments(); k++)
      {
        st.SetAssignment(w, k, out.GetAssignment(w, k), true);
      }
    }
  }
}

void IA_OutputManager::OutputState(const IA_Input& in, const IA_State& st, IA_Output& out) const
{
  int w, k;
  out.Reset();
  
  for(w=0; w<in.Workers(); w++)
  {
    for(k=0; k<in.TotalWorkerAssignments(); k++)
    if(st.GetAssignment(w, k) >= 0)
      out.AddAssignment(w, st.GetAssignment(w, k));
  }
  out.SetUsedWorkers(in.Workers());
  //TODO: remove unemployed workers?    
}

json IA_OutputManager::ConvertToJSON(const IA_Input& in, const IA_State& st) const
{
  IA_Output out(in);
  OutputState(in, st, out);
  return out.ConvertToJSON();
}


/*****************************************************************************
 * IA_Change Neighborhood Explorer Methods
 *****************************************************************************/

// initial move builder
void IA_ChangeNeighborhoodExplorer::RandomMove(const IA_Input& in, const IA_State& st, IA_Change& mv) const
{
  int c = 0;
  do
  {
    AnyRandomMove(in,st,mv);
    c++;
    if (c % 1000 == 0) throw EmptyNeighborhood(); 
  }
  while (!FeasibleMove(in,st,mv));
  //   cerr << c << " ";
}

void IA_ChangeNeighborhoodExplorer::AnyRandomMove(const IA_Input& in, const IA_State& st, IA_Change& mv) const
{
  if (st.NumSurplusItemAssignments() == 0)
    throw EmptyNeighborhood();
  int index;
  
  index = Random::Uniform<int>(0, st.NumSurplusItemAssignments()-1);
  mv.old_item = st.GetSurplusItemAssignments(index);
  index = Random::Uniform<int>(0, st.GetItemAssignments(mv.old_item) - 1);
  mv.worker = st.GetItemAssignment(mv.old_item, index);
  for (int i = 0; i < in.TotalWorkerAssignments(); i++)
  {
    if (st.GetAssignment(mv.worker,i) == mv.old_item)
    {
      mv.position = i;
      break;
    }
  }
  // mv.worker = Random::Uniform<int>(0, in.Workers() - 1);
  // mv.position = Random::Uniform<int>(0, in.TotalWorkerAssignments() - 1);
  // mv.old_item = st.GetAssignment(mv.worker, mv.position);
  
  // random new_item different from old_item
  mv.new_item = Random::Uniform<int>(0, in.Items()-2);
  if (mv.new_item >= mv.old_item)
    mv.new_item++;
} 

// check move feasibility
bool IA_ChangeNeighborhoodExplorer::FeasibleMove(const IA_Input& in, const IA_State& st, const IA_Change& mv) const
{
  return // mv.new_item != mv.old_item &&
  !st.WorkerItemAssignment(mv.worker, mv.new_item)
  && st.HasExceedingAssignments(mv.old_item);
} 

// update the state according to the move 
void IA_ChangeNeighborhoodExplorer::MakeMove(const IA_Input& in, IA_State& st, const IA_Change& mv) const
{
  //cerr << "*";
  st.SetAssignment(mv.worker, mv.position, mv.new_item, static_cast<IA_StateManager&>(sm).FullUpdate());
}  

void IA_ChangeNeighborhoodExplorer::AnyFirstMove(const IA_Input& in, const IA_State& st, IA_Change& mv) const
{
  mv.worker = 0;
  mv.position = 0;
  mv.old_item = st.GetAssignment(mv.worker, mv.position);
  if (mv.old_item > 0)
    mv.new_item = 0;
  else
    mv.new_item = 1;
}

void IA_ChangeNeighborhoodExplorer::FirstMove(const IA_Input& in, const IA_State& st, IA_Change& mv) const
{
  AnyFirstMove(in,st,mv);
  while (!FeasibleMove(in,st,mv))
  {
    if (!AnyNextMove(in,st,mv))
      throw EmptyNeighborhood();
  }
}

bool IA_ChangeNeighborhoodExplorer::AnyNextMove(const IA_Input& in, const IA_State& st, IA_Change& mv) const
{
  if ((mv.new_item < in.Items() - 1 &&  mv.old_item != in.Items() - 1)
      || mv.new_item < in.Items() - 2) 
  {
    if (mv.new_item != mv.old_item - 1)
      mv.new_item++;
    else
      mv.new_item += 2;
    return true;
  }
  else if (mv.position < in.TotalWorkerAssignments() - 1)
  {
    mv.position++;
    mv.old_item = st.GetAssignment(mv.worker, mv.position);
    if (mv.old_item > 0)
      mv.new_item = 0;
    else
      mv.new_item = 1;
    return true;
  }
  else if (mv.worker < in.Workers() - 1)
  {
    mv.worker++;
    mv.position = 0;
    mv.old_item = st.GetAssignment(mv.worker, mv.position);
    if (mv.old_item > 0)
      mv.new_item = 0;
    else
      mv.new_item = 1;
    return true;
  }
  else
    return false;
}

bool IA_ChangeNeighborhoodExplorer::NextMove(const IA_Input& in, const IA_State& st, IA_Change& mv) const
{
  do
    if (!AnyNextMove(in,st,mv))
      return false;
  while (!FeasibleMove(in,st,mv));
  return true;
}

int IA_ChangeDeltaEqualSets::ComputeDeltaCost(const IA_Input& in, const IA_State& st, const IA_Change& mv) const
{
  int cost = 0, w2;
  
  for(w2=0; w2<in.Workers(); w2++)
  {
    if(mv.worker != w2)
      cost += st.ComputeDeltaDistanceSets(mv.worker, w2, mv.old_item, mv.new_item);
  } 
  return cost;
}

int IA_ChangeDeltaCategoryWorkerAssignments::ComputeDeltaCost(const IA_Input& in, const IA_State& st, const IA_Change& mv) const
{
  int c, cost = 0;
  
  for(c = 0; c < in.ItemCategories(); c++)
  cost += st.ComputeDeltaCategoryWorkerAssignments(mv.worker, mv.old_item, mv.new_item, c);
  return cost;
}

int IA_ChangeDeltaPropertyItemAssignments::ComputeDeltaCost(const IA_Input& in, const IA_State& st, const IA_Change& mv) const
{
  int p, cost = 0;
  
  for(p = 0; p < in.WorkerProperties(); p++)
  cost += st.ComputeDeltaPropertyItemAssignments(mv.old_item, mv.new_item, p, in.GetWorker(mv.worker).GetPropertyLevel(p));
  return cost;
}

int IA_ChangeDeltaMinimumItemQualityLevel::ComputeDeltaCost(const IA_Input& in, const IA_State& st, const IA_Change& mv) const
{
  // int cost = 0;
  
  // if(st.ItemQualityLevel(mv.old_item) - in.GetWorker(mv.worker).Expertise() < in.MinItemQualityLevel())
  //   cost += min(in.GetWorker(mv.worker).Expertise(), in.MinItemQualityLevel() - st.ItemQualityLevel(mv.old_item) + in.GetWorker(mv.worker).Expertise());
  
  // if(st.ItemQualityLevel(mv.new_item) < in.MinItemQualityLevel())
  //   cost -= min(in.GetWorker(mv.worker).Expertise(), in.MinItemQualityLevel() - st.ItemQualityLevel(mv.new_item));
  
  // return cost;
  
  return st.ComputeDeltaChangeMinimumItemQualityLevel(mv.old_item, mv.new_item, mv.worker);
}


/*****************************************************************************
 * IA_Swap Neighborhood Explorer Methods
 *****************************************************************************/

// initial move builder
void IA_SwapNeighborhoodExplorer::RandomMove(const IA_Input& in, const IA_State& st, IA_Swap& mv) const
{
  do
  { 
    mv.worker1 = Random::Uniform<int>(0, in.Workers() - 1);
    mv.worker2 = Random::Uniform<int>(0, in.Workers() - 1);
    mv.position1 = Random::Uniform<int>(0, in.TotalWorkerAssignments() - 1);
    mv.position2 = Random::Uniform<int>(0, in.TotalWorkerAssignments() - 1);
    mv.item1 = st.GetAssignment(mv.worker1, mv.position1);
    mv.item2 = st.GetAssignment(mv.worker2, mv.position2);
    //       cerr << mv << endl;
  }
  while (mv.item1 == mv.item2 
         || mv.worker1 == mv.worker2 
         || st.WorkerItemAssignment(mv.worker1, mv.item2) || st.WorkerItemAssignment(mv.worker2, mv.item1)
         );
  if (mv.worker2 < mv.worker1)
  {
    swap(mv.worker1,mv.worker2);
    swap(mv.position1,mv.position2);
    swap(mv.item1,mv.item2);
  }
} 

// check move feasibility
bool IA_SwapNeighborhoodExplorer::FeasibleMove(const IA_Input& in, const IA_State& st, const IA_Swap& mv) const
{
  return mv.item1 != mv.item2
  && mv.worker1 < mv.worker2 
  && !st.WorkerItemAssignment(mv.worker1, mv.item2) && !st.WorkerItemAssignment(mv.worker2, mv.item1);
} 

// update the state according to the move 
void IA_SwapNeighborhoodExplorer::MakeMove(const IA_Input& in, IA_State& st, const IA_Swap& mv) const
{
  //cerr << "-";
  st.SetAssignment(mv.worker1, mv.position1, mv.item2, static_cast<IA_StateManager&>(sm).FullUpdate());
  st.SetAssignment(mv.worker2, mv.position2, mv.item1, static_cast<IA_StateManager&>(sm).FullUpdate()); 
}  

void IA_SwapNeighborhoodExplorer::FirstMove(const IA_Input& in, const IA_State& st, IA_Swap& mv) const
{
  mv.worker1 = 0;
  mv.worker2 = 1;
  mv.position1 = 0;
  mv.position2 = 0;
  mv.item1 = st.GetAssignment(mv.worker1, mv.position1);
  mv.item2 = st.GetAssignment(mv.worker2, mv.position2);
  while (!FeasibleMove(in,st,mv))
    AnyNextMove(in,st,mv);
}

bool IA_SwapNeighborhoodExplorer::AnyNextMove(const IA_Input& in, const IA_State& st, IA_Swap& mv) const
{
  if(mv.position2 < in.TotalWorkerAssignments() - 1)
  {
    mv.position2++;
    mv.item2 = st.GetAssignment(mv.worker2, mv.position2);
    return true;
  }
  else if(mv.worker2 < in.Workers() - 1)
  {
    mv.worker2++;
    mv.position2 = 0;
    mv.item2 = st.GetAssignment(mv.worker2, mv.position2);
    return true;
  }
  else if(mv.position1 < in.TotalWorkerAssignments() - 1)
  {
    mv.position1++;
    mv.item1 = st.GetAssignment(mv.worker1, mv.position1);
    mv.worker2 = mv.worker1 + 1;
    mv.position2 = 0;
    mv.item2 = st.GetAssignment(mv.worker2, mv.position2);
    return true;
  }
  else if (mv.worker1 < in.Workers() - 2)
  {
    mv.worker1++;
    mv.position1 = 0;
    mv.item1 = st.GetAssignment(mv.worker1, mv.position1);
    mv.worker2 = mv.worker1 + 1;
    mv.position2 = 0;
    mv.item2 = st.GetAssignment(mv.worker2, mv.position2);
    return true;
  }
  else
    return false;
}

bool IA_SwapNeighborhoodExplorer::NextMove(const IA_Input& in, const IA_State& st, IA_Swap& mv) const
{
  do
    if (!AnyNextMove(in,st,mv))
      return false;
  while (!FeasibleMove(in,st,mv));
  return true;
}

int IA_SwapDeltaEqualSets::ComputeDeltaCost(const IA_Input& in, const IA_State& st, const IA_Swap& mv) const
{
  
  int cost = 0, w;
  
  for(w=0; w<in.Workers(); w++)
  {
    if(w!=mv.worker1 && w!=mv.worker2)
    {
      cost += st.ComputeDeltaDistanceSets(mv.worker1, w, mv.item1, mv.item2);
      cost += st.ComputeDeltaDistanceSets(mv.worker2, w, mv.item2, mv.item1);
    }
  }
  
  return cost;
}

int IA_SwapDeltaCategoryWorkerAssignments::ComputeDeltaCost(const IA_Input& in, const IA_State& st, const IA_Swap& mv) const
{
  int cost = 0, c;
  
  for(c = 0; c < in.ItemCategories(); c++)
  {
    cost += st.ComputeDeltaCategoryWorkerAssignments(mv.worker1, mv.item1, mv.item2, c);
    cost += st.ComputeDeltaCategoryWorkerAssignments(mv.worker2, mv.item2, mv.item1, c);
  }
  
  return cost;
}

int IA_SwapDeltaPropertyItemAssignments::ComputeDeltaCost(const IA_Input& in, const IA_State& st, const IA_Swap& mv) const
{
  int cost = 0, p, level1, level2;
  
  for(p = 0; p < in.WorkerProperties(); p++)
  {
    level1 = in.GetWorker(mv.worker1).GetPropertyLevel(p);
    level2 = in.GetWorker(mv.worker2).GetPropertyLevel(p);
    if(level1 != level2)
    {
      cost += st.ComputeDeltaPropertyItemAssignments(mv.item1, mv.item2, p, level1);
      cost += st.ComputeDeltaPropertyItemAssignments(mv.item2, mv.item1, p, level2);
    }
  }
  
  return cost;
}


int IA_SwapDeltaMinimumItemQualityLevel::ComputeDeltaCost(const IA_Input& in, const IA_State& st, const IA_Swap& mv) const
{
  int cost = 0;
  
  cost += st.ComputeDeltaSwapMinimumItemQualityLevel(mv.item1, mv.item2, mv.worker1, mv.worker2);
  cost += st.ComputeDeltaSwapMinimumItemQualityLevel(mv.item2, mv.item1, mv.worker2, mv.worker1);
  
  return cost;
}
