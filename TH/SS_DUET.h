#ifndef _SS_DUET_H
#define _SS_DUET_H

/*Our algorithm SS_DUET*/

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
#include "ssummary.h"


using namespace std;
#define MAXMEM 1000000
#define DD 10
#define DW 1000000

#define DL 10000
#define DR 320

typedef unsigned long long int uint64;
typedef unsigned int uint32_t; 

//for dual filter
struct sbucket{
    // int V; 
    uint32_t x;
    uint32_t y;
    int count;
};

struct scell{
    uint32_t x;
    uint32_t y;
    int cnt;
};

class SS_DUET{
    private:
        int M1, d, w, l, r;
        int Nth;
        double Rth; 
        int K;
        ssummary *ss;
        BOBHash32 * hash[DD];
        BOBHash32 hashx;  
        int linenum[DL];
        unordered_map <string, int> est_H2;
        unordered_map <string, int> Top_x;
        sbucket B[DD][DW];
        scell C[DL][DR];
        
    public:
        SS_DUET(int M1, int d, int w, int l, int r, int Nth, double Rth, int K): M1(M1), d(d), w(w), l(l), r(r), Nth(Nth), Rth(Rth), K(K) {
            
            ss = new ssummary(M1);
            ss->clear();

            for (int i = 0; i < d; i++)
                hash[i] = new BOBHash32(i + 750);

            hashx = BOBHash32(750);
            
            
            for (int j = 0; j < l ; j++)
                linenum[j] = 0;
            
            est_H2.clear();
            Top_x.clear();
        }

        void clear(){
            for (int i = 0; i < d; i++){
                for (int j = 0; j < w+5; j++){
                    B[i][j].x = 0;
                    B[i][j].y = 0;
                    B[i][j].count = 0;
                    
                }
            }
            for(int i = 0; i < l; i++){
                for (int j = 0; j < r+5; j++){
                    C[i][j].x = 0;
                    C[i][j].y = 0;
                    C[i][j].cnt = 0;
                }
            }
            
        }

        void insert(uint32_t x, uint32_t y, string strx){
            int freq = Insert_x(strx);
            
            if (freq < Nth){
                int posy = (hash[0]->run((const char *)&y, 4)) % d; 
                int posx = (hash[posy]->run((const char *)&x, 4)) % w;
                if (B[posy][posx].count == 0){
                    B[posy][posx].count = 1;
                    B[posy][posx].x = x;
                    B[posy][posx].y = y;
                }
                else if (B[posy][posx].x == x &&  B[posy][posx].y == y){
                    B[posy][posx].count++;
                }
                else{
                    B[posy][posx].count--;
                    if (B[posy][posx].count == 0){
                        B[posy][posx].count = 1;
                        B[posy][posx].x = x;
                        B[posy][posx].y = y;
                    }
                }
            }

            else if (freq == Nth){
                insert_hottable(x, y, 1);
                for (int i = 0; i < d ; i++){
                    int pos = (hash[i]->run((const char *)&x, 4)) % w;
                    if (B[i][pos].x == x){
                        insert_hottable(x, B[i][pos].y, B[i][pos].count); 
                        B[i][pos].x = 0;
                        B[i][pos].y = 0;
                        B[i][pos].count = 0;
                    }
                }
            }

            else{
                insert_hottable(x, y, 1);
            }
        }


        int Insert_x(string x){
            bool mon=false;
            int p=ss->find(x);
            if (p) mon=true;
            if (!mon)
            {
                int q;
                if (ss->tot<M1) 
                    q=1; 
                else
                    q=ss->getmin()+1;

                int i=ss->getid();
                ss->add2(ss->location(x),i);
                ss->str[i]=x;
                ss->sum[i]=q;
                ss->link(i,0);
                while(ss->tot>M1)
                {
                    int t=ss->Right[0];
                    int tmp=ss->head[t];
                    ss->cut(ss->head[t]);
                    ss->recycling(tmp);
                }
            } 
            else
            {
                int tmp=ss->Left[ss->sum[p]];
                ss->cut(p);
                if(ss->head[ss->sum[p]]) tmp=ss->sum[p];
                ss->sum[p]++;
                ss->link(p,tmp);
            }
            return ss->sum[p];
        }

        void insert_hottable(uint32_t x, uint32_t y, int f){
            int pos = (hashx.run((const char *)&x, 4)) % l;
            int flag_xy = 0;
            for (int i = 0; i < r; i++){
                if (C[pos][i].x == x && C[pos][i].y == y){
                    flag_xy = 1;
                    C[pos][i].cnt = C[pos][i].cnt + f;
                }
            }

            if (flag_xy == 0){
                if (linenum[pos] < r){
                    C[pos][linenum[pos]].x = x;
                    C[pos][linenum[pos]].y = y;
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
                        C[pos][min_pos].x = x;
                        C[pos][min_pos].y = y;
                        C[pos][min_pos].cnt = -C[pos][min_pos].cnt; 
                    }
                }
            }
        }


       

        int query_x(uint32_t x){
            string strx = to_string(x);
            bool mon=false;
            int p=ss->find(strx);
            if (p){
                return ss->sum[p];
            }
            return 0;
        }

        struct Node {string x; int y;} q[MAXMEM+10];
        static int cmp(Node i,Node j) {return i.y>j.y;}
        void work(){
            int CNT=0;
            for(int i=N;i;i=ss->Left[i])
                for(int j=ss->head[i];j;j=ss->Next[j]) {q[CNT].x=ss->str[j]; q[CNT].y=ss->sum[j]; CNT++; }
            sort(q,q+CNT,cmp);
            for(int i = 0; i < K; i++){
                Top_x[q[i].x] = q[i].y;
            }
        }

        unordered_map <string, int> get_Topx(){
            work();
            return Top_x;
        }
        
        void query_hottable(){
            work();
            for(int i = 0; i < l; i++){
                for(int j = 0; j < r; j++){
                    string x = to_string(C[i][j].x);
                    int x_freq = Top_x[x];
                    int xy_freq = C[i][j].cnt;
                    if (x_freq != 0 && xy_freq >= Rth * x_freq){
                        string xy = to_string(C[i][j].x) + "|" + to_string(C[i][j].y);
                        est_H2[xy] = C[i][j].cnt;
                    }
                }
            }
        }

        unordered_map<string, int> get_est_TH(){
            return est_H2;
        }

};

#endif