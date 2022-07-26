// File IA_IPSolve.cc
#include <chrono>

#include "IA_IPSolve.hh"

tuple<double,double,IloAlgorithm::Status,double> IA_IP_Solver::Solve(IA_Output& out, int timeout, int threads, bool init_solution, bool verbose)
{
  std::stringstream name;
  int a, c, i, l, p, w, w2, used_workers = 0;
  
  // Declare variables
  IloArray<IloBoolVarArray> s(env, in.Workers()); // s[w][i] if item i is assigned to worker w
  IloBoolVarArray y(env, in.Workers()); // y[w] if worker w is employed
  IloArray<IloArray<IloBoolVarArray>> t(env, in.Workers()); // t[w][w2][i] if item i is assigned to worker w but not worker w2, or vice versa, 0 otherwise
  IloArray<IloArray<IloBoolVarArray>> r(env, in.Workers()); // r[w][w2][i] binary variable used to linearized contraint (3)

  IloExpr expr(env);
  IloNumExpr obj_f(env);

  // Declare constraints
  IloArray<IloArray<IloRangeArray>> category_worker_assignments(env, in.Workers()); // H1 (1), no items assigned to each worker
  // IloArray<IloArray<IloRangeArray>> category_item_assignments(env, in.Categories()); //H2 (2), no workers assigned to each item
  IloArray<IloRange> item_assignments(env, in.Items()); //H2 (2), no workers assigned to each item
  IloArray<IloRangeArray> item_sets_a(env, in.Workers()); // H3 (4) the same set of items cannot be assigned to two workers
  IloRangeArray channeling_s_y (env, in.Workers()); // (7) activation contraints for y[w]
  IloArray<IloArray<IloRangeArray>> channeling_s_t_a(env, in.Workers()); // (5) activation fot t[w][w2][i]
  IloArray<IloArray<IloRangeArray>> channeling_s_t_b(env, in.Workers()); // (6) activation fot t[w][w2][i]

  tuple<double,double,IloAlgorithm::Status,double> result;
  
  //Create variables
  // s[w][i]
  for(w=0; w<in.Workers(); w++)
   {
     s[w] = IloBoolVarArray(env, in.Items());
     for(i=0; i<in.Items(); i++)
       {
	 name << "s_" << w << "_" << i;
	 s[w][i] = IloNumVar(env, 0, 1, ILOBOOL, name.str().c_str());
	 name.str("");   
       }
   }

  // y[w] 
  for(w=0; w<in.Workers(); w++)
    {
      name << "y_" << w ;
      y[w] = IloNumVar(env, 0, 1, ILOBOOL, name.str().c_str());
      name.str("");   
    }
 
    // t[w][w2][i]
  for(w=0; w<in.Workers(); w++)
   {
     t[w] = IloArray<IloBoolVarArray>(env, in.Workers());
     for(w2=0; w2<in.Workers(); w2++)
       {
	 t[w][w2] = IloBoolVarArray(env, in.Items());
	 for(i=0; i<in.Items(); i++)
	   {
	     name << "t_" << w << "_" << w2 << "_" << i;
	     t[w][w2][i] = IloNumVar(env, 0, 1, ILOBOOL, name.str().c_str());
	     name.str("");   
	   }
       }
   }
  
  // r[w][w2][i]
  for(w=0; w<in.Workers(); w++)
    {
      r[w] = IloArray<IloBoolVarArray>(env, in.Workers());
      for(w2=0; w2<in.Workers(); w2++)
	{
	  r[w][w2] = IloBoolVarArray(env, in.Items());
	  for(i=0; i<in.Items(); i++)
	    {
	      name << "r_" << w << "_" << w2 << "_" << i;
	      r[w][w2][i] = IloNumVar(env, 0, 1, ILOBOOL, name.str().c_str());
	      name.str("");   
	    }
	}
    }
  cout << "Variables created" << endl;

    // soft constraint about item quality 

  IloExpr expr_obj_1(env);
  
  for(i=0; i<in.Items(); i++)
    {
      for(w=0; w<in.Workers(); w++)
	expr+= s[w][i] * in.GetWorker(w).Expertise();
	   
      expr_obj_1 += IloMax(0, in.MinItemQualityLevel() - expr);
      expr.clear();
    }

  // soft constraint about fairness

  IloExpr expr_obj_2(env);
  
  for(i=0; i<in.Items(); i++)
    {
      for(p=0; p<in.WorkerProperties(); p++)
	{
	  for(l=0; l<in.GetWorkerProperty(p).Levels(); l++)
	    {
	      for(w=0; w<in.Workers(); w++)
		{
		  if(in.GetWorker(w).GetPropertyLevel(p) == l)
		    expr+= s[w][i];
		}
	      expr_obj_2 += IloMax(0, in.GetWorkerProperty(p).Assignments() - expr);
	      expr.clear();
	    }
	}
    }

  // Objective function with soft constraints
  for(w=0; w<in.Workers(); w++)
    obj_f += HARD_WEIGHT * y[w];
  obj_f += W1 * expr_obj_1 + W2 * expr_obj_2;
   
  // Objective function
  // for(w=0; w<in.Workers(); w++)
  //   obj_f += y[w];

    // for solving only the feasible problem
  //obj_f += 0;
  //solver.setParam(IloCplex::Param::MIP::Strategy::FPHeur, 1);
  
    IloObjective obj(env, obj_f, IloObjective::Minimize);
    model.add(obj);
    
    cout << "Objective function created" << endl;

    // Channeling contraint s-w
      for(w=0; w<in.Workers(); w++)
	{
	  for(i=0; i<in.Items(); i++)
	    expr += s[w][i];
	  expr += - int(in.TotalWorkerAssignments()) * y[w];
	  name << "channeling_s_y_" << w;
	  channeling_s_y[w] = IloRange(env, -IloInfinity, expr, 0, name.str().c_str());
	  name.str(""); 
	  expr.clear();
	  model.add(channeling_s_y[w]);
	}
      cout << "Channeling s y constraints created" << endl;

    // H1, no items assigned to each worker
    for(w=0; w<in.Workers(); w++)
      {
	category_worker_assignments[w] = IloArray<IloRangeArray>(env, in.ItemCategories());	
	for(c=0; c<in.ItemCategories(); c++)
	  {
	    category_worker_assignments[w][c] = IloRangeArray(env, in.GetItemCategory(c).Levels());
	    for(l=0; l<in.GetItemCategory(c).Levels(); l++)
	      {
		for(i=0; i<in.Items(); i++)
		  if(in.GetItem(i).GetCategoryLevel(c) == int(l))
		    expr += s[w][i];
		expr += -in.GetItemCategory(c).Assignments() * y[w];
		name << "category_worker_assignments_" << w << "_" << c << "_" << l;
		category_worker_assignments[w][c][l] = IloRange(env, 0, expr, 0, name.str().c_str());
		name.str(""); 
		expr.clear();
		model.add(category_worker_assignments[w][c][l]);
	      }
	  }
      }
    cout << "Category worker assignments (H1) constraints created" << endl;

    // //H2 (2), no workers assigned to each item. Old version with min repetitions dependent on each category
    // for(c=0; c<in.Categories(); c++)
    //   {
    // 	category_item_assignments[c] = IloArray<IloRangeArray>(env, in.GetCategory(c).Levels());	
    // 	for(l=0; l<in.GetCategory(c).Levels(); l++)
    // 	  {
    // 	    category_item_assignments[c][l] = IloRangeArray(env, in.Items());
    // 	    for(i=0; i<in.Items(); i++)
    // 	      {
    // 		if(in.GetItem(i).GetCategoryLevel(c) == int(l))
    // 		  {
    // 		    for(w=0; w<in.Workers(); w++)
    // 		      expr += s[w][i];
    // 		    expr += -in.GetCategory(c).MinRepetitions();
    // 		    name << "category_item_assignments_" << c << "_" << l << "_" << i;
    // 		    category_item_assignments[c][l][i] = IloRange(env, 0, expr, IloInfinity, name.str().c_str());
    // 		    name.str(""); 
    // 		    expr.clear();
    // 		    model.add(category_item_assignments[c][l][i]);
    // 		  }
    // 	      }
    // 	  }
    //   }
    
    //H2 (2), no workers assigned to each item
    for(i=0; i<in.Items(); i++)
      {
	for(w=0; w<in.Workers(); w++)
	  expr += s[w][i];
	expr += -in.MinItemRepetitions();
	name << "item_assignments_" << i;
	item_assignments[i] = IloRange(env, 0, expr, IloInfinity, name.str().c_str());
	name.str(""); 
	expr.clear();
	model.add(item_assignments[i]);
      }

    cout << "Items assignments (H2) constraints created" << endl;

    // Number of constraints: factorial(in.Workers()-1) * in.Items
    // // (4) not linear
    // for(w=0; w<in.Workers()-1; w++)
    //   {
    //  	item_sets_a[w] = IloRangeArray(env, in.Workers());
    //  	for(w2=w+1; w2<in.Workers(); w2++)
    //  	  {
    // 	    for(i=0; i<in.Items(); i++)
    //  	      expr += IloAbs(s[w][i] - s[w2][i]);
	    
    //  	    expr += - y[w];
    //  	    name << "item_sets_a_" << w << "_" << w2;
    //  	    item_sets_a[w][w2] = IloRange(env, 0, expr, IloInfinity, name.str().c_str());
    //  	    name.str(""); 
    //  	    expr.clear();
    // 	    model.add(item_sets_a[w][w2]);
    //  	  }
    //   }
    // cout << "Item set (H3) constraints created" << endl;

    // (4)
    for(w=0; w<in.Workers()-1; w++)
      {
     	item_sets_a[w] = IloRangeArray(env, in.Workers());
     	for(w2=w+1; w2<in.Workers(); w2++)
     	  {
    	    for(i=0; i<in.Items(); i++)
     	      expr += t[w][w2][i];
	    
     	    expr += - y[w];
     	    name << "item_sets_a_" << w << "_" << w2;
     	    item_sets_a[w][w2] = IloRange(env, 0, expr, IloInfinity, name.str().c_str());
     	    name.str(""); 
     	    expr.clear();
	    model.add(item_sets_a[w][w2]);
     	  }
      }
    cout << "Item set (H3) constraints created" << endl;
    
    // Channeling contraint s-t (5)
      for(w=0; w<in.Workers()-1; w++)
      	{
    	  channeling_s_t_a[w] = IloArray<IloRangeArray>(env, in.Workers());
    	  for(w2=w+1; w2<in.Workers(); w2++)
    	    {
    	      channeling_s_t_a[w][w2] = IloRangeArray(env, in.Items());
    	      for(i=0; i<in.Items(); i++)
    		{		  
    		  expr += s[w][i] - s[w2][i] - t[w][w2][i] + 2 * r[w][w2][i];
    		  name << "channeling_s_t_a_" << w << "_" << w2 << "_" << i;
    		  channeling_s_t_a[w][w2][i] = IloRange(env, 0, expr, IloInfinity, name.str().c_str());
    		  name.str(""); 
    		  expr.clear();
		  model.add(channeling_s_t_a[w][w2][i]);
    		}
    	    }
    	}

    // Channeling contraint s-t (6)
      for(w=0; w<in.Workers()-1; w++)
      	{
    	  channeling_s_t_b[w] = IloArray<IloRangeArray>(env, in.Workers());
    	  for(w2=w+1; w2<in.Workers(); w2++)
    	    {
    	      channeling_s_t_b[w][w2] = IloRangeArray(env, in.Items());
    	      for(i=0; i<in.Items(); i++)
    		{		  
    		  expr += - s[w][i] + s[w2][i] - t[w][w2][i] - 2 * r[w][w2][i];
    		  name << "channeling_s_t_b_" << w << "_" << w2 << "_" << i;
    		  channeling_s_t_b[w][w2][i] = IloRange(env, -2, expr, IloInfinity, name.str().c_str());
    		  name.str(""); 
    		  expr.clear();
		  model.add(channeling_s_t_b[w][w2][i]);
    		}
    	    }
    	}

      cout << "Channeling t s constraints created" << endl;

    // Export model
    solver.exportModel("IA_model.lp");
   
    // Solve
    if (!verbose)
       solver.setOut(env.getNullStream());
     if (threads != 0)
       solver.setParam(IloCplex::Threads, threads);
     if (timeout > 0)
       solver.setParam(IloCplex::TiLim, timeout);

     if (init_solution)
       { // read initial solution from object out
     	 IloNumVarArray startVar(env);
     	 IloNumArray startVal(env);
	 vector<vector<bool>> s_matrix(in.Workers(), vector<bool>(in.Items(), 0));
	 for(w=0; w<out.UsedWorkers(); w++)
     	   {
	     for(i=0; i<in.Items(); i++)
	       {
		 for (a=0; a<in.TotalWorkerAssignments(); a++)
		   if (out.GetAssignment(w,a) == i)
		     s_matrix[w][i] = 1;
	       }
	   }
	 
     	 for(w=0; w<in.Workers(); w++)
     	   {
	     for(i=0; i<in.Items(); i++)
	       {
		 {
		   startVar.add(s[w][i]);
		   if (s_matrix[w][i] == 1) 
		     startVal.add(1);
		   else
		     startVal.add(0);
		 }
	       }
     	   }
     	 solver.addMIPStart(startVar, startVal);
     	 cerr << "Initial solution inserted" << endl;
     	 //      solver.setParam(IloCplex::HeurFreq, 0);
     	 //      solver.setParam(IloCplex::RINSHeur, rins_value);
      //      startVar.end();
      //      startVal.end();
       }
     
     // chrono::time_point<chrono::system_clock> start, end;
     // start = chrono::system_clock::now();
     solver.solve();
     // end = chrono::system_clock::now();
     get<1>(result) = solver.getTime();
     get<2>(result) = solver.getStatus(); //   // status in {Unknown,Feasible,Optimal,Infeasible,Unbounded,InfeasibleOrUnbounded,Error}
     get<3>(result) = solver.getBestObjValue(); // best known bound

     // Rebuild solution for output
     const double ROUNDING_ERROR = 0.01;
     out.Reset();
     if (get<2>(result) == IloAlgorithm::Feasible || get<2>(result) == IloAlgorithm::Optimal)
       {
	 cout << "s[w][i] variables:" << endl;
	 for(w=0; w<in.Workers(); w++)
	   {
	     cout << w << ": ";
	     for(i=0; i<in.Items(); i++)
	       {
		 cout << int(round(solver.getValue(s[w][i]))) << " ";
		 if(solver.getValue(s[w][i]) > 1.0 - ROUNDING_ERROR && solver.getValue(s[w][i]) < 1.0 + ROUNDING_ERROR)
		   {
		     out.AddAssignment(w, i);
		     //cout << in.GetItem(i).Id() << " "; 
		   }
	       }
	     cout << endl;
	     if(out.IsEmployed(w))
	       used_workers++;
	     //   cout << endl;
	   }
	 out.SetUsedWorkers(used_workers);
	  
	 cout << "y[w] variables:" << endl;
	 for(w=0; w<in.Workers(); w++)
	   {
	     if(solver.getValue(y[w]) > 1.0 - ROUNDING_ERROR && solver.getValue(y[w]) < 1.0 + ROUNDING_ERROR)
	       cout << solver.getValue(y[w]) << " ";
	     else
	       cout << 0 << " ";
	   }
	 cout << endl;

	 // cout << "t[w][w2][i]/r[w][w2][i] variables:" << endl;
	 // for(w=0; w<in.Workers()-1; w++)
	 //   {
	 //     cout  << w << ":" << endl;
	 //     for(w2=w+1; w2<in.Workers(); w2++)
	 //       {
	 // 	 cout << w2 << ": ";
	 // 	 for(i=0; i<in.Items(); i++)
	 // 	   cout << int(round(solver.getValue(t[w][w2][i]))) << "/"
	 // 		<< int(round(solver.getValue(r[w][w2][i]))) << " ";
	 // 	 cout << endl;
	 //       }
	 //     cout << endl;
	 //   }
	 // cout << endl;

	 	     
	 get<0>(result) = solver.getObjValue();
	 cout << "Obj " << get<0>(result) << endl;
	 cout << "Time " << get<1>(result) << endl;
	 
	 if(get<2>(result) == IloAlgorithm::Optimal)
	   cout << "OPTIMAL" << endl;
	 else
	   cout << "FEASIBLE" << endl;
       }
 else
   {
     cout << "Status " << get<2>(result) << endl;
     throw runtime_error("No feasible solution.");	 
     // get<0>(result) = 100000000;  // arbitrary high value
   }     
    return result;
}
