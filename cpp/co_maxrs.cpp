/*
 * co_maxrs.cpp
 * main program
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-09-10
 */

#include <bits/stdc++.h>
#include "objects.h"
using namespace std;

vector<vector<bool> > adjMatrix;
vector<MovingObject> saved;

map<int, int> dict2;
map<int, int> dict1;
map<int, int>:: iterator itm;

int numberOfObjects;

/// can't be solved using just priority queue
/// idea: use the priority queue to save index in a vector
/// that index contains events at earlies time
/// use the set to see if an event time exists currently
map<double, int> index_in_kds_data;
vector<vector<Event> > kds_data;
priority_queue<double> kds;

#include "utilities.h"

int main()
{
    vector<DataPoint> datapoints;
    numberOfObjects = readFromMNGT(1, datapoints);
    ///cout << numberOfObjects << endl;

    vector<bool> temp(numberOfObjects, false);
    for(int i=0; i<numberOfObjects; i++) adjMatrix.push_back(temp);

    int participant_id, nextID = 0, ind;
    for(int i=0; i<datapoints.size(); i++){
        participant_id = datapoints[i].participant_id;
        itm = dict1.find(participant_id);

        if(itm != dict1.end()){
            dict2[participant_id] += 1;
            if(dict2[participant_id] != STEP) continue;
            else{
                dict2[participant_id] = 0;
                ind = itm->second;
                saved[ind].addPointsToTrajectory(datapoints[i].trip_id, datapoints[i].latitude, datapoints[i].longitude,
                                                            datapoints[i].time, datapoints[i].act_speed);
            }
        }
        else{
            MovingObject moving_object(participant_id);
            moving_object.addPointsToTrajectory(datapoints[i].trip_id, datapoints[i].latitude, datapoints[i].longitude,
                                                datapoints[i].time, datapoints[i].act_speed);
            dict2[participant_id] = 0;
            dict1[participant_id] = nextID;
            saved.push_back(moving_object);
            nextID++;
        }
    }

    //display();

    for(int iteration=0; iteration<1; iteration++){
        printf("iteration = %d\n", iteration);
        int total_events = 0;
        double current_time = 0;
        double next_event = DINF; ///means next event time
        double next_query = DINF;

        vector<Trajectory> current_trajectories;
        vector<Line> current_lines;
        vector<int> current_objects; ///object id of current objects
        map<int, int> object_line_map; ///# keep a map of object->line

        ///clear kds realted data structres if iteration > 0

        /// Setting-up an overall result variable
        vector<CoMaxRes> comaxrs;

        /// setup the lines, trajectories, etc.
        for(int i=0; i<saved.size(); i++){
            if(iteration < saved[i].trajectories.size()){
                current_trajectories.push_back(saved[i].trajectories[iteration]);
                current_lines.push_back(saved[i].trajectories[iteration].path[0]);
                object_line_map[saved[i].object_id] = current_lines.size()-1;
                current_objects.push_back(saved[i].object_id);
            }
        }

        printf("# of Current trajectories: %d\n", (int)current_trajectories.size());
        printf("# of Current lines: %d\n", (int)current_lines.size());

        /// reset the object variables
        for(int i=0; i<saved.size(); i++){
            saved[i].cur_x = 0;
            saved[i].cur_y = 0;
            saved[i].int_num = 0;
            saved[i].inSolution = false;
        }

        /// Setup the total area and coverage
        /// Find the extreme values, and set accordingly
        double x_max = 0, x_min = DINF, y_max = 0, y_min = DINF,
        d_w = coverage.width/2.0, /// r_w/2
        d_h = coverage.height/2.0; /// r_h/2

        for(int i=0; i<current_trajectories.size(); i++){
            Trajectory trj = current_trajectories[i];
            for(int j=0; j<trj.path.size(); j++){
                Line l = trj.path[j];
                x_min = min(l.x_initial, x_min);
                y_min = min(l.y_initial, y_min);
                x_max = max(l.x_initial, x_max);
                y_max = max(l.y_initial, y_max);
            }
            Line l = trj.path[(int)(trj.path.size()-1)];
            x_min = min(l.x_final, x_min);
            y_min = min(l.y_final, y_min);
            x_max = max(l.x_final, x_max);
            y_max = max(l.y_final, y_max);
        }

        for(int i=0; i<current_trajectories.size(); i++){
            Trajectory trj = current_trajectories[i];
            for(int j=0; j<trj.path.size(); j++){
                Line l = trj.path[j];
                l.x_initial += d_w - x_min;
                l.y_initial += d_h - y_min;
                l.x_final += d_w - x_min;
                l.y_final += d_h - y_min;
            }
        }

        printf("x_max = %f\nx_min = %f\ny_max = %f\ny_min = %f\n", x_max, x_min, y_max, y_min);

        Area area(y_max - y_min + r_h, x_max - x_min + r_w);

        /// considering all samples are taken at same time, so one line change event for all current lines
        total_events = addLineEventsToKDS(total_events, current_time, current_lines[0].time_final);

        /// Create the current rectangles
        /// setup current coordinates for objects
        for(int i=0; i<current_lines.size(); i++){
            Line l = current_lines[i];
            Rectangle *rect = new Rectangle(max(0.0, l.x_initial - d_w), max(0.0, l.y_initial - d_h), min(area.width, l.x_initial + d_w),
                                            min(area.height, l.y_initial + d_h));
            current_lines[i].rect = rect;
            int index = dict1[l.grand_id];
            saved[index].cur_x = l.x_initial;
            saved[index].cur_y = l.y_initial;
        }

    }

    return 0;
}
