#ifndef _BLOOMFILTER_H
#define _BLOOMFILTER_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <unordered_map>
#include "limits.h" //INT_MAX
#include "BOBHash32.h"

using namespace std;

typedef unsigned int uint;
typedef unsigned char uchar;

class BloomFilter
{
public:
    BloomFilter(uint _LENGTH):LENGTH(_LENGTH){
        uint size = (LENGTH >> 3) + 1;
        bitset = new uchar[size];
        memset(bitset, 0, size * sizeof(uchar));
    }
    ~BloomFilter(){
        delete [] bitset;
    }
    void Set(uint index){
        uint position = (index >> 3);
        uint offset = (index & 0x7);
        bitset[position] |= (1 << offset);
    } //Set the index bit to 1
    void Clear(){
        uint size = (LENGTH >> 3) + 1;
        memset(bitset, 0, size * sizeof(uchar));
    } //Set all bit to 0
    void Clear(uint index){
        uint position = (index >> 3);
        uint offset = (index & 0x7);
        uchar mask = 0xff;
        mask ^= (1 << offset);
        bitset[position] &= mask;
    } //Set the index bit to 0
    bool Get(uint index){
        uint position = (index >> 3);
        uint offset = (index & 0x7);
        if((bitset[position] & (1 << offset)) != 0)
            return true;
        return false;
    } //return the index bit

private:
    const uint LENGTH;
    uchar* bitset;
};

#endif // BITSET_H
