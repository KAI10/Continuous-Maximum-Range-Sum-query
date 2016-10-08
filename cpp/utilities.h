/*
 * utilities.h
 *
 * necessary functions
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-09-10
 */

 using namespace std;
 #include <iostream>
 #include <cstdio>
 #include <vector>
 #include <cstring>
 #include <map>

const int STEP = 1;
const double r_w = 1000;
const double r_h = 1000;
const double a_w = 100;
const double a_h = 100;

const double DINF = 100000000.0;
const int IINF = 100000000;

Area coverage(r_h, r_w);
///Area area(a_h, a_w);

map<int, int> real_id_to_object_id;
map<int, int>:: iterator it;

int readFromMNGT(int num, vector<DataPoint>& datapoints)
{
    string baseFileName = "simpleBikeData.txt";

    int nextID = 0;
    FILE *fp;

    int participant_id, trip_id;
    double datatime, latitude, longitude, act_speed;
    char str[105];

    for(int i=0; i<num; i++){
        string name = baseFileName+"_"+to_string(i);
        fp = fopen(name.c_str(), "r");

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

int addLineEventsToKDS(int total_events, double current_time, double event_time)
{
    if(event_time > current_time){
        Event e(total_events, NEW_SAMPLE, -1, -1, event_time);
        addToKDS(e);
        total_events++;
    }
    return total_events;
}




