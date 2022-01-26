#include <cmath>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <map>
#include <set>
#include <fstream> 
#include <unordered_map>
#include <time.h>
#include "BOBHash32.h"
#include "SS_DUET.h"
#include "CUH_DUET.h"

using namespace std;
#define MAX_INSERT_PACKAGE 10000000

unordered_map<uint32_t, int> ground_truth;
unordered_map<string, int>ground_truth_xy;

uint32_t insert_x[MAX_INSERT_PACKAGE];
uint32_t insert_y[MAX_INSERT_PACKAGE];
string insert_strx[MAX_INSERT_PACKAGE];
string insert_strxy[MAX_INSERT_PACKAGE];

struct node {uint32_t x;int count;} n[10000005];
int cmp(node i,node j) {return i.count > j.count;}

int load_data(const char *filename) {
    FILE *pf = fopen(filename, "rb");
    if (!pf) {
        cerr << filename << " not found." << endl;
        exit(-1);
    }
    ground_truth.clear();ground_truth_xy.clear();
    char ip[13]; int ret = 0;
    while (fread(ip, 1, 13, pf)) {
        uint32_t x = *(uint32_t *) ip; uint32_t y = * ((uint32_t *) ip + 1);  //check this point
        if(x != 0 && y != 0){
            insert_x[ret] = x; insert_y[ret] = y;
            string xy = to_string(x) + "|" + to_string(y);
            ground_truth_xy[xy]++; ground_truth[x]++;
            ret++;
            if (ret == MAX_INSERT_PACKAGE)
                break;
        }
        
    }
    fclose(pf);
    return ret;
}

int packetnum;

unordered_map <string, int> true_TH; set <string> th;
void report_true(int Nth, double Rth, int K){
    ground_truth.clear(); ground_truth_xy.clear();
    packetnum = load_data("../data/125910.dat");

    int cnt = 0;
    for(auto it = ground_truth.begin(); it != ground_truth.end(); it++){
        n[cnt].x = it -> first; n[cnt].count = it -> second; cnt ++;
    }
    sort(n,n+cnt,cmp);
    
    unordered_map <uint32_t, int> top_x;
    for(int i = 0; i < K; i++){
        top_x[n[i].x] = n[i].count;
    }
    
    true_TH.clear();
    for(auto it = ground_truth_xy.begin(); it != ground_truth_xy.end(); it++){
        int xy_freq = it -> second; string str = it -> first;
        int i = 0;
        for(i = 0; i < str.length(); i++){
            if(str[i] == '|')
                break;
        }
        string xstr = str.substr(0, i);
        uint32_t x = stoul(xstr);
        int x_freq = ground_truth[x]; double ratio = double(xy_freq)/double(x_freq);
        if (top_x.find(x) != top_x.end() &&  ratio >= Rth){
            true_TH[str] = xy_freq;
        }
    }
    th.clear();
    for(auto it = true_TH.begin(); it != true_TH.end(); it++){
        th.insert(it->first);
    }
}

void report_CUHDUET(int d, int w, int l, int r, int Nth, double Rth, int K, int packetnum,  unordered_map <string, int> true_TH, set <string> th){
    clock_t start = clock();
    CUH_DUET *fh;
    fh = new CUH_DUET(d, w, l, r, Nth, Rth, K); fh -> clear();
    for(int i = 0; i < packetnum; i++){
        fh -> insert_filter(insert_x[i], insert_y[i]);
    }
    clock_t end = clock();
    fh -> query_hottable();
    unordered_map<string, int> fh_TH;
    fh_TH = fh->get_est_TH();

    set <string> fhTH;
    for(auto it = fh_TH.begin(); it != fh_TH.end(); it++){
        fhTH.insert(it -> first);
    }
    set <string> fh_t; 
    set_intersection(th.begin(), th.end(), fhTH.begin(), fhTH.end(),inserter( fh_t , fh_t.begin() ));
    double recall = double(fh_t.size()) / double(true_TH.size()); double precision = double(fh_t.size()) / double(fh_TH.size());
    double ARE = 0;double AAE = 0;
    for (auto it = fh_TH.begin(); it != fh_TH.end(); it++){
        string xy = it -> first; int freq = ground_truth_xy[xy];
        ARE += double(abs(it -> second - freq)) / freq; AAE += double(abs(it -> second - freq));
    }
    ARE /= fh_TH.size();
    AAE /= fh_TH.size();
    cout<<fixed<< setprecision(2)<<"["<<double(packetnum) / ((double)(end - start)/ CLOCKS_PER_SEC) / 1000000
    <<", "<<precision<<", "<<recall<<", "<<2*recall*precision/(recall+precision)
    <<", "<<ARE<<", "<<AAE<<"], "<<endl;
}

void report_SSDUET(int M1, int d, int w, int l, int r, int Nth, double Rth, int K, int packetnum,  unordered_map <string, int> true_TH, set <string> th){
    for(int i = 0; i< MAX_INSERT_PACKAGE; i++){
        insert_strx[i] = to_string(insert_x[i]); insert_strxy[i] = to_string(insert_x[i]) + "|" + to_string(insert_y[i]);
    }

    clock_t start = clock();
    SS_DUET *fh;
    fh = new SS_DUET(M1+K, d, w, l, r, Nth, Rth, K); fh -> clear();
    for(int i = 0; i < packetnum; i++){
        fh -> insert(insert_x[i], insert_y[i], insert_strx[i]);
    }
    clock_t end = clock();
    fh -> query_hottable();
    unordered_map<string, int> fh_TH; fh_TH = fh->get_est_TH();

    set <string> fhTH;
    for(auto it = fh_TH.begin(); it != fh_TH.end(); it++){
        fhTH.insert(it -> first);
    }
    set <string> fh_t; 
    set_intersection(th.begin(), th.end(), fhTH.begin(), fhTH.end(),inserter( fh_t , fh_t.begin() ));
    double recall = double(fh_t.size()) / double(true_TH.size()); double precision = double(fh_t.size()) / double(fh_TH.size());
    double ARE = 0; double AAE = 0;
    for (auto it = fh_TH.begin(); it != fh_TH.end(); it++){
        string xy = it -> first; int freq = ground_truth_xy[xy];
        ARE += double(abs(it  -> second - freq)) / freq; AAE += double(abs(it -> second - freq));
    }
    ARE /= fh_TH.size(); AAE /= fh_TH.size();
    cout<<fixed<< setprecision(2)<<"["<<double(packetnum) / ((double)(end - start)/ CLOCKS_PER_SEC) / 1000000
    <<", "<<precision<<", "<<recall<<", "<<2*recall*precision/(recall+precision)
    <<", "<<ARE<<", "<<AAE<<"], "<<endl;
}


