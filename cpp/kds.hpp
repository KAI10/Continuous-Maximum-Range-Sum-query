#include <boost/bimap/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

using namespace boost::bimaps;
using namespace boost;
using namespace std;

typedef bimap< multiset_of<int>, set_of<int> > size_to_index;
typedef size_to_index::value_type size_position;

typedef bimap< multiset_of<double>, set_of<int> > time_to_index;
typedef time_to_index::value_type time_position;

const int parts = 8; //# of smaller kds's
const double delta = 0.25;

struct kds_greedy{

    vector<vector<Event> > kds_data; //container of event data
    multimap<double, int> kds_part[parts]; //keeps track(index in kds data) of events by time

    time_to_index tmap; //keeps track(index) of the kds with earliest event
    size_to_index smap; //keeps track(index) of the smallest kds

    kds_greedy(){
        for(int i=0; i<parts; i++){
            smap.right.insert(make_pair(i, 0)); //initialize smap with size 0
        }
    }

    void set(){}

    void balance(){
        //get the current smallest and largest kds
        int largest_kds_size = (smap.left.rbegin())->first, largest_kds_index = (smap.left.rbegin())->second;
        int smallest_kds_size = (smap.left.begin())->first, smallest_kds_index = (smap.left.begin())->second;


        //cout << "largest_kds_size: " << ' ' << largest_kds_size << " smallest_kds_size: " << ' ' << smallest_kds_size << endl;
        //check if balancing is necessary
        if(1.0*(largest_kds_size - smallest_kds_size)/largest_kds_size < delta) return;

        int transfer_size = (largest_kds_size - smallest_kds_size)/2;
        if(transfer_size == 0) return;

        //transfer from largest to smallest
        for(int i=0; i<transfer_size; i++){
            kds_part[smallest_kds_index].insert(*(prev(kds_part[largest_kds_index].end()))); //taking entries from the back of largest kds
            kds_part[largest_kds_index].erase(prev(kds_part[largest_kds_index].end()));
        }


        bool successful_replace;

        ///update tmap entries
        //smallest kds
        double earliest_time = kds_part[smallest_kds_index].begin()->first;
        auto it = tmap.right.find(smallest_kds_index);
        if(it == tmap.right.end()){
            tmap.right.insert(make_pair(smallest_kds_index, earliest_time));
        }

        else if(it->second > earliest_time){
            successful_replace = tmap.right.replace_data(it, earliest_time);
            assert(successful_replace);
        }

        ///no need to update tmap for largest kds, as entries were taken from the back


        ///update smap entries
        //smallest kds
        auto sit = smap.right.find(smallest_kds_index);
        successful_replace = smap.right.replace_data(sit, smallest_kds_size+transfer_size);
        assert(successful_replace);

        //largest kds
        sit = smap.right.find(largest_kds_index);
        successful_replace = smap.right.replace_data(sit, largest_kds_size-transfer_size);
        assert(successful_replace);

        //cout << "\nAfetr balance...\n";
        //display();
    }

    void insert(Event event){

        //get the current smallest kds
        int smallest_kds_size = (smap.left.begin())->first, smallest_kds_index = (smap.left.begin())->second;

        //cout << "smallest_kds_index: " << ' ' << smallest_kds_index << " smallest_kds_size: " << ' ' << smallest_kds_size << endl;

        //insert the event in kds_data and kds_part
        vector<Event> temp; /// new event time
        temp.push_back(event); /// add event
        kds_data.push_back(temp); ///push in kds data

        int data_index = kds_data.size()-1; //index of new event data
        kds_part[smallest_kds_index].insert(make_pair(event.event_time, data_index)); //insert in kds part

        auto it = tmap.right.find(smallest_kds_index);
        if(it == tmap.right.end()){
            tmap.right.insert(make_pair(smallest_kds_index, event.event_time));
        }
        else if(it->second > event.event_time){
            bool successful_replace = tmap.right.replace_data(it, event.event_time);
            assert(successful_replace);
        }

        //update smap
        auto sit = smap.right.find(smallest_kds_index);
        if(sit == smap.right.end()){
            smap.right.insert(make_pair(smallest_kds_index, 1));
        }
        else{
            bool successful_replace = smap.right.replace_data(sit, smallest_kds_size+1);
            assert(successful_replace);
        }

        balance();

        //cout << "\nAfter insert...\n";
        //display();
    }

    vector<Event> pop(){
        assert(!tmap.left.empty()); ///make sure kds is not empty

        double next_event_time = tmap.left.begin()->first;
        int kds_part_index = tmap.left.begin()->second; //track the event in kds part

        tmap.left.erase(tmap.left.begin()); //erase it from tmap

        int data_index = kds_part[kds_part_index].begin()->second;
        vector<Event> ret = kds_data[data_index]; //saving the event in ret

        kds_part[kds_part_index].erase(kds_part[kds_part_index].begin()); //erase the event from kds part
        
        if(!kds_part[kds_part_index].empty()){
            //add the new earliest event of kds part to tmap
            tmap.left.insert(make_pair(kds_part[kds_part_index].begin()->first, kds_part_index));
        }

        ///update smap entry
        size_to_index::right_iterator sit = smap.right.find(kds_part_index);
        bool successful_replace = smap.right.replace_data(sit, sit->second-1);
        assert(successful_replace);

        balance();

        //cout << "\nAfter pop...\n";
        //display();
        return ret;
    }

    bool empty(){
        return (tmap.left.size() == 0);
    }

    void display(){
        cout << "****************** DISPLAY **************************\n";
        
        //cout << "\nTMAP:\n";
        //for(auto v: tmap.left) cout << "kds index: " << v.second << " time: " << v.first << '\n';

        cout << "\nSMAP:\n";
        for(auto v: smap.left) cout << "kds index: " << v.second << " size: " << v.first << endl;

        /*
        cout << "\nKDS parts:\n";
        for(int i=0; i<parts; i++){
            cout << "kds part " << i << ": ";
            for(auto v: kds_part[i]) cout << v.first << ' ';
            cout << endl;
        }
        */
    }
};


typedef bimap< multiset_of<double>, set_of<pair<int, int>>> time_to_grid_index;

const int xpart = 5, ypart = 5;

struct kds_spatial
{
    vector<vector<Event>> kds_data; //container of event data
    multimap<double, int> kds_part[xpart][ypart]; //keeps track(index in kds data) of events by time

    time_to_grid_index tmap; //keeps track(index) of the kds with earliest event

    double x_range_min, x_range_max, y_range_min, y_range_max, dx, dy;

    kds_spatial(){}

    void set(){
        ///These range values will always be positive
        x_range_min = d_w , x_range_max = d_w + x_max - x_min;
        y_range_min = d_h , y_range_max = d_h + y_max - y_min;

        dx = (x_range_max - x_range_min)/xpart; 
        dy = (y_range_max - y_range_min)/ypart;
    }

    void insert(Event event){
        vector<Event> temp;
        temp.push_back(event);
        kds_data.push_back(temp);
        int data_index = kds_data.size()-1;

        pair<int, int> cell;

        if(event.event_type == NEW_SAMPLE){
            int x = rand() % xpart;
            int y = rand() % ypart;
            cell = make_pair(x, y); ///choose a rndom cell to insert event 

            kds_part[x][y].insert(make_pair(event.event_time, data_index)); //insert in kds part
        }   

        else{

            MovingObject obj1 = saved[event.oid1], obj2 = saved[event.oid2];
            int line_index_1 = object_line_map[event.oid1], line_index_2 = object_line_map[event.oid2];
            Line l1 = current_lines[line_index_1], l2 = current_lines[line_index_2];

            //Is it necessary ??
            //setCurrentLoc(l1, obj1, current_time);
            //setCurrentLoc(l2, obj2, current_time);

            //get position at event time
            pair<double, double> position_1 = getLocation(l1, obj1, event.event_time);
            pair<double, double> position_2 = getLocation(l2, obj2, event.event_time); 

            //cout << "x_range_min: " << x_range_min << " x_range_max: " << x_range_max << endl;
            //cout << "position_1.first: " << position_1.first << " position_1.second: " << position_1.second << endl;

            //check if calculated position is valid
            assert(position_1.first >= x_range_min && position_1.first <= x_range_max);
            assert(position_1.second >= y_range_min && position_1.second <= y_range_max);

            assert(position_2.first >= x_range_min && position_2.first <= x_range_max);
            assert(position_2.second >= y_range_min && position_2.second <= y_range_max);

            ///determine the cells
            int x_1 = floor((position_1.first - x_range_min) / dx),
                y_1 = floor((position_1.second - y_range_min) / dy),
                x_2 = floor((position_2.first - x_range_min) / dx),
                y_2 = floor((position_2.second - y_range_min) / dy);

            //cout << "cells: " << x_1 << ' ' << y_1 << ' ' << x_2 << ' ' << y_2 << endl;
            if(x_1 == xpart) x_1--;
            if(x_2 == xpart) x_2--;
            if(y_1 == ypart) y_1--;
            if(y_2 == ypart) y_2--;

            assert(x_1 < xpart && x_2 < xpart && y_1 < ypart && y_2 < ypart);
            
            //insert in to cell with smaller kds
            if(kds_part[x_1][y_1].size() < kds_part[x_2][y_2].size()){
                cell = make_pair(x_1, y_1);
                kds_part[x_1][y_1].insert(make_pair(event.event_time, data_index));
            }
            else{
                cell = make_pair(x_2, y_2);
                kds_part[x_2][y_2].insert(make_pair(event.event_time, data_index));
            }
        }

        //update tmap
        auto it = tmap.right.find(cell);
        if(it == tmap.right.end()){ //if not in tmap,insert in tmap
            tmap.right.insert(make_pair(cell, event.event_time));
        }
        else if(it->second > event.event_time){ //oherwise update tmap if necessary
            bool successful_replace = tmap.right.replace_data(it, event.event_time);
            assert(successful_replace);
        }

        //display();    
    }


    vector<Event> pop(){
        assert(!tmap.left.empty());

        double next_event_time = tmap.left.begin()->first;
        pair<int, int> cell = tmap.left.begin()->second;

        tmap.left.erase(tmap.left.begin()); //erase it from tmap

        int data_index = kds_part[cell.first][cell.second].begin()->second;
        vector<Event> ret = kds_data[data_index]; //saving the event in ret

        //erase the event from kds part
        kds_part[cell.first][cell.second].erase(kds_part[cell.first][cell.second].begin());
        
        if(!kds_part[cell.first][cell.second].empty()){
            //add the new earliest event of kds part to tmap
            tmap.left.insert(make_pair(kds_part[cell.first][cell.second].begin()->first, cell));
        }

        //cout << "\nAfter pop...\n";
        //display();
        return ret;
    }

    bool empty(){
        return (tmap.left.size() == 0);
    }

    void display(){
        cout << "kds status:\n";
        for(int i=0; i<xpart; i++){
            for(int j=0; j<ypart; j++){
                printf("%6d ", kds_part[i][j].size());
            }
            cout << endl;
        }
    }
};


const double timeINT = 100.0; ///this value is equal to time interval between two consecutive samples
double dt = timeINT/parts;

struct kds_temporal{
    vector<vector<Event>> kds_data;
    multimap<double, int> kds_part[parts];

    int cur_kds_index; ///will be used in pop
    int intervalNumber; ///will be used in insert
    int totalEvents_in_kds;

    kds_temporal(){
        cur_kds_index = 0;
        totalEvents_in_kds = 0;
        intervalNumber = 0;
    }

    void insert(Event event){
        //cout << "intervalNumber: " << intervalNumber << endl;
        //cout << "INSERT\n";
        vector<Event> temp;
        temp.push_back(event);
        kds_data.push_back(temp);
        int data_index = kds_data.size()-1;
        
        if(event.event_type == NEW_SAMPLE){
            //intervalNumber = event.event_time / timeINT - 1;
            //cout << "INSERTING NEW SAMPLE EVENT\n";
            ///add the line change event in the last kds, as this is the last event of this interval
            kds_part[parts-1].insert(make_pair(event.event_time, data_index));
        }
        else{
            //cout << "INSERTING OD/DO event\n";
            double startTime = intervalNumber * timeINT;
            int kds_index = floor((event.event_time - startTime)/dt);
            if(kds_index == parts) kds_index--;

            //cout << "kds_index: " << kds_index << endl;
            kds_part[kds_index].insert(make_pair(event.event_time, data_index));
        }

        totalEvents_in_kds++;
    }

    vector<Event> pop(){
        //cout << "POP\n";
        //cout << "intervalNumber: " << intervalNumber << endl;
        
        assert(!empty());
        while(kds_part[cur_kds_index].empty()) cur_kds_index++; ///if no events in current (small)interval, go to next (small)interval
        
        //cout << "cur_kds_index: " << cur_kds_index << endl;
        
        int data_index = kds_part[cur_kds_index].begin()->second;
        
        //cout << "data_index: " << data_index << endl;
        
        vector<Event> ret = kds_data[data_index];

        kds_part[cur_kds_index].erase(kds_part[cur_kds_index].begin());
        if(kds_part[cur_kds_index].empty()) cur_kds_index++;

        totalEvents_in_kds--;
        if(ret[0].event_type == NEW_SAMPLE){
            intervalNumber++;
            cur_kds_index = 0;
        }
        /*
        if(totalEvents_in_kds == 0){
            intervalNumber++;
            cur_kds_index = 0;
            cout << "intervalNumber: " << intervalNumber << endl;
        }
        */
        ///assert that all events of previous interval are processed
        //for(int i=0; i<parts; i++) assert(kds_part[i].empty());
        return ret; 
    }
    
    void set(){}

    bool empty(){
        return (totalEvents_in_kds == 0);
    }

    void display(){
        for(int i=0; i<parts; i++){
            cout << kds_part[i].size() << ' ';
            cout << endl;
        }  
    }
        
};


