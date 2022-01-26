#include <cmath>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <set>
#include <fstream> 
#include <sstream>
#include <unordered_map>
#include <time.h>
#include <vector>
#include "BOBHash32.h"
#include "CM_DUET.h"
#include "CU_DUET.h"
#include "SS_DUET.h"

using namespace std;
#define MAX_INSERT_PACKAGE 10000000

unordered_map<uint32_t, int> ground_truth;
unordered_map<string, int>ground_truth_xy;

uint32_t insert_x[MAX_INSERT_PACKAGE];
uint32_t insert_y[MAX_INSERT_PACKAGE];

string insert_strx[MAX_INSERT_PACKAGE];
string insert_strxy[MAX_INSERT_PACKAGE];


/*################ CAIDA Data ####################*/
int load_data(const char *filename) {
    FILE *pf = fopen(filename, "rb");
    if (!pf) {
        cerr << filename << " not found." << endl;
        exit(-1);
    }

    ground_truth.clear(); ground_truth_xy.clear();
    char ip[13]; int ret = 0;
    while (fread(ip, 1, 13, pf)) {
        uint32_t x = *(uint32_t *) ip; uint32_t y = * ((uint32_t *) ip + 1);  //check this point
        if(x != 0 && y != 0){
            insert_x[ret] = x; insert_y[ret] = y;
            string xy = to_string(x) + "|" + to_string(y);
            //cout<<"xy = " <<xy<<endl;
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
unordered_map <string, int> true_H2;
set <string> tH2;
void report_true(int Xth, double Rth, int Nth){

    ground_truth.clear(); ground_truth_xy.clear();
    packetnum = load_data("../data/125910.dat");

    true_H2.clear();

    for(auto it = ground_truth_xy.begin(); it != ground_truth_xy.end(); it++){
        string str = it -> first;
        int i = 0;
        for(i = 0; i < str.length(); i++){
            if(str[i] == '|')
                break;
        }
        string xstr = str.substr(0, i); string ystr = str.substr(i+1, str.length()); uint32_t x = stoul(xstr);
        int x_freq = ground_truth[x]; int xy_freq = it -> second;
        if (x_freq >=Xth && xy_freq >= Rth * x_freq){ 
            true_H2[str] = xy_freq;
        }
    }
    tH2.clear();
    for(auto it = true_H2.begin(); it != true_H2.end(); it++){
        tH2.insert(it->first);
    }
}

void report_CMDUET(int d, int w, int l, int r, int Xth, double Rth, int Nth, int packetnum,  unordered_map <string, int> true_H2, set <string> tH2){
    clock_t start = clock();
    CM_DUET *sk_h; sk_h = new CM_DUET(d, w, l, r, Xth, Rth, Nth); sk_h -> clear();
    for(int i = 0; i < packetnum; i++){
        sk_h -> insert_scmsketch(insert_x[i], insert_y[i]);
    }
    clock_t end = clock(); 
    sk_h -> query_hottable();
    unordered_map<string, int> est; est = sk_h->get_est_H2(); set <string> eH2_h;
    for(auto it = est.begin(); it != est.end(); it++){
        eH2_h.insert(it -> first);
    }
    set <string> eh_t;
    set_intersection(tH2.begin(), tH2.end(), eH2_h.begin(), eH2_h.end(),inserter( eh_t , eh_t.begin() ));
    double recall = double(eh_t.size()) / double(true_H2.size()); double precision = double(eh_t.size()) / double(est.size());
    double ARE = 0; double AAE = 0;
    for (auto it = est.begin(); it != est.end(); it++){
        string xy = it -> first; int freq = ground_truth_xy[xy];
        ARE += double(abs(it -> second - freq)) / freq; AAE += double(abs(it -> second - freq));
    }
    ARE /= est.size(); AAE /= est.size();
    cout<<fixed<< setprecision(2)<<"["<<double(packetnum) / ((double)(end - start)/ CLOCKS_PER_SEC) / 1000000
    <<", "<<precision<<", "<<recall<<", "<<2*recall*precision/(recall+precision)
    <<", "<<ARE<<", "<<AAE<<"],"<<endl;
}

void report_CUDUET(int d, int w, int l, int r, int Xth, double Rth, int Nth, int packetnum,  unordered_map <string, int> true_H2, set <string> tH2){
    clock_t start = clock();
    CU_DUET *cu; cu = new CU_DUET(d, w, l, r, Xth, Rth, Nth); cu -> clear();
    for(int i = 0; i < packetnum; i++){
        cu -> insert_scusketch(insert_x[i], insert_y[i]);
    }
    clock_t end = clock();  
    cu -> query_hottable();
    unordered_map<string, int> cu_H2; cu_H2 = cu->get_est_H2(); set <string> cuH2;
    for(auto it = cu_H2.begin(); it != cu_H2.end(); it++){
        cuH2.insert(it -> first);
    }
    set <string> u_t;
    set_intersection(tH2.begin(), tH2.end(), cuH2.begin(), cuH2.end(),inserter( u_t , u_t.begin() ));
    double recall = double(u_t.size()) / double(true_H2.size()); double precision = double(u_t.size()) / double(cu_H2.size());
    double ARE = 0; double AAE = 0;
    for (auto it = cu_H2.begin(); it != cu_H2.end(); it++){
        string xy = it -> first; int freq = ground_truth_xy[xy];
        ARE += double(abs(it -> second - freq)) / freq; AAE += double(abs(it -> second - freq));
    }
    ARE /= cu_H2.size(); AAE /= cu_H2.size();
    cout<<fixed<< setprecision(2)<<"["<<double(packetnum) / ((double)(end - start)/ CLOCKS_PER_SEC) / 1000000
    <<", "<<precision<<", "<<recall<<", "<<2*recall*precision/(recall+precision)
    <<", "<<ARE<<", "<<AAE<<"],"<<endl;
}

void report_SSDUET(int M1, int d, int w, int l, int r, int Xth, double Rth, int Nth, int packetnum,  unordered_map <string, int> true_H2, set <string> tH2){
    
    for(int i = 0; i< MAX_INSERT_PACKAGE; i++){
        insert_strx[i] = to_string(insert_x[i]); insert_strxy[i] = to_string(insert_x[i]) + "|" + to_string(insert_y[i]);
    }
    clock_t start = clock();
    SS_DUET *sk; sk = new SS_DUET(M1, d, w, l, r, Xth, Rth, Nth); sk -> clear();
    for(int i = 0; i < packetnum; i++){
        sk -> insert(insert_x[i], insert_y[i], insert_strx[i]);
    }
    clock_t end = clock();  
    sk -> query_hottable();

    unordered_map<string, int> est; est = sk->get_est_H2(); set <string> eH2_h;
    for(auto it = est.begin(); it != est.end(); it++){
        eH2_h.insert(it -> first);
    }
    set <string> eh_t; set_intersection(tH2.begin(), tH2.end(), eH2_h.begin(), eH2_h.end(),inserter( eh_t , eh_t.begin() ));
    double recall = double(eh_t.size()) / double(true_H2.size()); double precision = double(eh_t.size()) / double(est.size());
    double ARE = 0; double AAE = 0;
    for (auto it = est.begin(); it != est.end(); it++){
        string xy = it -> first; int freq = ground_truth_xy[xy];
        ARE += double(abs(it -> second - freq)) / freq; AAE += double(abs(it -> second - freq));
    }
    ARE /= est.size(); AAE /= est.size();
    cout<<fixed<< setprecision(2)<<"["<<double(packetnum) / ((double)(end - start)/ CLOCKS_PER_SEC) / 1000000
    <<", "<<precision<<", "<<recall<<", "<<2*recall*precision/(recall+precision)
    <<", "<<ARE<<", "<<AAE<<"],"<<endl;
}

