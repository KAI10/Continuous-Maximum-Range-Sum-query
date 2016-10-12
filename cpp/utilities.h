/*
 * utilities.h
 *
 * necessary functions
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-10-08
 */

 using namespace std;
 #include <iostream>
 #include <cstdio>
 #include <vector>
 #include <cstring>
 #include <map>

const int STEP = 1;
const double r_w = d1;
const double r_h = d2;
const double a_w = 100;
const double a_h = 100;

const double DINF = 100000000.0;
const double SAFETY = 0.005;
const int IINF = 100000000;

Area coverage(r_h, r_w);
Area area(a_h, a_w);

 double x_max, x_min, y_max, y_min,
        d_w, /// r_w/2
        d_h; /// r_h/2

map<int, int> real_id_to_object_id;
map<int, int>:: iterator it;

int readFromMNGT(char* fileName, int num, vector<DataPoint>& datapoints)
{
    string baseFileName = fileName;

    int nextID = 0;
    FILE *fp;

    int participant_id, trip_id;
    double datatime, latitude, longitude, act_speed;
    char str[105];

    for(int i=0; i<num; i++){
        string name = baseFileName+"_"+to_string(i);

        fp = fopen(name.c_str(), "r");
        if(fp == NULL){
            cerr << "Error opening file \"" << name << "\"\n";
            exit(1);
        }

        fgets(str, 100, fp);

        real_id_to_object_id.clear();
        while(fscanf(fp, "%d %lf %s %lf %lf", &participant_id, &datatime, str, &latitude, &longitude) != EOF){

            //printf("%d %f %s %f %f\n", participant_id, datatime, str, latitude, longitude);
            trip_id = 1;

            it = real_id_to_object_id.find(participant_id);
            if(it != real_id_to_object_id.end()){
                participant_id = real_id_to_object_id[participant_id];
            }
            else{
                real_id_to_object_id[participant_id] = nextID;
                participant_id = nextID;
                nextID++;
            }

            datatime *= 10;
            act_speed = 20;
            datapoints.push_back(DataPoint(participant_id, trip_id, latitude, longitude, datatime, act_speed));
        }

        fclose(fp);
    }

    return nextID;
}

void display(vector<MovingObject>& saved)
{
    int totaltraj = 0, totalobj = 0, totalline = 0, maxt = 0;

    for(int i=0; i<saved.size(); i++){
        //saved[i].displayMovingObject();
        totalobj++;
        totaltraj += saved[i].trajectories.size();
        for(int j=0; j<saved[i].trajectories.size(); j++) totalline += saved[i].trajectories[j].path.size();
        maxt = max(maxt, (int)saved[i].trajectories.size());
    }
    printf("totalobj = %d\ntotaltraj = %d\ntotalline = %d\nmaxt = %d\n\n", totalobj, totaltraj, totalline, maxt);
}

void addToKDS(Event event)
{
    map<double, int>::iterator it;
    it = index_in_kds_data.find(event.event_time);
    if(it != index_in_kds_data.end()){ /// event time already exists
        int index = it->second;
        kds_data[index].push_back(event);
    }
    else{
        vector<Event> temp; /// new event time
        temp.push_back(event); /// add event
        kds_data.push_back(temp); ///push in kds data

        index_in_kds_data[event.event_time] = kds_data.size()-1; ///update map
        kds.push(event.event_time); ///update kds
    }
}

long long addLineEventsToKDS(long long total_events, double current_time, double event_time){
    //cout << "inside addLineEventsToKDS\n";
    if(event_time > current_time){
        Event e(total_events, NEW_SAMPLE, -1, -1, event_time);
        addToKDS(e);
        //cout << "total_events: " << total_events << endl;
        total_events++;
        //cout << "total_events: " << total_events << "\n\n";
    }
    return total_events;
}

long long addINIEventsToKDS(int oid1, int oid2, long long total_events, double event_time, int event_type){
    Event e(total_events, event_type, oid1, oid2, event_time);
    addToKDS(e);
    //cout << "total_events: " << total_events << endl;
    total_events++;
    //cout << "total_events: " << total_events << "\n\n";
    return total_events;
}

void setCurrentLoc(Line l, MovingObject &obj, double current_time){    /// current location of object found by linear interpolation
    if(l.time_initial >= l.time_final){
        cout << "Degenerate Line: time_initial>=time_final\n";
        exit(1);
    }

    double x = (((current_time - l.time_initial) / (l.time_final - l.time_initial)) * (l.x_final - l.x_initial)) + l.x_initial;
    obj.cur_x = x;

    double y = (((current_time - l.time_initial) / (l.time_final - l.time_initial)) * (l.y_final - l.y_initial)) + l.y_initial;
    obj.cur_y = y;
}

bool isWithin(double x, double y, Rectangle rect){
    return (x >= rect.x1 && x <= rect.x2) && (y >= rect.y1 && y <= rect.y2);
}

bool isIntersecting(Rectangle *r1, Rectangle *r2){
    bool x_int = false, y_int = false;
    if((r1->x1 >= r2->x1 && r1->x1 <= r2->x2) || (r1->x2 >= r2->x1 && r1->x2 <= r2->x2) || (r2->x1 >= r1->x1 && r2->x1 <= r1->x2)) x_int = true;
    if((r1->y1 >= r2->y1 && r1->y1 <= r2->y2) || (r1->y2 >= r2->y1 && r1->y2 <= r2->y2) || (r2->y1 >= r1->y1 && r2->y1 <= r1->y2)) y_int = true;

    return (x_int && y_int);
}

bool hasOverlap(Line l1, Line l2, double d_w, double d_h){
    double xmax_1 = max(l1.x_initial + d_w, l1.x_final + d_w),
    xmin_1 = min(l1.x_initial - d_w, l1.x_final - d_w),
    ymax_1 = max(l1.y_initial + d_h, l1.y_final + d_h),
    ymin_1 = min(l1.y_initial - d_h, l1.y_final - d_h),

    xmax_2 = max(l2.x_initial + d_w, l2.x_final + d_w),
    xmin_2 = min(l2.x_initial - d_w, l2.x_final - d_w),
    ymax_2 = max(l2.y_initial + d_h, l2.y_final + d_h),
    ymin_2 = min(l2.y_initial - d_h, l2.y_final - d_h);

    if (xmax_1 < xmin_2 || xmax_2 < xmin_1) return false;
    if (ymax_1 < ymin_2 || ymax_2 < ymin_1) return false;
    return true;
}

void computeEventTime(Line l1, Line l2, double p1x, double p1y, double p2x, double p2y, double d_w, double d_h, double current_time,
                        bool &hasint, double &t1, double &t2){
    double v1x = (l1.x_final - l1.x_initial) / (l1.time_final - l1.time_initial),
    v1y = (l1.y_final - l1.y_initial) / (l1.time_final - l1.time_initial),
    v2x = (l2.x_final - l2.x_initial) / (l2.time_final - l2.time_initial),
    v2y = (l2.y_final - l2.y_initial) / (l2.time_final - l2.time_initial),

    minvalid = max(l2.time_initial, l1.time_initial),
    maxvalid = min(l2.time_final, l1.time_final),

    tminx = 100000.00,
    tmaxx = -100000.00,
    tmina = 100000.00,
    tminb = 100000.00,
    tmaxa = -100000.00,
    tmaxb = -100000.00,

    tminy = 100000.00,
    tmaxy = -100000.00,
    tminf = 100000.00,
    tmine = 100000.00,
    tmaxf = -100000.00,
    tmaxe = -100000.00;

    if (v1x == v2x && v1y == v2y){
        if (isIntersecting(new Rectangle(p1x - d_w, p1y - d_h, p1x + d_w, p1y + d_h), new Rectangle(p2x - d_w, p2y - d_h, p2x + d_w, p2y + d_h))){
            //return True, minvalid, maxvalid
            hasint = true;
            t1 = minvalid;
            t2 = maxvalid;
            return;
        }
        else{
            //return False, -1, -1
            hasint = false;
            t1 = -1;
            t2 = -1;
            return;
        }
    }

    if (v1x == v2x){
        if (abs(p1x - p2x) > (2 * d_w)){
            //return False, -1, -1
            hasint = false;
            t1 = -1;
            t2 = -1;
            return;
        }

        tmaxx = 1000000;
        tminx = -1000000;
    }
    else{
        if (v1x - v2x > 0){
            tmina = (p2x - p1x) / (v1x - v2x);
            tmaxa = (p2x - p1x + (2 * d_w)) / (v1x - v2x);
            tminb = (p2x - p1x - (2 * d_w)) / (v1x - v2x);
            tmaxb = (p2x - p1x) / (v1x - v2x);
        }
        else{
            tmina = (p2x - p1x + (2 * d_w)) / (v1x - v2x);
            tmaxa = (p2x - p1x) / (v1x - v2x);
            tminb = (p2x - p1x) / (v1x - v2x);
            tmaxb = (p2x - p1x - (2 * d_w)) / (v1x - v2x);
        }

        tmaxx = max(tmina, max(tmaxa, max(tminb, tmaxb)));
        tminx = min(tmina, min(tmaxa, min(tminb, tmaxb)));
    }

    if (v1y == v2y){
        if (abs(p1y - p2y) > (2 * d_h)){
            //return False, -1, -1
            hasint = false;
            t1 = -1;
            t2 = -1;
            return;
        }

        tmaxy = 1000000;
        tminy = -1000000;
    }
    else{
        if (v1y - v2y > 0){
            tminf = (p2y - p1y) / (v1y - v2y);
            tmaxf = (p2y - p1y + (2 * d_h)) / (v1y - v2y);
            tmine = (p2y - p1y - (2 * d_h)) / (v1y - v2y);
            tmaxe = (p2y - p1y) / (v1y - v2y);
        }
        else{
            tminf = (p2y - p1y + (2 * d_h)) / (v1y - v2y);
            tmaxf = (p2y - p1y) / (v1y - v2y);
            tmine = (p2y - p1y) / (v1y - v2y);
            tmaxe = (p2y - p1y - (2 * d_h)) / (v1y - v2y);
        }
        tmaxy = max(tminf, max(tmaxf, max(tmine, tmaxe)));
        tminy = min(tminf, min(tmaxf, min(tmine, tmaxe)));
    }

    // print tmaxx,",", tminx,",", tmaxy,",", tminy
    if (tmaxx < tminy || tmaxy < tminx){
        //return False, -1, -1
        hasint = false;
        t1 = -1;
        t2 = -1;
        return;
    }

    double finalt1 = 100000.00, finalt2 = -100000.00;

    if (tminx >= tminy && tmaxx <= tmaxy){
        finalt1 = tminx;
        finalt2 = tmaxx;
    }
    else if(tminx <= tminy && tmaxx >= tmaxy){
        finalt1 = tminy;
        finalt2 = tmaxy;
    }
    else if(tminx <= tminy && tmaxx <= tmaxy){
        finalt1 = tminy;
        finalt2 = tmaxx;
    }
    else if(tminx >= tminy && tmaxx >= tmaxy){
        finalt1 = tminx;
        finalt2 = tmaxy;
    }

    finalt1 += current_time;
    finalt2 += current_time;

    if(finalt2 <= minvalid){
        //return False, -1, -1
        hasint = false;
        t1 = -1;
        t2 = -1;
        return;
    }
    else if(finalt1 >= maxvalid){
        //return False, -1, -1
        hasint = false;
        t1 = -1;
        t2 = -1;
        return;
    }

    if(finalt1 <= minvalid) finalt1 = minvalid;
    if(finalt2 >= maxvalid) finalt2 = maxvalid;

    hasint = true;
    t1 = finalt1;
    t2 = finalt2;
}



