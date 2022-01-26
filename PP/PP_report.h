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
#include <iomanip>
#include "BOBHash32.h"
#include "BF_DUET.h"
#include "DUET_OO.h"

using namespace std;
#define MAX_INSERT_PACKAGE 10000000
#define MAX_PERIOD 10000
#define period_len 1000

unordered_map<uint32_t, int> ground_truth; 
unordered_map<string, int> ground_truth_xy;

unordered_map<string, int> persistence_x;
unordered_map<string, int> persistence_xy;

uint32_t insert_x[MAX_PERIOD][period_len];
uint32_t insert_y[MAX_PERIOD][period_len];

string insert_strx[MAX_PERIOD][period_len];
string insert_strxy[MAX_PERIOD][period_len];


int ret = 0;
int period_num = 0;
int total = 0;
int load_data(const char *filename) {
    FILE *pf = fopen(filename, "rb");
    if (!pf) {
        cerr << filename << " not found." << endl;
        exit(-1);
    }

    char ip[13];
    while (fread(ip, 1, 13, pf)) {
        uint32_t x = *(uint32_t *) ip;
        uint32_t y = * ((uint32_t *) ip + 1);
        if(x != 0 && y != 0){
            insert_x[period_num][ret] = x;
            insert_y[period_num][ret] = y;
            string xy = to_string(x) + "|" + to_string(y);
            ground_truth_xy[xy]++;
            ground_truth[x]++;
            ret++;
            total++;
            if(ret == period_len){
                for (auto it = ground_truth.begin(); it != ground_truth.end(); it++){
                    string x = to_string(it->first);
                    persistence_x[x] ++ ;
                }
                for (auto it = ground_truth_xy.begin(); it != ground_truth_xy.end(); it++){
                    persistence_xy[it->first] ++; 
                }
                period_num ++;
                ret = 0;
                ground_truth.clear();
                ground_truth_xy.clear();
            }

            if (total == MAX_INSERT_PACKAGE)
                break;
        }
        
    }
    fclose(pf);

    return period_num; //return the number of periods
}


unordered_map <string, int> true_PP;
set <string> tp;
void report_true(int Xth, double Rth){
    period_num = load_data("../data/125910.dat");
    // period_num = load_data("../data/B-130000.dat");

    true_PP.clear();
    tp.clear();
    for(auto it = persistence_xy.begin(); it != persistence_xy.end(); it ++){
        string str = it -> first;
        int i = 0;
        for(i = 0; i < str.length(); i++){
            if(str[i] == '|')
                break;
        }
        string xstr = str.substr(0, i);
        string ystr = str.substr(i+1, str.length());
        int x_pers = persistence_x[xstr];
        int xy_pers = it -> second;
        double ratio = double (xy_pers) / double (x_pers);
        if(x_pers >= Xth && ratio >= Rth){
            true_PP[str] = xy_pers;
            tp.insert(it->first);
        }
    }

    cout<<"the number of true_PP is "<< true_PP.size()<<endl;
    cout<< "----------------------------------------------------------------------------------"<<endl;
    cout<<"Throughput"<<setw(11)<<"Precision"<<setw(9)<<"Recall"<<setw(12)<<"F1 score"<<setw(7)<<"ARE"<<setw(8)<<"AAE"<<setw(12)<<"TP"<<setw(9)<<"Num"<<endl;
    cout<< "----------------------------------------------------------------------------------"<<endl;

}

void report_BFDUET(int d, int w, int l, int r, int Xth, double Rth, int Nth, int length, int T, unordered_map <string, int> true_PP, set <string> tp){
    for(int i = 0; i < T; i++){
        for(int j = 0; j < period_len; j++){
            insert_strx [i][j]= to_string(insert_x [i][j]);
            insert_strxy [i][j] = to_string(insert_x[i][j]) + "|" + to_string(insert_y[i][j]);
        }
    }

    clock_t start = clock();
    BF_DUET *fh;
    fh = new BF_DUET(d, w, l, r, Xth, Rth, Nth, length); 
    fh -> clear();

    for(int i = 0; i < T; i++){
        for(int j = 0; j < period_len; j++){
            fh ->insert(insert_x [i][j], insert_y[i][j], insert_strxy [i][j], i);
        }
    }
    clock_t end = clock();  
    fh -> query_hottable();
    unordered_map<string, int> fh_PP;
    fh_PP = fh->get_est_PP();
    set <string> fp;
    for(auto it = fh_PP.begin(); it != fh_PP.end(); it++){
        fp.insert(it -> first);
    }

    set <string> f_t;
    set_intersection(tp.begin(), tp.end(), fp.begin(), fp.end(),inserter( f_t , f_t.begin() ));
    double recall = double(f_t.size()) / double(true_PP.size());double precision = double(f_t.size()) / double(fh_PP.size());
    double ARE = 0;
    double AAE = 0;
    for (auto it = fh_PP.begin(); it != fh_PP.end(); it++){
        string xy = it ->first;
        int pers = persistence_xy[xy];
        ARE += double(abs(it->second - pers)) / pers;
        AAE += double(abs(it ->second - pers));
    }
    ARE /= fh_PP.size();
    AAE /= fh_PP.size();
    cout<<fixed<< setprecision(2)<<"["<<double(period_num * period_len) / ((double)(end - start)/ CLOCKS_PER_SEC) / 1000000
    <<", "<<precision<<", "<<recall<<", "<<2*recall*precision/(recall+precision)
    <<", "<<ARE<<", "<<AAE<<"],"<<endl;
}

void report_duet_oo(int d, int w, int l, int r, int Xth, double Rth, int Nth, int T, unordered_map <string, int> true_PP, set <string> tp){
    clock_t start = clock();
    DUET_OO *fh;
    fh = new DUET_OO(d, w, l, r, Xth, Rth, Nth);
    fh -> clear();
    for(int i = 0; i < T; i++){
        for(int j = 0; j < period_len; j++){
            fh ->insert_filter(insert_x [i][j], insert_y[i][j], i);
        }
    }
    clock_t end = clock();  
    fh -> query_table();
    unordered_map<string, int> fh_PP;
    fh_PP = fh->get_est_PP();

    set <string> fp;
    for(auto it = fh_PP.begin(); it != fh_PP.end(); it++){
        fp.insert(it -> first);
    }

    set <string> f_t;
    set_intersection(tp.begin(), tp.end(), fp.begin(), fp.end(),inserter( f_t , f_t.begin() ));
    double recall = double(f_t.size()) / double(true_PP.size());double precision = double(f_t.size()) / double(fh_PP.size());
    double ARE = 0;double AAE = 0;
    for (auto it = fh_PP.begin(); it != fh_PP.end(); it++){
        string xy = it ->first;
        int pers = persistence_xy[xy];
        ARE += double(abs(it->second - pers)) / pers;
        AAE += double(abs(it ->second - pers));
    }
    ARE /= fh_PP.size();
    AAE /= fh_PP.size();
    cout<<fixed<< setprecision(2)<<"["<<double(period_num * period_len) / ((double)(end - start)/ CLOCKS_PER_SEC) / 1000000
    <<", "<<precision<<", "<<recall<<", "<<2*recall*precision/(recall+precision)
    <<", "<<ARE<<", "<<AAE<<"],"<<endl;
}
