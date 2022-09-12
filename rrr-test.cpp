#include <sdsl/bit_vectors.hpp>
#include <iostream>
#include <time.h>
#include <math.h>
#include <cstdlib>

using namespace std;
using namespace sdsl;

int main()
{
    int bsize, m, new_density; double density;
    time_t now = time(0);
    cout << "Enter nr of zeros to bitmap size: " << endl; cin >>  bsize;
    cout << "Enter density: (example: 0.30 = 30%): " << endl; cin >> density;
    cout << "Enter number of measurements: (0<m<=bitmap_size)" << endl; cin >> m;
    bsize = pow(10,bsize);
    bit_vector b(bsize, 0);//bsize bits
    new_density = (1/density);
    for(int i = 0; i < bsize; i++){
       now = rand();
       now = (double)now;
       if(now%new_density==0) b[i] = 1;
    }
    rank_support_v<> rb(&b);

    cout<< "size of b in MB: " << size_in_mega_bytes(b)<< endl;
    cout<< "size of rb in MB: " << size_in_mega_bytes(rb)<< endl;

    rrr_vector<127> rrrb(b);
    rrr_vector<127>::rank_1_type rank_rrrb(&rrrb);
    rrr_vector<127>::select_1_type select_rrrb(&rrrb); //addded
    double rank, rank_time, select_time;
    clock_t start, end;

    start = clock();
    for(int i = 0; i < m; i++){
        rank = rank_rrrb((i/m)*bsize);
        //select[i] = select_rrrb(multiply);
        //select_time[i] = (double)(end-start)/CLOCKS_PER_SEC;    
    }
    end = clock();
    rank_time = (double)(end-start)/CLOCKS_PER_SEC;
    
    
    cout<< "size of rrrb in MB: " << size_in_mega_bytes(
rrrb)<< endl;
    //cout<< "size of rank_rrrb in MB: " << size_in_bytes(rank_rrrb)<< endl;
    cout<<"Rank time: " <<rank_time<<endl; 


    //rrr_vector<127>::select_1_type select_rrrb(&rrrb);
    //cout<<"position of first one in b: "<<select_rrrb(1)<<endl;

}