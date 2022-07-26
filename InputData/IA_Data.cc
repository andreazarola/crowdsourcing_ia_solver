// File IA_Data.cc
#include "IA_Data.hh"
#include "Utils.hh"

#include <fstream>

ostream& operator<<(ostream& os, const Feature& ca)
{
  int c;
  os << ca.id << ":"  << " assignments=" << ca.assignments << ", [";
  for (c=0; c<ca.levels-1; c++)
    os << ca.level_vector[c] << ", ";
  os << ca.level_vector[c] << "]";
  return os;
}

Feature::Feature(string c_id, int a, int l)
{
  id = c_id;
  assignments = a;
  levels = l;
  level_vector.resize(levels);
}

Feature& Feature::operator=(Feature c)
{
  id = c.id;
  assignments = c.assignments;
  levels = c.levels;
  level_vector = c.level_vector;
  return *this;
}

ostream& operator<<(ostream& os, const Item& i)
{
  size_t c;
  os << i.id << ": ";
  for(c=0; c<i.categories.size()-1; c++)
    os << i.categories[c] << ", ";
  os << i.categories[c];
  return os;
}

ostream& operator<<(ostream& os, const Worker& w)
{
  size_t p;
  os << w.id << ": ";
  for(p=0; p< w.properties.size()-1; p++)
    os << w.properties[p] << ", ";
  os << w.properties[p];
  return os;
}

IA_Input::IA_Input(json j)
{
  int c, i, k, l, p, expertise;
  string item_id, worker_id, cat, level, pro;
  int max_levels = 0;

  name = j["id"];
  item_categories = static_cast<int>(j["categories"].size());
  items = static_cast<int>(j["items"].size());
  min_item_repetitions = j["min_item_repetitions"];
  min_item_quality_level = j["min_item_quality_level"];

  worker_properties = static_cast<int>(j["properties"].size());
  workers = static_cast<int>(j["workers"].size());

  for(c=0; c<item_categories; c++)
    {
      item_category_vector.push_back(Feature(j["categories"][c]["id"], j["categories"][c]["worker_assignments"], static_cast<int>(j["categories"][c]["levels"].size())));
      for(l=0; l<int(j["categories"][c]["levels"].size()); l++)
       	item_category_vector[c].SetLevel(l, j["categories"][c]["levels"][l]);
      if(c==0)
        {
          total_worker_assignments = item_category_vector[c].Levels() * item_category_vector[c].Assignments();
          if (workers * total_worker_assignments < items * min_item_repetitions)
            throw runtime_error("Insufficient number of workers for item covering");
        }
      else if(total_worker_assignments != item_category_vector[c].Levels() * item_category_vector[c].Assignments())
	throw runtime_error("Total worker assignments error for category " + item_category_vector[c].Id());
      if(max_levels < item_category_vector[c].Levels())
	max_levels = item_category_vector[c].Levels();
    }
  
   for(i=0; i<items; i++)
     {
       item_id = j["items"][i]["id"];
       item_vector.push_back(Item(item_id, item_categories));
       if(int(j["items"][i]["categories"].size()) != item_categories)
	 throw runtime_error("Uncomplete categories for item: " + item_id);
       for(k=0; k<item_categories; k++)
	 {
	   cat = j["items"][i]["categories"][k]["id"];
	   c = FindItemCategory(cat);
	   if(c == item_categories)
	     throw runtime_error("Unexisting category: " + cat);
	   level = j["items"][i]["categories"][k]["level"];
	   l = FindItemCategoryLevel(c, level);
	   if(item_category_vector[c].Levels() == l)
	     throw runtime_error("Item " + item_id + ", unexisting level: " + level +
				 " for category " + cat);
	   item_vector[i].SetCategoryLevel(c,l);
	 }
     }

   max_item_category_levels = max_levels;

   max_levels = 0;
   for(p=0; p<worker_properties; p++)
     {
       worker_property_vector.push_back(Feature(j["properties"][p]["id"], j["properties"][p]["item_assignments"], static_cast<int>(j["properties"][p]["levels"].size())));
      for(l=0; l<int(j["properties"][p]["levels"].size()); l++)
       	worker_property_vector[p].SetLevel(l, j["properties"][p]["levels"][l]);
      if(max_levels < worker_property_vector[p].Levels())
	max_levels = worker_property_vector[p].Levels();
    }

   max_worker_property_levels = max_levels;
   
   for(i=0; i<workers; i++)
     {
       worker_id = j["workers"][i]["id"];
       expertise = j["workers"][i]["expertise"];
       worker_vector.push_back(Worker(worker_id, expertise, worker_properties));
       if(int(j["workers"][i]["properties"].size()) != worker_properties)
	 throw runtime_error("Uncomplete properties for worker: " + worker_id);
       for(k=0; k<worker_properties; k++)
	 {
	   pro = j["workers"][i]["properties"][k]["id"];
	   p = FindWorkerProperty(pro);
	   if(p == worker_properties)
	     throw runtime_error("Unexisting property: " + pro);
	   level = j["workers"][i]["properties"][k]["level"];
	   l = FindWorkerPropertyLevel(p, level);
	   if(worker_property_vector[p].Levels() == l)
	     throw runtime_error("Worker " + worker_id + ", unexisting level: " + level +
				 " for property " + pro);
	   worker_vector[i].SetPropertyLevel(p,l);
	 }
     }
   
   // //Old version: without explicit worker list
   // // Upper Bound
   // double approx = 1.0,  lower_bound = ComputeLowerBound(); //(items * min_item_repetitions)/total_worker_assignments;
   // if(!j["workers"].empty())
   //   {
   //     workers = j["workers"];
   //     if(workers < lower_bound)
   // 	 throw runtime_error("Solution Infeasible: insufficient number of workers to obtain a feasible state");
   //   }
   // else
   //   workers = round(approx * lower_bound);
}

ostream& operator<<(ostream& os, const IA_Input& ia)
{
  int c, i, p;
   
  os << "ID: " << ia.name << endl;

  os << "ITEM_CATEGORIES:" << endl;
  for(c=0; c<ia.item_categories; c++)
    os << ia.item_category_vector[c] << endl;

  os << "ITEMS:" << endl;
  for(i=0; i<ia.items; i++)
    {
      //      os << ia.item_vector[i] << endl;
      os << ia.item_vector[i].Id() << ": ";
      for(c=0; c<ia.item_categories-1; c++)
	{
	  os << ia.item_category_vector[c].Id() << "/"
	     << ia.item_category_vector[c].GetLevel(ia.item_vector[i].GetCategoryLevel(c)) << ", ";
	}
	  os << ia.item_category_vector[c].Id() << "/"
	     << ia.item_category_vector[c].GetLevel(ia.item_vector[i].GetCategoryLevel(c)) << endl;
    }

  os << "WORKER_PROPERTIES:" << endl;
  for(p=0; p<ia.worker_properties; p++)
    os << ia.worker_property_vector[p] << endl;
  
  os << "WORKERS:" << endl;
  for(i=0; i<ia.workers; i++)
    {
      //      os << ia.worker_vector[i] << endl;
      os << ia.worker_vector[i].Id() << ": ";
      for(p=0; p<ia.worker_properties-1; p++)
	{
	  os << ia.worker_property_vector[p].Id() << "/"
	     << ia.worker_property_vector[p].GetLevel(ia.worker_vector[i].GetPropertyLevel(p)) << ", ";
	}
      os << ia.worker_property_vector[p].Id() << "/"
	 << ia.worker_property_vector[p].GetLevel(ia.worker_vector[i].GetPropertyLevel(p)) << endl;
    }
  
  return os;
}

IA_Output::IA_Output(const IA_Input& my_in)
  : in(my_in), assignments(in.Workers())
{
  used_workers = in.Workers();
}

void IA_Output::Reset()
{
  for(int w=0; w<in.Workers(); w++)
      assignments[w].clear();
}

IA_Output& IA_Output::operator=(const IA_Output& out)	
{
  assignments = out.assignments;
  return *this;
}

ostream& operator<<(ostream& os, const IA_Output& out)
{
  int w, k;
  
  os << "ASSIGNMENTS: " << out.used_workers << endl; // << " / " << out.in.Workers() << endl;
  for(w=0; w<out.in.Workers(); w++)
  {
    if(!out.assignments[w].empty())
    {
      os << w << ": ";
      for(k=0; k<out.in.TotalWorkerAssignments(); k++)
      os << out.in.GetItem(out.assignments[w][k]).Id() << " ";
      os << endl;
    }
  }
  
  return os;
}

json IA_Output::ConvertToJSON() const
{
  json j_output;
  int w, k, uw=0;
  
  j_output["Instance_id"] = in.Name();
  j_output["Used_workers"] = used_workers;
  
  for(w=0; w<in.Workers(); w++)
  {
    if(!assignments[w].empty())
    {
      json j_worker;
      for(k=0; k<in.TotalWorkerAssignments(); k++)
      {
        j_worker["Assignments"][k] = in.GetItem(assignments[w][k]).Id();
      }
      j_worker["Id"] = in.GetWorker(w).Id();
      j_output["Workers"][uw] = j_worker;
      uw++;
    }
  }
  return j_output;
}

void IA_Output::PrintJsonFormat(ostream& os) const
{
  os << ConvertToJSON().dump(4) << endl;
}

void IA_Output::ReadSolution(string sol_file)
{
  ifstream is;
  is.open(sol_file);
  is >> *this;
  is.close();
}

istream& operator>>(istream& is, IA_Output& out)
{
  string s, i_id;
  int a, i, w;

  out.Reset();
 
  //  cerr << is.peek();
  
  if(is.peek() == 'A') // txt format
    {
      is >> s >> out.used_workers;
      
      for (w=0; w<out.used_workers; w++)
	{
	  out.assignments[w].resize(out.in.TotalWorkerAssignments());
	  is >> s;
	  for(a=0; a<out.in.TotalWorkerAssignments(); a++)
	    {
	      is >> i_id;
	      i = out.in.FindItemId(i_id);
	      if(i == out.in.Items())
		throw runtime_error("Unexisting item: " + i_id);
	      out.assignments[w][a] = i;
	    }
	}
    }
  else // json format
    {
      json j;
      
      is >> j;
      out.used_workers = j["Used_workers"];
      for (w=0; w<out.used_workers; w++)
	{
	  out.assignments[w].resize(out.in.TotalWorkerAssignments());
	  for(a=0; a<out.in.TotalWorkerAssignments(); a++)
	    {
	      i_id = j["Workers"][w]["Assignments"][a];
	      i = out.in.FindItemId(i_id);
	      if(i == out.in.Items())
		throw runtime_error("Unexisting item: " + i_id);
	      out.assignments[w][a] = i;
	    }
	}
    }

  return is;
}

// istream& operator>>(istream& is, IA_Output& out)
// {
//   string s, i_id;
//   int a, i, w;
  
//   is >> s >> out.used_workers;

//   for (w=0; w<out.used_workers; w++)
//     {
//       out.assignments[w].resize(out.in.TotalWorkerAssignments());
//       is >> s;
//       for(a=0; a<out.in.TotalWorkerAssignments(); a++)
// 	{
// 	  is >> i_id;
// 	  i = out.in.FindItemId(i_id);
// 	  if(i == out.in.Items())
// 	     throw runtime_error("Unexisting item: " + i_id);
// 	  out.assignments[w][a] = i;
// 	}
//     }
//   return is;
// }

void IA_Output::SortAssignments()
{ 
  SortMatrix(assignments);
}

int IA_Input::FindItemCategory(string id) const
{  
  for (int i = 0; i < static_cast<int>(item_category_vector.size()); i++)
    if (id == item_category_vector[i].Id())
      return i;
  return static_cast<int>(item_category_vector.size());
}

int IA_Input::FindItemCategoryLevel(int c, string id) const
{
  for (int i = 0; i < item_category_vector[c].Levels(); i++)
    if (id == item_category_vector[c].GetLevel(i))
      return i;
  return item_category_vector[c].Levels();
}

int IA_Input::FindItemId(string id) const
{  
  for (int i = 0; i < items; i++)
    if (id == item_vector[i].Id())
      return i;
  return static_cast<int>(item_vector.size());
}

int IA_Input::FindWorkerProperty(string id) const
{  
  for (int i = 0; i < static_cast<int>(worker_property_vector.size()); i++)
    if (id == worker_property_vector[i].Id())
      return i;
  return static_cast<int>(worker_property_vector.size());
}

int IA_Input::FindWorkerPropertyLevel(int p, string id) const
{
  for (int i = 0; i < worker_property_vector[p].Levels(); i++)
    if (id == worker_property_vector[p].GetLevel(i))
      return i;
  return worker_property_vector[p].Levels();
}


