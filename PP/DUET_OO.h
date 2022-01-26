#ifndef _DUET_OO_H
#define _DUET_OO_H

/*Our algorithm Flag_DUET*/

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

#define OD 10
#define OW 1000000

#define OL 10000
#define OR 320

typedef unsigned long long int uint64;
typedef unsigned int uint32_t; 

// we set the size of both the sketch and the filter as d * w
struct bucket_o{
    int flag_V;
    int V; 
    int flag_p;
    uint32_t x;
    uint32_t y;
    int count;
};

struct cell_o{
    int flag;
    uint32_t cx;
    uint32_t cy;
    int cnt;
};

class DUET_OO{
    private:
        int d, w, l, r, Xth;
        double Rth;
        int Nth;
        int current_period;
        BOBHash32 * hash[OD];

        BOBHash32 hashx;
        int linenum[OL];
        unordered_map <string, int> est_PP;
        BloomFilter* bitset; 
        bucket_o B[OD][OW];
        cell_o C[OL][OR];
    public:
        DUET_OO(int d, int w, int l, int r, int Xth, double Rth, int Nth): d(d), w(w), l(l), r(r), Xth(Xth), Rth(Rth), Nth(Nth){
            current_period = 0;
            for (int i = 0; i < d; i++)
                hash[i] = new BOBHash32(i + 750);
            hashx = BOBHash32(750);
            for (int j = 0; j < l ; j++)
                linenum[j] = 0;
            est_PP.clear();
        } 
        void clear(){
            for (int i = 0; i < d; i++){
                for (int j = 0; j < w+5; j++){
                    B[i][j].flag_V = 0;
                    B[i][j].V = 0;
                    B[i][j].flag_p = 0;
                    B[i][j].x = 0;
                    B[i][j].y = 0;
                    B[i][j].count = 0;
                    
                }
            }
            for(int i = 0; i < l; i++){
                for (int j = 0; j < r+5; j++){
                    C[i][j].flag = 0;
                    C[i][j].cx = 0;
                    C[i][j].cy = 0;
                    C[i][j].cnt = 0;
                }
            }
        }

        void clear_flag(){
           for (int i = 0; i < d; i++){
                for (int j = 0; j < w+5; j++){
                    B[i][j].flag_V = 0;
                    B[i][j].flag_p = 0;
                } 
           }
           for(int i = 0; i < l; i++){
                for (int j = 0; j < r+5; j++){
                    C[i][j].flag = 0;
                }
            }
        }

        void insert_filter(uint32_t x, uint32_t y, int period){
            if(period > current_period){
                clear_flag();
                current_period = period;
            }
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
                for (int i = 0; i < d; i++){
                    if(B[i][pos[i]].flag_V == 0){
                        B[i][pos[i]].V = max(B[i][pos[i]].V, min_x);
                        B[i][pos[i]].flag_V = 1;
                    }
                }
                insert_table(x, y, 1);
            }
            else{
                min_x ++;
                for (int i = 0; i < d; i++){
                    if(B[i][pos[i]].flag_V == 0){
                        B[i][pos[i]].V = max(B[i][pos[i]].V, min_x);
                        B[i][pos[i]].flag_V = 1;
                    }
                } 

                int posy = (hash[0]->run((const char *)&y, 4)) % d; 
                if(B[posy][pos[posy]].flag_p == 0){
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
                    B[posy][pos[posy]].flag_p = 1;
                }

                if (min_x >= Nth){
                    for (int i = 0; i < d ; i++){
                        if (B[i][pos[i]].x == x){
                            insert_table(x, B[i][pos[i]].y, B[i][pos[i]].count);
                            B[i][pos[i]].flag_p = 0;
                            B[i][pos[i]].x = 0;
                            B[i][pos[i]].y = 0;
                            B[i][pos[i]].count = 0;
                        }
                    }
                }
            }

        }

        void insert_table(uint32_t x, uint32_t y, int f){
            int pos = (hashx.run((const char *)&x, 4)) % l;
            int flag_xy = 0;
            for (int i = 0; i < r; i++){
                if (C[pos][i].cx == x && C[pos][i].cy == y){
                    flag_xy = 1;
                    if(C[pos][i].flag == 0){
                        C[pos][i].cnt = C[pos][i].cnt + f;
                        C[pos][i].flag = 1;
                    } 
                } 
            }
            if (flag_xy == 0){
                if (linenum[pos] < r){
                    C[pos][linenum[pos]].cx = x;
                    C[pos][linenum[pos]].cy = y;
                    C[pos][linenum[pos]].cnt = f;
                    C[pos][linenum[pos]].flag = 1;
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
                    if(C[pos][min_pos].flag == 0){
                        C[pos][min_pos].cnt = C[pos][min_pos].cnt - f;
                        if (C[pos][min_pos].cnt < 0){
                            C[pos][min_pos].flag = 1;
                            C[pos][min_pos].cx = x;
                            C[pos][min_pos].cy = y;
                            C[pos][min_pos].cnt = -C[pos][min_pos].cnt; 
                        }
                    }
                }
            }
        }

        void query_table(){
            for(int i = 0; i < l; i++){
                for(int j = 0; j < r; j++){
                    int x_freq = query_filter(C[i][j].cx);
                    int xy_freq = C[i][j].cnt;
                    if (x_freq >= Xth && xy_freq >= Rth * x_freq){
                        string xy = to_string(C[i][j].cx) + "|" + to_string(C[i][j].cy);
                        est_PP[xy] = C[i][j].cnt;
                    }
                }
            }
        }

        int query_filter(uint32_t x){
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