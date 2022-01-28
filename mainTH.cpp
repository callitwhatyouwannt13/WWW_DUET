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
#include "../TH/BOBHash32.h"
#include "../TH/TH_report.h"

using namespace std;

int main(){
    int d, w, l ,r;
    int K, Nth;
    double Rth;

    K = 1000;
    d = 4;
    Nth = 1000;
    Rth = 0.1;
    int wj[1000];
    int lj[11] = {200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200};
    int rj[11] = {8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28};

    cout << "d = "<< d << ", w = "<< w << ", l = "<< l << ", r = " << r << endl;
    cout << "Nth = "<< Nth << ", Rth = "<< Rth << ", K = "<< K << endl;
    int j = 0;
    report_true(Nth, Rth, K);

    int start_mem = 50000; //
    int end_mem = 500000; //
    int step_mem = 50000;
    

    cout<< "CUH_DUET = [" <<endl;
    j = 0;
    for(int i = start_mem; i <= end_mem; i += step_mem){
        wj[j] = (i - (lj[j] * rj[j] * 12)- K * 12) / d / 14;
        report_CUHDUET(d, wj[j], lj[j], rj[j], Nth, Rth, K, packetnum, true_TH, th);
        j++;
    }
    cout<<"]"<<endl;
}