#ifndef _BF_DUET_H
#define _BF_DUET_H

/*BF_DUET*/

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
#include "BloomFilter.h"

using namespace std;

#define SD 10
#define SW 1000000

#define L 10000
#define R 320

#define HASH_NUM 2 //for bloomfilter

typedef unsigned long long int uint64;
typedef unsigned int uint32_t; 

struct bucket{
    int V; 
    uint32_t x;
    uint32_t y;
    int count;
};

struct cell{
    uint32_t cx;
    uint32_t cy;
    int cnt;
};



class BF_DUET{
    private:

        
        int d, w, l, r, Xth;
        double Rth;
        int Nth;
        int length; 
        int current_period;
        BOBHash32 * hash[SD];
        BOBHash32 * hash_bf[HASH_NUM];
        BOBHash32 hashx;  
        int linenum[L];
        unordered_map <string, int> est_PP;
        BloomFilter* bitset; 
        bucket B[SD][SW];
        cell C[L][R];

    public:
        BF_DUET(int d, int w, int l, int r, int Xth, double Rth, int Nth, int length): d(d), w(w), l(l), r(r), Xth(Xth), Rth(Rth), Nth(Nth), length(length) { //#############
            
            current_period = 0;
            for (int i = 0; i < d; i++)
                hash[i] = new BOBHash32(i + 750);
            for (int i = 0; i < HASH_NUM; i++)
                hash_bf[i] = new BOBHash32(i + 750);
            hashx = BOBHash32(750);
            for (int j = 0; j < l ; j++)
                linenum[j] = 0;
            
            est_PP.clear();
            bitset = new BloomFilter(length);
        }
        
        void clear(){
            for (int i = 0; i < d; i++){
                for (int j = 0; j < w+5; j++){
                    B[i][j].V = 0;
                    B[i][j].x = 0;
                    B[i][j].y = 0;
                    B[i][j].count = 0;
                    
                }
            }
            for(int i = 0; i < l; i++){
                for (int j = 0; j < r+5; j++){
                    C[i][j].cx = 0;
                    C[i][j].cy = 0;
                    C[i][j].cnt = 0;
                }
            }
        }

        void insert(uint32_t x, uint32_t y, string xy, int period){
            if(period > current_period){
                current_period = period;
                bitset -> Clear();
            }

            bool flag_xy = true;
            bool flag_x = true;
            for(int i = 0; i < HASH_NUM; i++){
                uint p_xy = (hash_bf[i]->run(xy.c_str(), xy.size())) % length;
                uint p_x = (hash_bf[i]->run((const char *)&y, 4)) % length;
                if(!bitset ->Get(p_xy)){
                    flag_xy = false;
                    bitset->Set(p_xy);
                }
                if(!bitset ->Get(p_x)){
                    flag_x = false;
                    bitset->Set(p_x);
                }
            }

            if(!flag_xy){
                insert_scmsketch(x, y, flag_x);
            }
            
        }


        void insert_scmsketch(uint32_t x, uint32_t y, int flag){
            int pos[d];
            int min_x = INT_MAX;
            for (int i = 0; i < d; i++){
                pos[i] = (hash[i]->run((const char *)&x, 4)) % w;
                if (B[i][pos[i]].V < min_x){
                    min_x = B[i][pos[i]].V;
                }
            }

            if (min_x >= Nth){
                //CU update
                min_x ++;
                if(!flag){
                    for (int i = 0; i < d; i++){
                        B[i][pos[i]].V = max(B[i][pos[i]].V, min_x);
                    }
                }
                insert_hottable(x, y, 1);
            }
            else{
                min_x ++;
                if(!flag){
                    for (int i = 0; i < d; i++){
                        B[i][pos[i]].V = max(B[i][pos[i]].V, min_x);
                    }
                }
                int posy = (hash[0]->run((const char *)&y, 4)) % d; 
                if (B[posy][pos[posy]].count == 0){
                    B[posy][pos[posy]].count = 1;
                    B[posy][pos[posy]].x = x;
                    B[posy][pos[posy]].y = y;
                }
                else if (B[posy][pos[posy]].x == x &&  B[posy][pos[posy]].y == y){
                    B[posy][pos[posy]].count++;
                }
                else{
                    B[posy][pos[posy]].count--;
                    if (B[posy][pos[posy]].count == 0){
                        B[posy][pos[posy]].count = 1;
                        B[posy][pos[posy]].x = x;
                        B[posy][pos[posy]].y = y;
                    }
                }

                if (min_x >= Nth){
                    for (int i = 0; i < d ; i++){
                        if (B[i][pos[i]].x == x){
                            insert_hottable(x, B[i][pos[i]].y, B[i][pos[i]].count);
                            B[i][pos[i]].x = 0;
                            B[i][pos[i]].y = 0;
                            B[i][pos[i]].count = 0;
                        }
                    }
                }
            }  
        }

        void insert_hottable(uint32_t x, uint32_t y, int f){
            int pos = (hashx.run((const char *)&x, 4)) % l;
            int flag_xy = 0;
            for (int i = 0; i < r; i++){
                if (C[pos][i].cx == x && C[pos][i].cy == y){
                    flag_xy = 1;
                    C[pos][i].cnt = C[pos][i].cnt + f;
                }
            }

            if (flag_xy == 0){
                if (linenum[pos] < r){
                    C[pos][linenum[pos]].cx = x;
                    C[pos][linenum[pos]].cy = y;
                    C[pos][linenum[pos]].cnt = f;
                    linenum[pos]++;
                }

                else{
                    int min_cnt = INT_MAX;
                    int min_pos = 0;
                    for (int i = 0; i < r; i++){
                        if (C[pos][i].cnt < min_cnt){
                            min_cnt = C[pos][i].cnt;
                            min_pos = i;
                        }
                    }
            
                    C[pos][min_pos].cnt = C[pos][min_pos].cnt - f;
                    if (C[pos][min_pos].cnt < 0){
                        C[pos][min_pos].cx = x;
                        C[pos][min_pos].cy = y;
                        C[pos][min_pos].cnt = -C[pos][min_pos].cnt;
                    }
                }
            }
        }

        void query_hottable(){
            for(int i = 0; i < l; i++){
                for(int j = 0; j < r; j++){
                    int x_freq = query_scmsketch(C[i][j].cx);
                    int xy_freq = C[i][j].cnt;
                    if (x_freq >= Xth && xy_freq >= Rth * x_freq){
                        string xy = to_string(C[i][j].cx) + "|" + to_string(C[i][j].cy);
                        est_PP[xy] = C[i][j].cnt;
                    }
                }
            }
        }


        int query_scmsketch(uint32_t x){
            int pos[d];
            int min_x = INT_MAX;
            for (int i = 0; i < d; i++){
                pos[i] = (hash[i]->run((const char *)&x, 4)) % w;
                if (B[i][pos[i]].V < min_x){
                    min_x = B[i][pos[i]].V;
                }
            }
            return min_x;
        }
        unordered_map<string, int> get_est_PP(){
            return est_PP;
        }
        
};

#endif