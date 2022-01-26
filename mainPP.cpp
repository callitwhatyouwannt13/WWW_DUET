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
#include "../PP/BOBHash32.h"
#include "../PP/PP_report.h"

using namespace std;


int main(){

    int d, w, l ,r;
    int Xth, Nth;
    double Rth;

    d = 4;
    l = 200;
    r = 12;
    Xth = 1000;
    Nth = 500;
    Rth = 0.25;
    int wj[1000];
    int lj[11] = {200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200};
    int rj[11] = {4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24};
    cout << "d = "<< d << ", w = "<< w << ", l = "<< l << ", r = " << r << endl;
    cout << "Xth = "<< Xth << ", Rth = "<< Rth << ", Nth = "<< Nth << endl;
    
    int start_mem = 50000; //
    int end_mem = 500000; //
    int step_mem = 50000;

    int j = 0;
    report_true(Xth, Rth);
    cout<<"period num: "<<period_num<<endl;

    cout<<"BF_DUET = ["<<endl;
    int length = 80000; //the length of Bloom Filter
    for(int i = start_mem; i <= end_mem; i += step_mem){
        wj[j] = (i - (lj[j] * rj[j] * 12) - length / 8) / d / 14;
        report_BFDUET(d, wj[j], lj[j], rj[j], Xth, Rth, Nth, length, period_num, true_PP, tp);
        j++;
    }
    cout<<"]"<<endl;

    j = 0;
    cout << "DUET_OO = [" << endl;
    for(int i = start_mem; i <= end_mem; i += step_mem){
        rj[j] = rj[j] * 96 / 97; //use the same memory for elements as BF_DUET.
        wj[j] = (i - (lj[j] * rj[j] * 97 / 8)) / d / (114/8); // (8+4+2)*8 + 2-bit flag  = 114
        report_duet_oo(d, wj[j], lj[j], rj[j], Xth, Rth, Nth, period_num, true_PP, tp);
        j++;
    }
    cout<< "]"<<endl;

    return 0;
}