//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.h"
#include <map>
#include <vector>
#include <utility>
#include <iostream>

using namespace std;

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;     // Number of sets in the I$
uint32_t icacheAssoc;    // Associativity of the I$
uint32_t icacheHitTime;  // Hit Time of the I$

uint32_t dcacheSets;     // Number of sets in the D$
uint32_t dcacheAssoc;    // Associativity of the D$
uint32_t dcacheHitTime;  // Hit Time of the D$

uint32_t l2cacheSets;    // Number of sets in the L2$
uint32_t l2cacheAssoc;   // Associativity of the L2$
uint32_t l2cacheHitTime; // Hit Time of the L2$
uint32_t inclusive;      // Indicates if the L2 is inclusive

uint32_t blocksize;      // Block/Line size
uint32_t memspeed;       // Latency of Main Memory

//------------------------------------//

//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
//

uint32_t itagMask = 0;
uint32_t iindexMask = 0;
uint32_t dtagMask = 0;
uint32_t dindexMask = 0;
uint32_t l2tagMask = 0;
uint32_t l2indexMask = 0;

map<uint32_t, vector<uint32_t>> imap;
map<uint32_t, vector<uint32_t>> dmap;
map<uint32_t, vector<uint32_t>> l2map;


//------------------------------------//
//          Cache Functions           //
//------------------------------------//


// Generate Mask to extract tag and index from the address
void generate_mask(uint32_t cacheSets, uint32_t cacheAssoc, uint32_t *tag, uint32_t *index) {
  uint32_t tagXor = 0;
  for (int i = 1; i < cacheSets; i *= 2) {
    (*index) <<= 1;
    (*index) += 1;
    tagXor <<= 1;
    tagXor += 1;
  }

  for (int i = 1; i < blocksize; i *= 2) {
    (*index) <<= 1;
    tagXor <<= 1;
    tagXor += 1;
  }

  (*tag) = ~tagXor;
}

// Update l1 cache to ensure the inclusive property
void update_l1cache(uint32_t addr) {
  uint32_t iindex = iindexMask & addr;
  uint32_t itag = itagMask & addr;
  uint32_t dindex = dindexMask & addr;
  uint32_t dtag = dtagMask & addr;

  if (imap.find(iindex) != imap.end()) {
    vector<uint32_t> &v = imap[iindex];
    int i = 0;
    for (; i < v.size(); i++) {
      if (v[i] == itag)
      break;
    }
    if (i < v.size())
      v.erase(v.begin() + i);
  }

  if (dmap.find(dindex) != dmap.end()) {
    vector<uint32_t> &v = dmap[dindex];
    int i = 0;
    for (; i < v.size(); i++) {
      if (v[i] == dtag)
      break;
    }
    if (i < v.size())
      v.erase(v.begin() + i);
  }
}


// Initialize the Cache Hierarchy
//
void
init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;
  
  // Initialize Cache Simulator Data Structures
  //

  generate_mask(icacheSets, icacheAssoc, &itagMask, &iindexMask);
  generate_mask(dcacheSets, dcacheAssoc, &dtagMask, &dindexMask);
  generate_mask(l2cacheSets, l2cacheAssoc, &l2tagMask, &l2indexMask);
}


// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
icache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //
  icacheRefs++;


  if (icacheSets == 0)
    return l2cache_access(addr);

  uint32_t tag = itagMask & addr;
  uint32_t index = iindexMask & addr;

  vector<uint32_t>& v = imap[index];

  int i = 0;
  for (; i < v.size(); i++) {
    if (v[i] == tag)
      break;
  }

  if (i < v.size()) {
    v.erase(v.begin() + i);
    v.push_back(tag);
    return icacheHitTime;
  }

  icacheMisses++;
  uint32_t penalty = l2cache_access(addr);

  if (v.size() == icacheAssoc) {
    v.erase(v.begin());
  }

  v.push_back(tag);

  icachePenalties += penalty;
  
  return icacheHitTime + penalty;
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  //
  //TODO: Implement D$
  //
  dcacheRefs++;


  if (dcacheSets == 0)
    return l2cache_access(addr);

  uint32_t tag = dtagMask & addr;
  uint32_t index = dindexMask & addr;

  vector<uint32_t>& v = dmap[index];

  int i = 0;
  for (; i < v.size(); i++) {
    if (v[i] == tag)
      break;
  }

  if (i < v.size()) {
    v.erase(v.begin() + i);
    v.push_back(tag);
    return dcacheHitTime;
  }

  dcacheMisses++;
  uint32_t penalty = l2cache_access(addr);

  if (v.size() == dcacheAssoc) {
    v.erase(v.begin());
  }

  v.push_back(tag);

  dcachePenalties += penalty;
  
  return dcacheHitTime + penalty;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
l2cache_access(uint32_t addr)
{
  //
  //TODO: Implement L2$
  //
  l2cacheRefs++;

  uint32_t tag = l2tagMask & addr;
  uint32_t index = l2indexMask & addr;

  vector<uint32_t>& v = l2map[index];

  int i = 0;
  for (; i < v.size(); i++) {
    if (v[i] == tag)
      break;
  }

  if (i < v.size()) {
    v.erase(v.begin() + i);
    v.push_back(tag);
    return l2cacheHitTime;
  }

  l2cacheMisses++;

  if (v.size() == l2cacheAssoc) {
    if (inclusive)
      update_l1cache(v[0] | index);
    v.erase(v.begin());
  }

  v.push_back(tag);


  l2cachePenalties += memspeed;
  
  return l2cacheHitTime + memspeed;
}
