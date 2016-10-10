/*
 * maxrs.cpp
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-10-09
 */

#include "maxrs_utilities.h"
#define tol 1e-4

Window* process_maxrs(Area area, Area coverage, vector<Object> &objects)
{
    restrict_precision(objects);

    /*
    cout << "computing maxrs with " << objects.size() << " objects.\n";
    for(int i=0; i<objects.size(); i++){
        cout << objects[i].x << ' ' << objects[i].y << ' ' << objects[i].weight << endl;
    }
    */


    sort(objects.begin(), objects.end());
    ///cout << "sorted\n";
    ///print to see the objects

    vector<Rectangle> rectangles; ///contains all the rectangles
    for(int i=0; i<objects.size(); i++)
    {
        Rectangle temp(max(0.0, objects[i].x - coverage.width/2), max(0.0, objects[i].y - coverage.height/2),
                       min(area.width, objects[i].x + coverage.width/2), min(area.height, objects[i].y + coverage.height/2),
                       objects[i].weight);
        rectangles.push_back(temp);
    }

    ///print to see the rectangles
    ///cout << "rectangles created\n";

    vector<double> x1s, xs; ///x1s contains all the x coords, xs contains unique x coords sorted
    for(int i=0; i<rectangles.size(); i++)
    {
        x1s.push_back(rectangles[i].x1);
        x1s.push_back(rectangles[i].x2);
    }

    sort(x1s.begin(), x1s.end());

    //cout.precision(20);

    xs.push_back(x1s[0]);
    double last = x1s[0];
    for(int i=1; i<x1s.size(); i++)
    {
        //cout << "cur: " << x1s[i] << " last: " << last << endl;
        if(fabs(x1s[i] - last) == 0) continue;
        xs.push_back(x1s[i]);
        //cout << x1s[i] << " pushed\n";
        last = x1s[i];
    }

    /*
    ///print to see xs
    printf("xs created, size = %d\n", (int)xs.size());
    for(int i=0; i<xs.size(); i++) cout << xs[i] << ' ';
    cout << "\n\n";
    */

    IntervalTree *root = buildIntervalTree(0, xs.size()-1, xs, NULL);

    ///puts("tree built");
    //puts("$$$$$$$$$$$$$$$$$$$$$$$$$$");
    //preOrderTraverse(root);
    //puts("$$$$$$$$$$$$$$$$$$$$$$$$$$");

    interval_tree_root = root;
    root->window = new Window(xs[0], xs[xs.size() - 1], (double)(-5), (double)(0));

    ///puts("root window created.");

    Window *optimal_window = maxEnclosing(rectangles, coverage, root);
    ///GETTING SEGMENTATION FAULT HERE

    return optimal_window;
}
