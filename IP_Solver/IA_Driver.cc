#include <iostream>
#include <cstdlib>
#include "IA_Data.hh"
#include "IA_IPSolve.hh"

int main(int argc, char* argv[])
{
  if (argc == 1)
    {
      throw runtime_error("Usage: IA_Test.exe <instance_file> <timeout> <solution_file>");
    }
  else if (argc == 2 || argc == 3 || argc == 4 )
    {
      unsigned timeout = 0;
      IA_Input in(argv[1]);
      bool init_solution = false;
      
      if(argc >= 3)
	timeout = atoi(argv[2]);
      IA_Output out(in);
      if(argc == 4)
	{
	  string sol_file = argv[3];
	  out.ReadSolution(sol_file);
	  //out.SortAssignments();
	  cout << out << endl;
	  init_solution = true;
	}
      IA_IP_Solver ip_solver(in);
      tuple<double,double,IloAlgorithm::Status,double> result;
      result = ip_solver.Solve(out,timeout,0,init_solution, true); // false: no init sol, true: verbose
      //out.SortAssignments();
      cout << out << endl;
      cout << "Cost: " << get<0>(result) << endl;
      cout << "Time: " << get<1>(result) << endl;
      out.PrintJsonFormat(cout);
    }
  else
    {
      cerr << "Usage: IA_Test.exe <instance_file> <timeout> <solution_file>" << endl;
      exit(1);
    }
  return 0;
}

