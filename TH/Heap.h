#ifndef HEAP_H
#define HEAP_H

/*
    The source code comes from https://github.com/WavingSketch/Waving-Sketch.
*/

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

class Heap{
public:

    struct Counter{
        uint32_t item;
        int counter;
    };

    Heap(uint32_t _SIZE):SIZE(_SIZE), heap_num(0){
        heaps = new Counter[SIZE];
        memset(heaps, 0, SIZE * sizeof(Counter));
    }

    ~Heap(){
        delete [] heaps;
    }

    void Insert(const uint32_t item, int counter){
        if(mp.find(item) != mp.end()){
            heaps[mp[item]].counter++;
            Heap_Down(mp[item]);
        }
        else if(heap_num < SIZE){
            heaps[heap_num].item = item;
            heaps[heap_num].counter = counter;
            mp[item] = heap_num++;
            Heap_Up(heap_num - 1);
        }
        else if(counter > heaps[0].counter){
            mp.erase(heaps[0].item);
            heaps[0].item = item;
            heaps[0].counter = counter;
            mp[item] = 0;
            Heap_Down(0);
        }
    }

    int Query(const int item){
        return mp.find(item) == mp.end()? 0 : heaps[mp[item]].counter;
    }

private:
    Counter* heaps;
    uint32_t heap_num;
    const uint32_t SIZE;
    unordered_map<uint32_t, int> mp;

    void Heap_Down(uint32_t i){
        while (i < heap_num / 2) {
            uint32_t l_child = 2 * i + 1;
            uint32_t r_child = 2 * i + 2;
            uint32_t larger_one = i;
            if (l_child < heap_num && heaps[l_child].counter < heaps[larger_one].counter) {
                larger_one = l_child;
            }
            if (r_child < heap_num && heaps[r_child].counter < heaps[larger_one].counter) {
                larger_one = r_child;
            }
            if (larger_one != i) {
                swap(heaps[i], heaps[larger_one]);
                swap(mp[heaps[i].item], mp[heaps[larger_one].item]);
                Heap_Down(larger_one);
            } else {
                break;
            }
        }
    }

    void Heap_Up(uint32_t i) {
        while (i > 1) {
            uint32_t parent = (i - 1) / 2;
            if (heaps[parent].counter <= heaps[i].counter) {
                break;
            }
            swap(heaps[i], heaps[parent]);
            swap(mp[heaps[i].item], mp[heaps[parent].item]);
            i = parent;
        }
    }
};

#endif
