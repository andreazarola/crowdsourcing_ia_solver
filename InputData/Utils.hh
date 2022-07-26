// File Utils.hh
#ifndef UTILS_HH
#define UTILS_HH

#include <vector>
#include <algorithm>

using namespace std;

bool MinorCompare(const vector<int>& v1, const vector<int>& v2);
void SortMatrix(vector<vector<int> >& m);

inline void RemoveElement(vector<int>& v, int e)
{
  v.erase(remove(v.begin(), v.end(), e), v.end());
}

inline bool Member(const vector<int>& v, int e)
{
  return find(v.begin(), v.end(), e) != v.end();
}
#endif
