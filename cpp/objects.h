/*
 * objects.h
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-09-10
 */

 using namespace std;

/// Necessary Structures
/// FOR MAKING TIME SAMPLING UNIFORM
const double delta_t = 100;

#include <cmath>
#include <vector>

double dist(double x1, double y1, double x2, double y2)
{
    double dx = x1-x2, dy = y1-y2;
    return sqrt(dx*dx + dy*dy);
}

struct Area
{
    double height, width;
    Area(double height, double width){
        this-> height = height;
        this->width = width;
    }
};

struct Rectangle
{
    double x1, y1, x2, y2, weight;
    Rectangle(double x1, double y1, double x2, double y2, double weight){
        this->x1 = x1;
        this->x2 = x2;
        this->y1 = y1;
        this->y2 = y2;
        this->weight = weight;
    }
};

struct Object
{
    double x,y,weight;
    Object(double x, double y, double weight){
        this->x = x;
        this->y = y;
        this->weight = weight;
    }
};

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
            /// t1 = self.t_last + delta_t  # UNCOMMENT THIS LINE

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
