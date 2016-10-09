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
map<int, int> dict1; ///maps oid -> index in saved(MovingObject)
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

        /// need to update in current trajectories
        for(int i=0; i<current_trajectories.size(); i++){
            //Trajectory trj = current_trajectories[i];
            for(int j=0; j<current_trajectories[i].path.size(); j++){
                //Line l = trj.path[j];
                current_trajectories[i].path[j].x_initial += d_w - x_min;
                current_trajectories[i].path[j].y_initial += d_h - y_min;
                current_trajectories[i].path[j].x_final += d_w - x_min;
                current_trajectories[i].path[j].y_final += d_h - y_min;
            }
        }

        /// and in current lines
        for(int i=0; i<current_lines.size(); i++){
            current_lines[i].x_initial += d_w - x_min;
            current_lines[i].y_initial += d_h - y_min;
            current_lines[i].x_final += d_w - x_min;
            current_lines[i].y_final += d_h - y_min;
        }

        printf("x_max = %f\nx_min = %f\ny_max = %f\ny_min = %f\n", x_max, x_min, y_max, y_min);

        Area area(y_max - y_min + r_h, x_max - x_min + r_w);

        /// considering all samples are taken at same time, so one line change event for all current lines
        //total_events = addLineEventsToKDS(total_events, current_time, current_lines[0].time_final);

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

        /// add the initial intersecting/non-intersecting events
        /// also set initial variable values
        for(int i=0; i<current_lines.size(); i++){
            Line l1 = current_lines[i];
            int index1 = dict1[l1.grand_id];
            for(int j=i+1; j<current_lines.size(); j++){
                Line l2 = current_lines[j];
                int index2 = dict1[l2.grand_id];

                /// test if currently intersecting
                /// if intersecting, see when it will become non-intersecting next
                /// if that time is less than t_final of both l1 and l2, insert in kds
                /// do other necessary processing for intersecting

                //cout << "isIntersecting: " << l1.rect->x1 << ' ' << l1.rect->y1 << endl;

                if(isIntersecting(l1.rect, l2.rect)){
                    /// do something

                    adjMatrix[l1.grand_id][l2.grand_id] = true;  /// add edge in rectangle graph
                    adjMatrix[l2.grand_id][l1.grand_id] = true;

                    saved[index1].int_num += saved[index2].weight;  /// keeping track of neighbour weight sum
                    saved[index2].int_num += saved[index1].weight;

                    /// find the non-intersecting event
                    bool hasint;
                    double t1, t2;
                    computeEventTime(l1, l2, l1.x_initial, l1.y_initial, l2.x_initial, l2.y_initial, d_w, d_h, current_time, hasint, t1, t2);
                    t2 += 0.001;
                    if(hasint && t2 >= current_time && t2 < min(l1.time_final,l2.time_final)){
                        // create non-intersecting event

                        total_events = addINIEventsToKDS(l1.grand_id, l2.grand_id, total_events, t2, NON_INT);
                    }
                }
                else{
                    adjMatrix[l1.grand_id][l2.grand_id] = false;  /// no edge in rectangle graph
                    adjMatrix[l2.grand_id][l1.grand_id] = false;

                    /// find the intersecting event time if there is any
                    if(hasOverlap(l1, l2, d_w, d_h)){
                        bool hasint;
                        double t1, t2;
                        computeEventTime(l1, l2, l1.x_initial, l1.y_initial, l2.x_initial, l2.y_initial, d_w, d_h, current_time, hasint, t1, t2);
                        if(hasint && t1 > current_time && t1 < min(l1.time_final,l2.time_final)){
                            // create intersecting event
                            //cout << l1.grand_id << ' ' << l2.grand_id << endl;
                            total_events = addINIEventsToKDS(l1.grand_id, l2.grand_id, total_events, t1, INT);
                        }
                    }
                }
            }
        }
        cout << total_events << endl;
        ///implement maxrs
    }

    return 0;
}
