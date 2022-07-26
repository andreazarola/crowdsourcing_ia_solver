#include "Utils.hh"

bool MinorCompare(const vector<int>& v1, const vector<int>& v2)
{
  if(v2.size() == 0)
    return false;
  for (size_t i = 0; i < v1.size(); i++)
    if (v1[i] < v2[i])
	  return true;
	else if (v1[i] > v2[i])
	  return false;
  return false;
}

void SortMatrix(vector<vector<int> >& m)
{
  // sort each row by ascending order
  for(size_t i=0; i<m.size(); i++)
    sort(m[i].begin(), m[i].end());

  // sort by column
  sort(m.begin(), m.end(), MinorCompare);

  // empty assignments at the end
} 
