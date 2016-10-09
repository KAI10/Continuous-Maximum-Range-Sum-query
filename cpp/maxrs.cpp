/*
 * maxrs.cpp
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-10-09
 */

#include "maxrs_utilities.h"


Window* process_maxrs(Area area, Area coverage, vector<Object> &objects)
{
    sort(objects.begin(), objects.end());
    ///print to see the objects

    vector<Rectangle> rectangles; ///contains all the rectangles
    for(int i=0; i<objects.size(); i++){
        Rectangle temp(max(0.0, objects[i].x - coverage.width/2), max(0.0, objects[i].y - coverage.height/2),
                       min(area.width, objects[i].x + coverage.width/2), min(area.height, objects[i].y + coverage.height/2),
                       objects[i].weight);
        rectangles.push_back(temp);
    }

    ///print to see the rectangles

    vector<double> x1s, xs; ///x1s contains all the x coords, xs contains unique x coords sorted
    for(int i=0; i<rectangles.size(); i++){
        x1s.push_back(rectangles[i].x1);
        x1s.push_back(rectangles[i].x2);
    }

    sort(x1s.begin(), x1s.end());

    xs.push_back(x1s[0]);
    double last = x1s[0];
    for(int i=1; i<x1s.size(); i++){
        if(x1s[i] == last) continue;
        xs.push_back(x1s[i]);
        last = x1s[i];
    }

    ///print to see xs

    IntervalTree *root = buildIntervalTree(0, xs.size()-1, xs, NULL);

    //puts("$$$$$$$$$$$$$$$$$$$$$$$$$$");
    //preOrderTraverse(root);
    //puts("$$$$$$$$$$$$$$$$$$$$$$$$$$");

    interval_tree_root = root;
    root->window = new Window(xs[0], xs[xs.size() - 1], (double)(-5), (double)(0));

    Window *optimal_window = maxEnclosing(rectangles, coverage, root);
    return optimal_window;
}
