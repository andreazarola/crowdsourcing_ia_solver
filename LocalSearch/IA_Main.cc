#include "IA_Helpers.hh"
#include "testers/resttester.hh"

using namespace EasyLocal::Debug;

int main(int argc, const char* argv[])
{
  ParameterBox main_parameters("main", "Main Program options");
  
  // The set of arguments added are the following:
  
  Parameter<string> instance("instance", "Input instance", main_parameters);
  Parameter<long unsigned> seed("seed", "Random seed", main_parameters);
  Parameter<string> method("method", "Solution method (empty for tester)", main_parameters);
  Parameter<string> init_state("init_state", "Initial state (to be read from file)", main_parameters);
  Parameter<unsigned> observer("observer", "Attach the observers", main_parameters);
  Parameter<string> output_file("output_file", "Write the output to a file (filename required)", main_parameters);
  Parameter<double> swap_rate("swap_rate", "Swap rate", main_parameters);
  Parameter<bool> hard_only("hard_only", "Formulation with only hard constraints", main_parameters);
  Parameter<unsigned> hard_weight("hard_weight", "Weight of hard constraints", main_parameters);
  
  swap_rate = 0.8;
  
  // 3rd parameter: false = do not check unregistered parameters
  // 4th parameter: true = silent
  CommandLineParameters::Parse(argc, argv, false, true);
  
  //  if (!instance.IsSet())
  //    {
  //      cout << "Error: --main::instance filename option must always be set" << endl;
  //      return 1;
  //    }
  //  IA_Input in(instance);
  
  // Correction for large instances
  //   if(in.Items() >= 1000)
  //     swap_rate = 0.5;
  
  if (seed.IsSet())
    Random::SetSeed(seed);
  const int default_hard_weight = 1000;
  
  if (!hard_weight.IsSet())
    hard_weight = default_hard_weight;
  
  //cerr << Random::SetSeed(seed) << endl;
  // cerr << "LB " << in.ComputeLowerBound() << endl;
  
  // Cost components: second parameter is the cost, third is the type (true -> hard, false -> soft)
  IA_EqualSets cc1(default_hard_weight, true); // H3 (H1 and H2 are satisfied by construction)
  IA_CategoryWorkerAssignments cc2(hard_weight, true);
  IA_MinimumItemQualityLevel cc3(W1, false);
  IA_PropertyItemAssignments cc4(W2, false);
  
  IA_ChangeDeltaEqualSets dcc1(cc1);
  IA_ChangeDeltaCategoryWorkerAssignments dcc2(cc2);
  IA_ChangeDeltaMinimumItemQualityLevel dcc3(cc3);
  IA_ChangeDeltaPropertyItemAssignments dcc4(cc4);
  
  IA_SwapDeltaEqualSets s_dcc1(cc1);
  IA_SwapDeltaCategoryWorkerAssignments s_dcc2(cc2);
  IA_SwapDeltaMinimumItemQualityLevel s_dcc3(cc3);
  IA_SwapDeltaPropertyItemAssignments s_dcc4(cc4);
  
  // helpers
  IA_StateManager IA_sm;
  IA_ChangeNeighborhoodExplorer IA_nhe(IA_sm);
  IA_SwapNeighborhoodExplorer IA_snhe(IA_sm);
  
  IA_OutputManager IA_om;
  
  // All cost components must be added to the state manager
  //IA_sm.AddCostComponent(cc1);
  // IA_sm.SetFullUpdate();
  IA_sm.AddCostComponent(cc2);
  if(!hard_only.IsSet() || (hard_only.IsSet() && hard_only == false))
  {
    IA_sm.AddCostComponent(cc3);
    IA_sm.AddCostComponent(cc4);
  }
  
  // All delta cost components must be added to the neighborhood explorer
  //   IA_nhe.AddDeltaCostComponent(dcc1);
  IA_nhe.AddDeltaCostComponent(dcc2);
  if(!hard_only.IsSet() || (hard_only.IsSet() && hard_only == false))
  {
    IA_nhe.AddDeltaCostComponent(dcc3);
    IA_nhe.AddDeltaCostComponent(dcc4);
  }
  
  //   IA_snhe.AddDeltaCostComponent(s_dcc1);
  IA_snhe.AddDeltaCostComponent(s_dcc2);
  if(!hard_only.IsSet() || (hard_only.IsSet() && hard_only == false))
  {
    IA_snhe.AddDeltaCostComponent(s_dcc3);
    IA_snhe.AddDeltaCostComponent(s_dcc4);
  }
  
  SetUnionNeighborhoodExplorer<IA_Input, IA_State, DefaultCostStructure<int>, decltype(IA_nhe), decltype(IA_snhe)>
  IA_bnhe(IA_sm, "Bimodal", IA_nhe, IA_snhe, {1.0 - swap_rate, swap_rate});
  
  // runners
  HillClimbing<IA_Input, IA_State, IA_Change> IA_hc(IA_sm, IA_nhe, "IA_ChangeHillClimbing", "Change Hill Climbing");
  SteepestDescent<IA_Input, IA_State, IA_Change> IA_sd(IA_sm, IA_nhe, "IA_ChangeSteepestDescent", "Change Steepest Descent");
  SimulatedAnnealingEvaluationBased<IA_Input, IA_State, IA_Change> IA_sa(IA_sm, IA_nhe, "IA_ChangeSimulatedAnnealing", "Change Simulated Annealing");
  
  HillClimbing<IA_Input, IA_State, IA_Swap> IA_hc_sw(IA_sm, IA_snhe, "SW_HC", "Swap Hill Climbing");
  SteepestDescent<IA_Input, IA_State, IA_Swap> IA_sd_sw(IA_sm, IA_snhe, "IA_SwapSteepestDescent", "Swap Steepest Descent");
  SimulatedAnnealingEvaluationBased<IA_Input, IA_State, IA_Swap> IA_sa_sw(IA_sm, IA_snhe, "SSA", "Swap Simulated Annealing");
  
  SimulatedAnnealingEvaluationBased<IA_Input, IA_State, decltype(IA_bnhe)::Move> bsa(IA_sm, IA_bnhe, "BSA", "Bimodal Simulated Annealing");
  
  // tester
  Tester<IA_Input, IA_Output, IA_State> tester(IA_sm,IA_om);
  tester.AddMoveTester(IA_nhe, "IA_Change move");
  tester.AddMoveTester(IA_snhe, "IA_Swap move");
  tester.AddMoveTester(IA_bnhe, "IA_BI move");
  
  // rest tester
  RESTTester<IA_Input, IA_Output, IA_State> rest_tester(IA_sm, IA_om, "IA_REST");
  
  SimpleLocalSearch<IA_Input, IA_Output, IA_State> IA_solver(IA_sm, IA_om, "IA solver");
  if (!CommandLineParameters::Parse(argc, argv, true, false))
    return 1;
  
  if (!CommandLineParameters::Parse(argc, argv))
    return 1;
  
  if (seed.IsSet())
    Random::SetSeed(seed);
  
  if (method.IsSet() && method == string("rest"))
  {
    /*
    BSA::neighbors_accepted_ratio 0.19 --main::hard_weight 100000 --BSA::max_evaluations 100000000 --BSA::cooling_rate 0.99 --BSA::start_temperature 1765.63 --BSA::expected_min_temperature 0.1 */
    // configure a standard bsa for running without any parameter
    hard_weight = 100000;
    bsa.SetParameter("neighbors_accepted_ratio", 0.19);
    bsa.SetParameter("max_evaluations", 100000000UL);
    bsa.SetParameter("cooling_rate", 0.99);
    bsa.SetParameter("start_temperature", 1765.63);
    bsa.SetParameter("expected_min_temperature", 0.1);
    // set parameters of sa and friends
    // sa.SetParameter("...")
    rest_tester.Run();
    return 0;
  }
  
  if (!instance.IsSet())
    throw std::logic_error("Error when not using the rest tester --main::instance filename option must always be set");
  
  if (string(instance).find(".json") == string::npos)
    throw runtime_error("The input file must be in json format.");
  
  ifstream is(instance);
  json content;
  is >> content;
  IA_Input in(content);
  
  if (!method.IsSet())
  { // If no search method is set -> enter in the tester
    if (init_state.IsSet())
      tester.RunMainMenu(init_state);
    else
      tester.RunMainMenu();
  }
  else
  {
    if (method == string("SA"))
    {
      IA_solver.SetRunner(IA_sa);
    }
    else if (method == string("BSA"))
    {
      //           cerr << in.Workers() << " " << in.ComputeLowerBound() << endl;
      if(in.Workers() == in.ComputeLowerBound())
      {
        double v;
        long unsigned e;
        bsa.GetParameterValue("start_temperature", v);
        IA_sa_sw.SetParameter("start_temperature",v);
        
        bsa.GetParameterValue("max_evaluations", e);
        IA_sa_sw.SetParameter("max_evaluations", e);
        
        bsa.GetParameterValue("cooling_rate", v);
        IA_sa_sw.SetParameter("cooling_rate",v);
        
        bsa.GetParameterValue("neighbors_accepted_ratio", v);
        IA_sa_sw.SetParameter("neighbors_accepted_ratio",v);
        
        bsa.GetParameterValue("expected_min_temperature", v);
        IA_sa_sw.SetParameter("expected_min_temperature",v);
        
        IA_solver.SetRunner(IA_sa_sw);
      }
      else
        IA_solver.SetRunner(bsa);
    }
    else if (method == string("HC_SW"))
    {
      IA_solver.SetRunner(IA_hc_sw);
    }
    else if (method == string("SD"))
    {
      IA_solver.SetRunner(IA_sd_sw);
    }
    else
      throw runtime_error("Unknown method");
    auto result = IA_solver.Solve(in);
    // result is a tuple: 0: solutio, 1: number of violations, 2: total cost, 3: computing time
    IA_Output out = result.output;
    
    // repair state for equalset violations
    IA_State st(in);
    IA_sm.SetFullUpdate();
    IA_om.InputState(in,st,out);
    int equal_set_violations = cc1.ComputeCost(in,st);
    if (equal_set_violations > 0)
    {
      IA_sm.AddCostComponent(cc1);
      IA_snhe.AddDeltaCostComponent(s_dcc1);
      IA_solver.SetRunner(IA_hc_sw);
      IA_hc_sw.SetParameter("max_idle_iterations", 1000000UL);
      result = IA_solver.Resolve(in,out);
      IA_om.InputState(in,st,out);
    }
    int num_items_low_quality = st.ComputeNumItemsLowQuality();
    int fairness_cost = cc4.ComputeCost(in,st);
    if (output_file.IsSet())
    { // write the output on the file passed in the command line
      ofstream os(static_cast<string>(output_file).c_str());
      //os << out << endl;
      out.PrintJsonFormat(os);
      os << "Cost: " << result.cost.total << endl;
      os << "Violations: " << result.cost.violations << endl;
      
      os << "Initial_cost: " << IA_sm.initial_objective  << endl;
      os << "Initial_violations: " << IA_sm.initial_violations << endl;
      os << "Initial_time: " << IA_sm.initial_time << endl;
      os << "Time: " << result.running_time << "s " << endl;
      os << "Seed: " << Random::GetSeed()  << endl;
      os.close();
    }
    else
    { // write the solution in the standard output
      // cout << out << endl;
      // cout << "Cost: " << result.cost.total << endl;
      // cout << "Time: " << result.running_time << "s " << endl;
      //out.PrintJsonFormat(cout);
      double avg_qual_deficit = 0.0;
      if (num_items_low_quality > 0)
        avg_qual_deficit = cc3.ComputeCost(in,st)/((double)num_items_low_quality*in.MinItemQualityLevel());
      
      cout << "{\"cost\": " << result.cost.total <<  ", "
      << "\"violations\": " << result.cost.violations <<  ", "
      //<< "\"time\": " << result.running_time + result.cost.violations * 10000 <<  ", "
      << "\"initial_cost\": " << IA_sm.initial_objective <<  ", "
      << "\"initial_violations\": " << IA_sm.initial_violations <<  ", "
      << "\"initial_time\": " << IA_sm.initial_time <<  ", "
      << "\"equal_set_previolations\": " << equal_set_violations <<  ", "
      << "\"time\": " << result.running_time <<  ", "
      << "\"workers\": " << in.Workers() <<  ", "
      << "\"seed\": " << Random::GetSeed() << ", "
      << "\"item_quality\": " << cc3.ComputeCost(in,st) <<  ", "
      << "\"fairness\": " << fairness_cost <<  ", "
      << "\"fairness_index\": " << st.ComputeFairnessIndex(fairness_cost) <<  ", "
      << "\"num_low_qual_items\": " <<   num_items_low_quality << ", "
      << "\"perc_low_qual_items\": " <<  num_items_low_quality/(double)in.Items() << ", "
      << "\"avg_qual_deficit\": " <<  avg_qual_deficit
      << "} " << endl;
    }
  }
  return 0;
}
