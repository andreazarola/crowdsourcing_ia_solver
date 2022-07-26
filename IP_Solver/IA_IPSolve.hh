// File IA_IPSolve.hh
#ifndef IA_IPSOLVE_HH
#define IA_IPSOLVE_HH

#include "IA_Data.hh"

#define IL_STD
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

class IA_IP_Solver
{
public:
  IA_IP_Solver(const IA_Input &my_in) : in(my_in),  model(env), solver(model) {}
  tuple<double,double,IloAlgorithm::Status,double> Solve(IA_Output& out, int timeout, int threads, bool init_solution, bool verbose);

protected:
  const IA_Input & in;  
  IloEnv env;
  IloModel model;  
  IloCplex solver;
//   IloSolution sol;
};
#endif
