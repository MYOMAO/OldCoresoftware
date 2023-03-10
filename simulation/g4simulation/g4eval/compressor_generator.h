/**
 * Compression algorithm for use in the CMSSW and sPHENIX projects.
 * Using 16-bit short integers to store 32-bit floating-point values.
 * Author: fishyu@iii.org.tw
 * June 8, 2021
 */
//-----------------------------------------------------------------------------
#include <map>
#include <set>
#include <utility>
#include <random>
#include <vector>

#include "RtypesCore.h"
//-----------------------------------------------------------------------------
using namespace std;

UShort_t residesIn(Float_t raw, vector<Float_t>* dict) {
  for (size_t i = 0; i < dict->size(); ++i) {
    if (raw <= dict->at(i)) {
      if (i == 0)
        return 0;
      else if ((dict->at(i) - raw) < (raw - dict->at(i-1)))
        return i;
      else
        return i - 1;
    }
  }
  return dict->size() - 1;
}
//-----------------------------------------------------------------------------
/**
 * approx() compresses data generated by a normal distribution and returns the maximum absolute difference between the actual and approximated data.
 */
Float_t approx(
  vector<UShort_t>* order, 
  vector<Float_t>* dict, 
  vector<size_t>* cnt, 
  Int_t n_entries, 
  std::default_random_engine& generator,
  std::normal_distribution<double>& distribution,
  size_t maxNumClusters
);
//-----------------------------------------------------------------------------
Int_t newLoc(vector<Int_t>* loc_vec, vector<vector<Int_t>>* loc_vec_vec);
void removeDiff(Float_t distance, Float_t min, map<Float_t, set<Float_t>>* distance_min_set_map);
void addDiff(Float_t distance, Float_t min, map<Float_t, set<Float_t>>* distance_min_set_map);
//-----------------------------------------------------------------------------
Float_t approx(vector<UShort_t>* order, vector<Float_t>* dict, vector<size_t>* cnt, Int_t n_entries, std::default_random_engine& generator, std::normal_distribution<double>& distribution, size_t maxNumClusters)
{
  Float_t maxAbsErrorDoubled = (Float_t) 0;

  map<Float_t, pair<Float_t, Int_t>> min_max_loc_map;
  vector<vector<Int_t>> loc_vec_vec;
  vector<Int_t> loc_vec;
  map<Float_t, set<Float_t>> distance_min_set_map;

  for (Int_t j = 0 ; j < n_entries; j++){
    Float_t number = distribution(generator);
    Float_t* gen_ = &number;
    
    map<Float_t, pair<Float_t, Int_t>>::iterator mmlm = min_max_loc_map.find(*gen_);

    if (mmlm != min_max_loc_map.end())
      loc_vec_vec[mmlm->second.second].push_back(j);
    else {
      Int_t loc = newLoc(&loc_vec, &loc_vec_vec);

      loc_vec_vec[loc].push_back(j);

      min_max_loc_map[*gen_] = pair<Float_t, Int_t>(*gen_, loc);

      mmlm = min_max_loc_map.find(*gen_);
      if (mmlm != min_max_loc_map.begin() && *gen_ <= prev(mmlm)->second.first) {
        loc_vec_vec[prev(mmlm)->second.second].push_back(j);
        loc_vec_vec[mmlm->second.second].clear();
        loc_vec.push_back(mmlm->second.second);

        min_max_loc_map.erase(mmlm);

      } else if (min_max_loc_map.size() >= 2) {
        if (mmlm != min_max_loc_map.begin() && mmlm != prev(min_max_loc_map.end())) {

          removeDiff(next(mmlm)->second.first - prev(mmlm)->first, prev(mmlm)->first, &distance_min_set_map);
        }

        if (mmlm != min_max_loc_map.begin())
          addDiff(mmlm->second.first - prev(mmlm)->first, prev(mmlm)->first, &distance_min_set_map);

        if (mmlm != prev(min_max_loc_map.end()))
          addDiff(next(mmlm)->second.first - mmlm->first, mmlm->first, &distance_min_set_map);
      }
    }

    if (min_max_loc_map.size() <= maxNumClusters)
      continue;
   
    map<Float_t, set<Float_t>>::iterator dmsm = distance_min_set_map.begin();
    Float_t min = *(dmsm->second.begin());
    
    dmsm->second.erase(min);
    if (dmsm->second.empty())
      distance_min_set_map.erase(dmsm);
    
    mmlm = min_max_loc_map.find(min);
    if (mmlm != min_max_loc_map.begin())
      removeDiff(mmlm->second.first - prev(mmlm)->first, prev(mmlm)->first, &distance_min_set_map);

    if (next(mmlm) != prev(min_max_loc_map.end()))
      removeDiff(next(next(mmlm))->second.first - next(mmlm)->first, next(mmlm)->first, &distance_min_set_map);

    vector<Int_t>* s = &(loc_vec_vec[next(mmlm)->second.second]);
    loc_vec_vec[mmlm->second.second].insert(loc_vec_vec[mmlm->second.second].end(),s->begin(), s->end());
    mmlm->second.first = next(mmlm)->second.first;
    min_max_loc_map.erase(next(mmlm));
    mmlm = min_max_loc_map.find(min);
    maxAbsErrorDoubled = max(maxAbsErrorDoubled, mmlm->second.first - mmlm->first);
    if (mmlm != min_max_loc_map.begin())
      addDiff(mmlm->second.first - prev(mmlm)->first, prev(mmlm)->first, &distance_min_set_map);

    if (mmlm != prev(min_max_loc_map.end()))
      addDiff(next(mmlm)->second.first - mmlm->first, mmlm->first, &distance_min_set_map);

  }
  
  order->resize(n_entries);
  for (const auto &mmlm : min_max_loc_map) {
    Double_t estimate = (Double_t) (mmlm.first + mmlm.second.first) / (Double_t) 2;
	  
    for (const auto &index : loc_vec_vec[mmlm.second.second]) {
      (*order)[index] = dict->size();	  
    }

    dict->push_back(estimate);
    cnt->push_back(loc_vec_vec[mmlm.second.second].size());
  }
  
  return maxAbsErrorDoubled / (double) 2; //sqrt((squaredSum / (Double_t) n_entries) - avg * avg);
}

Int_t newLoc(vector<Int_t>* loc_vec, vector<vector<Int_t>>* loc_vec_vec)
{
  if (!loc_vec->empty()) {
    Int_t loc = loc_vec->back();
    loc_vec->pop_back();
    return loc;
  }
 
  Int_t loc = loc_vec_vec->size();
  loc_vec_vec->push_back({});
  return loc;
}


void removeDiff(Float_t distance, Float_t min, map<Float_t, set<Float_t>>* distance_min_set_map) 
{
  map<Float_t, set<Float_t>>::iterator dmsm = distance_min_set_map->find(distance);
  dmsm->second.erase(min);

  if (dmsm->second.empty())
    distance_min_set_map->erase(dmsm);
}

void addDiff(Float_t distance, Float_t min, map<Float_t, set<Float_t>>* distance_min_set_map) 
{
  map<Float_t, set<Float_t>>::iterator dmsm = distance_min_set_map->find(distance);
  if (dmsm == distance_min_set_map->end()) {
    (*distance_min_set_map)[distance] = {min};
  } else {
    dmsm->second.insert(min);
  }
}
