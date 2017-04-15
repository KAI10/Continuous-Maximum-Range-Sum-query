/*
 * co_maxrs.cpp
 * main program
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-10-08
 */

#include <bits/stdc++.h>
#include "maxrs.cpp"
#include "objects.h"
using namespace std;

///for changing mode (indexed <-> non-indexed)
///comment out this line for non-indexed mode
///and another line in event_handlers.h
//#define INDEX 1

vector<vector<bool> > adjMatrix;
vector<MovingObject> saved;

map<int, int> dict1; ///maps oid -> index in saved(MovingObject)

int numberOfObjects;
double current_time;

vector<Trajectory> current_trajectories;
vector<Line> current_lines;
vector<int> current_objects; ///object id of current objects
map<int, int> object_line_map; ///# keep a map of object->line

double x_max, x_min, y_max, y_min,
        d_w, /// r_w/2
        d_h; /// r_h/2


/*
/// can't be solved using just priority queue
/// idea: use the priority queue to save index in a vector
/// that index contains events at that time
/// use the map to see if an event time exists currently
map<double, int> index_in_kds_data;
vector<vector<Event> > kds_data;
priority_queue<double, vector<double>, greater<double> > kds;
*/

#ifdef INDEX
    #include "tree_utilities.h"
#else
    #define d1 .01
    #define d2 .01
#endif // INDEX

#include "utilities.h"
#include "kds.hpp"
kds_spatial kds;

#include "kds_utilities.hpp"
#include "event_handlers.h"

int main(int argc, char **argv)
{
    if(argc < 2){
        cerr << "Run Command: ./co_maxrs.out inputFileName" << endl;
        exit(1);
    }

    const clock_t begin_time = clock();

    vector<DataPoint> datapoints;
    numberOfObjects = readFromMNGT(argv[1], 1, datapoints);
    ///cout << numberOfObjects << endl;

    map<int, int> dict2;
    map<int, int>:: iterator itm;

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

    ///free unnecessary memory
    dict2.clear();
    datapoints.clear();

    //display();

    for(int iteration=0; iteration<1; iteration++){
        printf("iteration = %d\n", iteration);
        long long total_events = 0;
        
        current_time = 0;
        double next_event = DINF; ///means next event time
        double next_query = DINF;

        /*
        vector<Trajectory> current_trajectories;
        vector<Line> current_lines;
        vector<int> current_objects; ///object id of current objects
        map<int, int> object_line_map; ///# keep a map of object->line
        */

        current_trajectories.clear();
        current_lines.clear();
        current_objects.clear();
        object_line_map.clear();

        ///clear kds realted data structres if iteration > 0

        /// Setting-up an overall result variable
        vector<CoMaxRes> comaxrs;

        /// setup the lines, trajectories, etc.
        for(int i=0; i<saved.size(); i++){
            if(iteration < saved[i].trajectories.size()){
                current_trajectories.push_back(saved[i].trajectories[iteration]);
                current_lines.push_back(saved[i].trajectories[iteration].path[0]);
                object_line_map[saved[i].object_id] = (int)current_lines.size()-1;
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
        x_max = -DINF, x_min = DINF, y_max = -DINF, y_min = DINF,
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
        kds.set();

        area.height = y_max - y_min + r_h,
        area.width = x_max - x_min + r_w;

        restrict_precision(area.height);
        restrict_precision(area.width);

        cout << area.height << ' ' << area.width << endl;

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

        /// considering all samples are taken at same time, so one line change event for all current lines
        total_events = addLineEventsToKDS(total_events, current_time, current_lines[0].time_final);


        #ifdef INDEX

        ///setup tprtree index
        sampleNumber = 0;
        diskFileName = "tprtree" + to_string(sampleNumber);
        idx = getIndexFromDisk(diskFileName.c_str(), numberOfObjects);

        bool done[numberOfObjects];
        memset(done, false, sizeof(done));

        #endif // INDEX

        /// add the initial intersecting/non-intersecting events
        /// also set initial variable values

        for(int i=0; i<current_lines.size(); i++){
            Line l1 = current_lines[i];
            int index1 = dict1[l1.grand_id];

            #ifdef INDEX

            done[l1.grand_id] = true;
            vector<int> intersects;
            query(intersects, l1.x_initial+x_min-d_w, l1.y_initial+y_min-d_h, l1.time_initial,
                            l1.x_final+x_min-d_w, l1.y_final+y_min-d_h, l1.time_final, l1.speed);

            /*
            cout << "QUERY RESULT of " << l1.grand_id << ": ";
            for(int i=0; i<intersects.size(); i++){
                cout << intersects[i] << endl;
            }
            */

            for(int j=0; j<intersects.size(); j++){
                int object_id = intersects[j];
                if(done[object_id]) continue;
                int line_index = object_line_map[object_id];
                Line l2 = current_lines[line_index];

            #else

            for(int j=i+1; j<current_lines.size(); j++){
                Line l2 = current_lines[j];

            #endif // INDEX

                int index2 = dict1[l2.grand_id];

                /// test if currently intersecting
                /// if intersecting, see when it will become non-intersecting next
                /// if that time is less than t_final of both l1 and l2, insert in kds
                /// do other necessary processing for intersecting

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
                    t2 += SAFETY;
                    if(hasint && t2 >= current_time && t2 < min(l1.time_final,l2.time_final)){
                        // create non-intersecting event
                        //cout << l1.grand_id << ' ' << l2.grand_id << endl;
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
        cout << "After preli O(n^2), total_events: " << total_events << endl;

        kds.display();
        exit(1);


        /// perform the initial maxrs
        vector<Object> objects;
        for(int i=0; i<current_lines.size(); i++){
            Line l = current_lines[i];
            int index = dict1[l.grand_id];
            MovingObject mo = saved[index];
            Object temp(mo.cur_x, mo.cur_y, mo.weight);
            objects.push_back(temp);
        }

        //for(int i=0; i<objects.size(); i++){
          //  cout << objects[i].x << ' ' << objects[i].y << " 1 " << objects[i].weight << endl;
        //}

        Window *opt_window = process_maxrs(area, coverage, objects);

        /// Find the objects within MaxRS solution
        double x_co = (opt_window->l + opt_window->r) / 2.0, y_co = opt_window->h;
        Rectangle rect(max(0.0, x_co - d_w), max(0.0, y_co - d_h), min(area.width, x_co + d_w), min(area.height, y_co + d_h));

        vector<int> lobj;
        CoMaxRes current_maxrs(current_time, 100000, lobj, opt_window->score);

        for(int i=0; i<current_lines.size(); i++){
            Line l = current_lines[i];
            int index = dict1[l.grand_id];
            MovingObject mo = saved[index];

            restrict_precision(mo.cur_x);
            restrict_precision(mo.cur_y);

            if(isWithin(mo.cur_x, mo.cur_y, rect)){
                saved[index].inSolution = true;
                current_maxrs.lobj.push_back(l.grand_id);
            }
        }

        //cout << "inSolution objects:\n";
        //for(int i=0; i<current_maxrs.lobj.size(); i++){
          //  cout << current_maxrs.lobj[i] << endl;
        //}

        cout << "Time range: " << current_maxrs.t1 << " to " << current_maxrs.t2 << "\n";
        cout << "[ " << current_maxrs.lobj[0];
        for(int j=1; j<current_maxrs.lobj.size(); j++) cout << ", " << current_maxrs.lobj[j];
        cout << " ]\n";

        printf("Preliminary Result:\nTime range: %f to %f\nlength: %d\nscore: %f\n", current_maxrs.t1, current_maxrs.t2,
                (int)current_maxrs.lobj.size(), current_maxrs.countmax);

        //cout << kds.size() << endl;

        while(!kds.empty()){
            //if(current_lines.size() < 500) break;
            vector<Event> cur_event = kds.pop();
            double next_event_time = cur_event[0].event_time;
            
            cout << "next_event_time: " << next_event_time << endl;

            current_time = next_event_time;

            //int index = index_in_kds_data[next_event_time];
            
            //cout << "index: " << index << endl;

            for(int i=0; i<cur_event.size(); i++){
                Event event = cur_event[i];

                bool changed;
                CoMaxRes nmaxrs;

                if(event.event_type == NEW_SAMPLE){
                    puts("NEW_SAMPLE Event\n");

                    total_events = handle_NEW_SAMPLE_Event(event, current_objects, current_lines, current_trajectories, object_line_map,
                                            iteration, total_events, current_time, current_maxrs, nmaxrs, changed);
                }
                else if(event.event_type == INT){
                    printf("DO Event between: %d %d\n", event.oid1, event.oid2);

                    total_events = handle_INT_Event(event, current_lines, object_line_map, total_events, current_time, current_maxrs, nmaxrs, changed);
                }
                else{ ///event.event_type == NON_INT
                    printf("OD Event between: %d %d\n", event.oid1, event.oid2);
                    total_events = handle_NON_INT_Event(event, current_lines, object_line_map, total_events, current_time, current_maxrs, nmaxrs, changed);
                }

                if(changed){
                    //cout << "previous score: " << current_maxrs.countmax << endl;

                    vector<int> tempobj;
                    for(int j=0; j<current_maxrs.lobj.size(); j++) tempobj.push_back(current_maxrs.lobj[j]);
                    CoMaxRes tempmaxrs(current_maxrs.t1, current_time, tempobj, current_maxrs.countmax);
                    comaxrs.push_back(tempmaxrs);
                    current_maxrs = nmaxrs;

                    //cout << "# of solutions: " << comaxrs.size() << endl;
                    /*
                    cout << "Time range: " << current_maxrs.t1 << " to " << current_maxrs.t2 << "\nscore: " << current_maxrs.countmax
                        << "\nactual: " << current_maxrs.lobj.size() << "\n";
                    cout << "[ ";
                    if(current_maxrs.lobj.size()) cout << current_maxrs.lobj[0];
                    for(int j=1; j<current_maxrs.lobj.size(); j++) cout << ", " << current_maxrs.lobj[j];
                    cout << " ]\n\n";
                    */
                }

                else{
                    //cout << "total_events: " << total_events << endl;
                    //cout << "Solution not changed\n\n";
                    //cout << "\n";
                }

                //getchar();
            }

        }

        vector<int> tempobj;
        for(int j=0; j<current_maxrs.lobj.size(); j++) tempobj.push_back(current_maxrs.lobj[j]);
        CoMaxRes tempmaxrs(current_maxrs.t1, current_time, tempobj, current_maxrs.countmax);
        comaxrs.push_back(tempmaxrs);


        puts("Final Result:");
        for(int i=0; i<comaxrs.size(); i++){
            CoMaxRes res = comaxrs[i];
            cout << "Time range: " << res.t1 << " to " << res.t2 << "\nscore: " << res.countmax << "\nactual: " << res.lobj.size() << "\n";
            cout << "[ ";
            if(res.lobj.size()) cout << res.lobj[0];
            for(int j=1; j<res.lobj.size(); j++) cout << ", " << res.lobj[j];
            cout << " ]\n\n";
        }

        cout << "Total events processed: " << total_events << endl;
    }

    cout << "elapsed time: " << double( clock () - begin_time ) /  CLOCKS_PER_SEC << " seconds\n";

    return 0;
}
