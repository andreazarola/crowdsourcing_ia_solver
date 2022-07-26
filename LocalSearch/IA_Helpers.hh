// File IA_Helpers.hh
#ifndef IA_HELPERS_HH
#define IA_HELPERS_HH

#include "IA_Basics.hh"
#include <easylocal.hh>

using namespace EasyLocal::Core;

/***************************************************************************
 * State Manager 
 ***************************************************************************/

class IA_EqualSets : public CostComponent<IA_Input,IA_State> 
{
public:
  IA_EqualSets(int w, bool hard) :    CostComponent<IA_Input,IA_State>(w,hard,"IA_EqualSets") 
  {}
  int ComputeCost(const IA_Input & in, const IA_State& st) const;
  void PrintViolations(const IA_Input & in, const IA_State& st, ostream& os = cout) const;
};

class  IA_CategoryWorkerAssignments: public CostComponent<IA_Input,IA_State> 
{
public:
  IA_CategoryWorkerAssignments(int w, bool hard) : CostComponent<IA_Input,IA_State>(w,hard,"IA_CategoryWorkerAssignments")
  {}
  int ComputeCost(const IA_Input & in, const IA_State& st) const;
  void PrintViolations(const IA_Input & in, const IA_State& st, ostream& os = cout) const;
};

class  IA_PropertyItemAssignments: public CostComponent<IA_Input,IA_State> 
{
public:
  IA_PropertyItemAssignments(int w, bool hard) : CostComponent<IA_Input,IA_State>(w,hard,"IA_PropertyItemAssignments")
  {}
  int ComputeCost(const IA_Input & in, const IA_State& st) const;
  void PrintViolations(const IA_Input & in, const IA_State& st, ostream& os = cout) const;
};

class  IA_MinimumItemQualityLevel: public CostComponent<IA_Input,IA_State> 
{
public:
  IA_MinimumItemQualityLevel(int w, bool hard) : CostComponent<IA_Input,IA_State>(w,hard,"IA_MinimumItemQualityLevel")
  {}
  int ComputeCost(const IA_Input & in, const IA_State& st) const;
  void PrintViolations(const IA_Input & in, const IA_State& st, ostream& os = cout) const;
};

class IA_StateManager : public StateManager<IA_Input,IA_State> 
{
public:
  IA_StateManager();
  void RandomState(const IA_Input&, IA_State&);
  void GreedyState(const IA_Input&, IA_State&);
  bool CheckConsistency(const IA_Input&, const IA_State& st) const;

  int initial_violations, initial_objective;  // used to check the costs of the initial greedy solution
  double initial_time;
  void SetFullUpdate() { full_update = true; }
  bool FullUpdate() const { return full_update; }
protected:
  bool CreateRandomMinimalState(const IA_Input&, IA_State& st) const;
  void SingleCategoryGreedyState(const IA_Input&, IA_State& st) const;
  void TwoCategoriesGreedyState(const IA_Input&, IA_State& st) const;
  void ThreeCategoriesGreedyState(const IA_Input&, IA_State& st) const;
  bool UnavailableItems(vector<int>& p0, vector<int>& p1, 
                        vector<vector<vector<pair<int,int>>>> available_items) const;
  bool UnavailableItems(vector<int>& p0, vector<int>& p1, vector<int>& p2, 
                        vector<vector<vector<vector<pair<int,int>>>>> available_items) const;
  bool full_update;
}; 

/***************************************************************************
 * IA_Change Neighborhood Explorer:
 ***************************************************************************/

class IA_ChangeDeltaEqualSets
  : public DeltaCostComponent<IA_Input,IA_State,IA_Change>
{
public:
  IA_ChangeDeltaEqualSets(IA_EqualSets& cc)
    : DeltaCostComponent<IA_Input,IA_State,IA_Change>(cc,"IA_ChangeDeltaEqualSets")
  {}
  int ComputeDeltaCost(const IA_Input&, const IA_State& st, const IA_Change& mv) const;
};

class IA_ChangeDeltaCategoryWorkerAssignments
  : public DeltaCostComponent<IA_Input,IA_State,IA_Change>
{
public:
  IA_ChangeDeltaCategoryWorkerAssignments(IA_CategoryWorkerAssignments& cc)
    : DeltaCostComponent<IA_Input,IA_State,IA_Change>(cc,"IA_ChangeDeltaCategoryWorkerAssignments")
  {}
  int ComputeDeltaCost(const IA_Input&, const IA_State& st, const IA_Change& mv) const;
};

class IA_ChangeDeltaPropertyItemAssignments
  : public DeltaCostComponent<IA_Input,IA_State,IA_Change>
{
public:
  IA_ChangeDeltaPropertyItemAssignments(IA_PropertyItemAssignments& cc)
    : DeltaCostComponent<IA_Input,IA_State,IA_Change>(cc,"IA_ChangeDeltaPropertyItemAssignments")
  {}
  int ComputeDeltaCost(const IA_Input&, const IA_State& st, const IA_Change& mv) const;
};

class IA_ChangeDeltaMinimumItemQualityLevel
  : public DeltaCostComponent<IA_Input,IA_State,IA_Change>
{
public:
  IA_ChangeDeltaMinimumItemQualityLevel(IA_MinimumItemQualityLevel& cc)
    : DeltaCostComponent<IA_Input,IA_State,IA_Change>(cc,"IA_ChangeDeltaMinimumItemQualityLevel")
  {}
  int ComputeDeltaCost(const IA_Input&, const IA_State& st, const IA_Change& mv) const;
};

class IA_ChangeNeighborhoodExplorer
  : public NeighborhoodExplorer<IA_Input,IA_State,IA_Change> 
{
public:
  IA_ChangeNeighborhoodExplorer(StateManager<IA_Input,IA_State>& psm)
    : NeighborhoodExplorer<IA_Input,IA_State,IA_Change>(psm, "IA_ChangeNeighborhoodExplorer") {}
  void RandomMove(const IA_Input&, const IA_State&, IA_Change&) const;
  bool FeasibleMove(const IA_Input&, const IA_State&, const IA_Change&) const;
  void MakeMove(const IA_Input&, IA_State&,const IA_Change&) const;
  void FirstMove(const IA_Input&, const IA_State&,IA_Change&) const;
  bool NextMove(const IA_Input&, const IA_State&,IA_Change&) const;
protected:
  virtual void AnyRandomMove(const IA_Input&, const IA_State& st, IA_Change& mv) const;
  virtual void AnyFirstMove(const IA_Input&, const IA_State& st, IA_Change& mv) const;
  virtual bool AnyNextMove(const IA_Input&, const IA_State& st, IA_Change& mv) const;
};


/***************************************************************************
 * IA_Swap Neighborhood Explorer:
 ***************************************************************************/

class IA_SwapDeltaEqualSets
  : public DeltaCostComponent<IA_Input,IA_State,IA_Swap>
{
public:
  IA_SwapDeltaEqualSets(IA_EqualSets& cc)
    : DeltaCostComponent<IA_Input,IA_State,IA_Swap>(cc,"IA_SwapDeltaEqualSets")
  {}
  int ComputeDeltaCost(const IA_Input&, const IA_State& st, const IA_Swap& mv) const;
};

class IA_SwapDeltaCategoryWorkerAssignments
  : public DeltaCostComponent<IA_Input,IA_State,IA_Swap>
{
public:
  IA_SwapDeltaCategoryWorkerAssignments(IA_CategoryWorkerAssignments& cc)
    : DeltaCostComponent<IA_Input,IA_State,IA_Swap>(cc,"IA_SwapDeltaCategoryWorkerAssignments")
  {}
  int ComputeDeltaCost(const IA_Input&, const IA_State& st, const IA_Swap& mv) const;
};

class IA_SwapDeltaPropertyItemAssignments
  : public DeltaCostComponent<IA_Input,IA_State,IA_Swap>
{
public:
  IA_SwapDeltaPropertyItemAssignments(IA_PropertyItemAssignments& cc)
    : DeltaCostComponent<IA_Input,IA_State,IA_Swap>(cc,"IA_SwapDeltaPropertyItemAssignments")
  {}
  int ComputeDeltaCost(const IA_Input&, const IA_State& st, const IA_Swap& mv) const;
};

class IA_SwapDeltaMinimumItemQualityLevel
  : public DeltaCostComponent<IA_Input,IA_State,IA_Swap>
{
public:
  IA_SwapDeltaMinimumItemQualityLevel(IA_MinimumItemQualityLevel& cc)
    : DeltaCostComponent<IA_Input,IA_State,IA_Swap>(cc,"IA_SwapDeltaMinimumItemQualityLevel")
  {}
  int ComputeDeltaCost(const IA_Input&, const IA_State& st, const IA_Swap& mv) const;
};

class IA_SwapNeighborhoodExplorer
  : public NeighborhoodExplorer<IA_Input,IA_State,IA_Swap> 
{
public:
  IA_SwapNeighborhoodExplorer(StateManager<IA_Input,IA_State>& psm)
    : NeighborhoodExplorer<IA_Input,IA_State,IA_Swap>(psm, "IA_SwapNeighborhoodExplorer") {}
  void RandomMove(const IA_Input&, const IA_State&, IA_Swap&) const;
  bool FeasibleMove(const IA_Input&, const IA_State&, const IA_Swap&) const;
  void MakeMove(const IA_Input&, IA_State&,const IA_Swap&) const;
  void FirstMove(const IA_Input&, const IA_State&,IA_Swap&) const;
  bool NextMove(const IA_Input&, const IA_State&,IA_Swap&) const;
protected:
  bool AnyNextMove(const IA_Input&, const IA_State&,IA_Swap&) const;
};


/***************************************************************************
 * Output Manager:
 ***************************************************************************/
class IA_OutputManager
  : public OutputManager<IA_Input,IA_Output,IA_State> 
{
public:
  IA_OutputManager()
    : OutputManager<IA_Input,IA_Output,IA_State>("IAOutputManager") {}
  void InputState(const IA_Input&, IA_State&, const IA_Output&) const;
  void OutputState(const IA_Input&, const IA_State&, IA_Output&) const;
  json ConvertToJSON(const IA_Input& in, const IA_State& st) const;
}; 

inline void InitialPermutation(vector<int>& v, int levels)
{
  //  int repetitions = v.size()/levels;
  for (unsigned i = 0; i < v.size(); i++)
    v[i] = i % levels; 
}

inline void FirstPermutation(vector<int>& v, int levels)
{
  int repetitions = static_cast<int>(v.size())/levels;
  for (unsigned i = 0; i < v.size(); i++)
    v[i] = i/repetitions;
}

#endif
