#ifndef _CU_DUET_H
#define _CU_DUET_H

/*Our algorithm CU_DUET*/

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

#define UD 10
#define UW 1000000

#define UL 10000
#define UR 320

typedef unsigned long long int uint64;
typedef unsigned int uint32_t; 


// set both CU sketch and the filter as d * w two-dimensional arrays 
struct Ubucket{
    int V; 
    uint32_t x;
    uint32_t y;
    int count;
};

struct Ucell{
    uint32_t cx;
    uint32_t cy;
    int cnt;
};

class CU_DUET{
    private:
        int d, w, l, r, Xth;
        double Rth;
        int Nth;

        BOBHash32 * hash[UD];
        BOBHash32 hashx;  
        int linenum[UL];
        unordered_map <string, int> est_H2;
        unordered_map <string, int> cellxy;
        Ubucket B[UD][UW];
        Ucell C[UL][UR]; 

    public:
        CU_DUET(int d, int w, int l, int r, int Xth, double Rth, int Nth): d(d), w(w), l(l), r(r), Xth(Xth), Rth(Rth), Nth(Nth){
            for (int i = 0; i < d; i++)
                hash[i] = new BOBHash32(i + 750);

            hashx = BOBHash32(750);
        
            for (int j = 0; j < l ; j++)
                linenum[j] = 0;
            
            est_H2.clear();
            cellxy.clear();
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

        /*
        CU sketch for recording item x here.
        Options: replace this part with CM sketch or some other sketch to record items. 
        */
        void insert_scusketch(uint32_t x, uint32_t y){
            int pos[d];
            int min_x = INT_MAX;
            for (int i = 0; i < d; i++){
                pos[i] = (hash[i]->run((const char *)&x, 4)) % w;
                if (B[i][pos[i]].V <= min_x){
                    min_x = B[i][pos[i]].V;
                }
            }

            if (min_x >= Nth){
                min_x ++;
                for (int i = 0; i < d; i++){
                    B[i][pos[i]].V = max(B[i][pos[i]].V, min_x);
                
                }
                insert_hottable(x, y, 1);
            }

            else{
                min_x ++;
                for (int i = 0; i < d; i++){
                    B[i][pos[i]].V = max(B[i][pos[i]].V, min_x);
                
                }
                int posy = (hash[0]->run((const char *)&y, 4)) % d; 
                if (B[posy][pos[posy]].count == 0){
                    B[posy][pos[posy]].count = 1; B[posy][pos[posy]].x = x; B[posy][pos[posy]].y = y;
                }
                else if (B[posy][pos[posy]].x == x &&  B[posy][pos[posy]].y == y){
                    B[posy][pos[posy]].count++;
                }
                else{
                    B[posy][pos[posy]].count--;
                    if (B[posy][pos[posy]].count == 0){
                        B[posy][pos[posy]].count = 1; B[posy][pos[posy]].x = x; B[posy][pos[posy]].y = y;
                    }
                }

                if (min_x >= Nth){
                    for (int i = 0; i < d ; i++){
                        if (B[i][pos[i]].x == x){ 
                            insert_hottable(x, B[i][pos[i]].y, B[i][pos[i]].count);
                            B[i][pos[i]].x = 0; B[i][pos[i]].y = 0; B[i][pos[i]].count = 0;
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
                    C[pos][linenum[pos]].cx = x; C[pos][linenum[pos]].cy = y; C[pos][linenum[pos]].cnt = f; linenum[pos]++;
                }

                else{
                    int min_cnt = INT_MAX; int min_pos = 0;
                    for (int i = 0; i < r; i++){
                        if (C[pos][i].cnt < min_cnt){
                            min_cnt = C[pos][i].cnt;
                            min_pos = i;
                        }
                    }
                    C[pos][min_pos].cnt = C[pos][min_pos].cnt - f;
                    if (C[pos][min_pos].cnt < 0){
                        C[pos][min_pos].cx = x; C[pos][min_pos].cy = y;C[pos][min_pos].cnt = -C[pos][min_pos].cnt;
                    }
                }
            }
        }

        int query_scusketch(uint32_t x){
            int pos[d];
            int min_x = INT_MAX;
            for (int i = 0; i < d; i++){
                pos[i] = (hash[i]->run((const char *)&x, 4)) % w;
                if (B[i][pos[i]].V <= min_x){
                    min_x = B[i][pos[i]].V;
                }
            }
            return min_x;
        }

        int query_filter(uint32_t x, uint32_t y){
            int pos[d];
            int min_x = INT_MAX;
            for (int i = 0; i < d; i++){
                pos[i] = (hash[i]->run((const char *)&x, 4)) % w;
                if (B[i][pos[i]].x == x && B[i][pos[i]].y == y){
                    return  B[i][pos[i]].count;
                }
            }
            return 0;
        }

        void query_hottable(){
            for(int i = 0; i < l; i++){
                for(int j = 0; j < r; j++){
                    int x_freq = query_scusketch(C[i][j].cx); int xy_freq = C[i][j].cnt; 
                    if (x_freq >= Xth && xy_freq >= Rth * x_freq){
                        string xy = to_string(C[i][j].cx) + "|" + to_string(C[i][j].cy);
                        est_H2[xy] = C[i][j].cnt;
                    }
                }
            }
        }

        Ubucket query_bucket(int index1, int index2){
            return B[index1][index2];
        }
        Ucell query_cell(int index1, int index2){
            return C[index1][index2];
        }
        unordered_map<string, int> get_est_H2(){
            return est_H2;
        }

};

#endif