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
#include "../HH/BOBHash32.h"
#include "../HH/HH_report.h"

using namespace std;

int main(){
    int d, w, l ,r;
    int Xth, Nth;
    double Rth;

    // cin >> d >> w >> l >> r >> Xth >> Rth;
    // cout << "d = "<< d << ", w = "<< w << ", l = "<< l << ", r = " << r << endl;
    // cout << "Xth = "<< Xth << " Rth = "<< Rth << endl;
    
    // for CAIDA
    d = 4;
    l = 200;
    Xth = 4000;
    Nth = 1000;
    Rth = 0.1;
    int lj[11] = {200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200};
    int rj[11] = {8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28};


    int wj[1000];
    
    cout << "d = "<< d << ", w = "<< w << ", l = "<< l << ", r = " << r << endl;
    cout << "Xth = "<< Xth << ", Rth = "<< Rth << ", Nth = "<< Nth << endl;
    int j = 0;
    report_true(Xth, Rth, Nth);

    int start_mem = 50000; //
    int end_mem = 500000; //
    int step_mem = 50000;


    j = 0;
    cout<< "CU_DUET = [" <<endl;
    for(int i = start_mem; i <= end_mem; i += step_mem){
        wj[j] = (i - (lj[j] * rj[j] * 12)) / d / 14;
        // wj[j] = i * 0.6 / d / 14;
        // lj[j] = i * 0.4 / rj[j] / 12;
        report_CUDUET(d, wj[j], lj[j], rj[j], Xth, Rth, Nth, packetnum, true_H2, tH2);
        j++;
    }
    cout<<" ]"<<endl;
    
    return 0;
}