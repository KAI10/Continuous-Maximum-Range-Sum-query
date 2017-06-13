/*
 * objects.h
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-10-08
 */

/// Necessary Structures
using namespace std;

const int NEW_SAMPLE = 1;
const int INT = 2;
const int NON_INT = 3;

/// FOR MAKING TIME SAMPLING UNIFORM
const double delta_t = 100;

#include <cmath>
#include <vector>
//#include "Area.h"
//#include "Object.h"
//#include "Rectangle.h"

double dist(double x1, double y1, double x2, double y2)
{
    double dx = x1-x2, dy = y1-y2;
    return sqrt(dx*dx + dy*dy);
}

struct Line
{
    int grand_id, parent_id, line_id;
    double x_initial, y_initial, time_initial, x_final, y_final, time_final, speed, di;
    Rectangle *rect;

    Line(int grand_id, int parent_id, int line_id, double x1, double y1, double t1, double x2,double y2, double t2, double speed=20.0,
    Rectangle *rect=NULL){
        this->grand_id = grand_id;
        this->parent_id = parent_id;
        this->line_id = line_id;

        x_initial = x1;
        y_initial = y1;
        time_initial = t1;

        x_final = x2;
        y_final = y2;
        time_final = t2;

        this->speed = speed;
        this->rect = rect;

        di = dist(x1, y1, x2, y2);
    }

    void displayLine(){
        printf("x_initial = %f\n\
                y_initial = %f\n\
                time_initial = %f\n\
                x_final = %f\n\
                y_final = %f\n\
                time_final = %f\n\
                di = %f\n\
                speed = %f\n\n", x_initial, y_initial, time_initial, x_final, y_final, time_final, di, speed);
    }
};

struct Trajectory
{
    int parent_id, trajectory_id, count;
    double x_last, y_last, t_last, offset;
    vector<Line> path;

    Trajectory(int parent_id, int trajectory_id, double x_last=0, double y_last=0, double t_last=0){
        this->parent_id = parent_id;
        this->trajectory_id = trajectory_id;

        this->x_last = x_last;
        this->y_last = y_last;
        this->t_last = t_last;

        offset = 0;
        count = 0;
    }

    void addLineToTrajectory(double x1, double y1, double t1, double speed=20){
        double calcspeed = 0;
        if (x_last || y_last || t_last){
            t1 = t1 - offset;

            /// MAKING TIME SAMPLING UNIFORM
            /// (x1,y1,t1) is the new time sample
            t1 = t_last + delta_t;  /// UNCOMMENT THIS LINE

            double distance = dist(x_last, y_last, x1, y1)/1000/1.6093; /// dist measured in mile
            double td = t1 - t_last;
            if (td > 0) calcspeed = (distance / float(td)) * 3600;
        }
        else{
            offset = t1;
            t1 = t1 - offset;
        }

        if (t_last != t1){   /// starts to add lines from 2nd point
            Line line = Line(parent_id, trajectory_id, count, x_last, y_last, t_last, x1, y1, t1, (calcspeed * 1000 * 1.6093) / 3600);
            path.push_back(line);
            count++;
        }

        x_last = x1;
        y_last = y1;
        t_last = t1;
    }

    void displayTrajectory(){
        printf("Trajectory:\ntrajectory_id = %d\n", trajectory_id);
        for(int i=0; i<path.size(); i++) path[i].displayLine();
    }
};

struct MovingObject
{
    int object_id, int_num;
    double weight, cur_x, cur_y;
    bool inSolution;

    vector<Trajectory> trajectories;

    MovingObject(int oid, double weight=1, bool inSolution=false, int int_num=0, double x1=0.0, double y1=0.0){
        this->object_id = oid;
        this->weight = weight;
        this->inSolution = inSolution;
        this->int_num - int_num;

        cur_x = x1;
        cur_y = y1;
    }

    bool checkTrajectoryId(int trajectory_id){
        for(int i=0; i<trajectories.size(); i++){
            if(trajectories[i].trajectory_id == trajectory_id) return true;
        }
        return false;
    }

    void addTrajectoryToMovingObject(int trajectory_id){
        trajectories.push_back(Trajectory(object_id, trajectory_id));
    }

    void addPointsToTrajectory(int trajectory_id, double x, double y, double t, double speed=20){
        bool check = false;
        for(int i=0; i<trajectories.size(); i++){
            if(trajectories[i].trajectory_id == trajectory_id){
                trajectories[i].addLineToTrajectory(x,y,t,speed);
                check = true;
            }
        }
        if(!check){
            Trajectory trajectory(object_id, trajectory_id);
            trajectory.addLineToTrajectory(x,y,t,speed);
            trajectories.push_back(trajectory);
        }
    }

    void displayMovingObject(){
        puts("<MovingObject>");
        printf("MovingObject ID: %d\n", object_id);
        printf("MovingObject Weight: %f\n", weight);
        printf("MovingObject inSolution: %d\n", inSolution);
        printf("MovingObject int_num: %d\n", int_num);
        printf("MovingObject current position: %f %f\n", cur_x, cur_y);
        for(int i=0; i<trajectories.size(); i++) trajectories[i].displayTrajectory();
        puts("</MovingObject>");
    }
};


struct DataPoint
{
    int participant_id, trip_id;
    double latitude, longitude, time, act_speed;

    DataPoint(int participant_id, int trip_id, double latitude, double longitude, double time, double act_speed){
        this->participant_id = participant_id;
        this-> trip_id = trip_id;
        this->latitude = latitude;
        this->longitude = longitude;
        this->time = time;
        this->act_speed = act_speed;
    }
};

struct Event
{
    int event_id, event_type, oid1, oid2;
    double event_time;

    Event(int event_id, int event_type, int oid1, int oid2, double event_time){
        this->event_id = event_id;
        this->event_type = event_type;
        this->oid1 = oid1;
        this->oid2 = oid2;
        this->event_time = event_time;
    }
};

struct pos
{
    double lat, lon;
    pos(double lat, double lon){
        this->lat = lat;
        this->lon = lon;
    }
};

struct CoMaxRes
{
    double t1, t2, countmax;
    vector<int> lobj;
    vector<pos> location;

    CoMaxRes(){}
    CoMaxRes(double t1, double t2, vector<int> lobj, double countmax){
        this->t1 = t1;
        this->t2 = t2;
        this->lobj = lobj;
        this->countmax = countmax;
    }

    void Set(double t1, double t2, vector<int> lobj, double countmax){
        this->t1 = t1;
        this->t2 = t2;
        this->lobj = lobj;
        this->countmax = countmax;
    }

    void recordLocation(vector<MovingObject> & saved, double x_min, double y_min, double d_w, double d_h){
        for(int i=0; i<lobj.size(); i++){
            location.push_back(pos(saved[lobj[i]].cur_x + x_min - d_w, saved[lobj[i]].cur_y + y_min - d_h));
        }
    }

    void recordLocation(CoMaxRes &p){
        for(int i=0; i<p.location.size(); i++){
            location.push_back(pos(p.location[i].lat, p.location[i].lon));
        }
    }    
};
