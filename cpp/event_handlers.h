/*
 * event_handlers.h
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-10-08
 */

long long handle_NEW_SAMPLE_Event(Event event, vector<int> &current_objects, vector<Line> &current_lines, vector<Trajectory> &current_trajectories,
                                map<int, int> &object_line_map, int iteration, long long total_events, double current_time,
                                CoMaxRes &current_maxrs, CoMaxRes &nmaxrs, bool &changed)
{
    //cout << "total_events: " << total_events << endl;
    ///setup tprtree index
    sampleNumber++;
    diskFileName = "tprtree" + to_string(sampleNumber);
    //idx = getIndexFromDisk(diskFileName.c_str(), numberOfObjects);

    vector<int> nCurrent_objects;
    vector<Line> nCurrent_lines;
    vector<Trajectory> nCurrent_trajectories; ///will change if an object disappeares
    map<int , int> nObject_line_map;

    bool disappeares = false;
    double disappeared = 0.0;

    ///setup current location of all current objects
    for(int i=0; i<current_lines.size(); i++){
        Line l = current_lines[i];
        Rectangle *rect = new Rectangle(max(0.0, l.x_final - d_w), max(0.0, l.y_final - d_h), min(area.width, l.x_final + d_w),
                                        min(area.height, l.y_final + d_h));
        current_lines[i].rect = rect;

        int index = dict1[l.grand_id];
        saved[index].cur_x = l.x_final;
        saved[index].cur_y = l.y_final;
    }

    //cout << "current location set\n";

    for(int i=0; i<current_lines.size(); i++){
        Line l = current_lines[i];
        int next_line_id = l.line_id + 1;
        int index = dict1[l.grand_id];

        if(next_line_id < saved[index].trajectories[iteration].path.size()){
            ///there is a next line for this object
            ///so add it to new DS's
            Line next_line = saved[index].trajectories[iteration].path[next_line_id];

            nCurrent_objects.push_back(next_line.grand_id);
            nCurrent_lines.push_back(next_line);
            nCurrent_trajectories.push_back(saved[index].trajectories[iteration]);
            nObject_line_map[next_line.grand_id] = (int)nCurrent_lines.size() - 1;
        }
        else{
            if(saved[index].inSolution){
                disappeares = true;
                disappeared += saved[index].weight;
            }

            ///there is no next line for this object, i.e disappeares
            ///don't add to new DS's
            ///update adjMatrix, and intersecting objects int_num

            ///checking intersection in O(n), later will use tprtree query output
            for(int j=0; j<current_lines.size(); j++){
                Line l2 = current_lines[j];
                if(l2.grand_id == l.grand_id) continue;

                if(isIntersecting(l.rect, l2.rect)){
                    adjMatrix[l.grand_id][l2.grand_id] = false;
                    adjMatrix[l2.grand_id][l.grand_id] = false;

                    int index2 = dict1[l2.grand_id];
                    saved[index2].int_num -= saved[index].weight;
                }
            }
        }
    }

    //cout << "appear disappear handled\n";

    /// need to update in current trajectories
    for(int i=0; i<nCurrent_trajectories.size(); i++){
        //Trajectory trj = current_trajectories[i];
        for(int j=0; j<nCurrent_trajectories[i].path.size(); j++){
            //Line l = trj.path[j];
            nCurrent_trajectories[i].path[j].x_initial += d_w - x_min;
            nCurrent_trajectories[i].path[j].y_initial += d_h - y_min;
            nCurrent_trajectories[i].path[j].x_final += d_w - x_min;
            nCurrent_trajectories[i].path[j].y_final += d_h - y_min;
        }
    }

    /// and in current lines
    for(int i=0; i<nCurrent_lines.size(); i++){
        nCurrent_lines[i].x_initial += d_w - x_min;
        nCurrent_lines[i].y_initial += d_h - y_min;
        nCurrent_lines[i].x_final += d_w - x_min;
        nCurrent_lines[i].y_final += d_h - y_min;
    }

    //cout << "new location set\n";

    /// Create the current rectangles
    /// setup current coordinates for objects
    for(int i=0; i<nCurrent_lines.size(); i++){
        Line l = nCurrent_lines[i];
        Rectangle *rect = new Rectangle(max(0.0, l.x_initial - d_w), max(0.0, l.y_initial - d_h), min(area.width, l.x_initial + d_w),
                                        min(area.height, l.y_initial + d_h));
        nCurrent_lines[i].rect = rect;
        int index = dict1[l.grand_id];
        saved[index].cur_x = l.x_initial;
        saved[index].cur_y = l.y_initial;
    }

    //cout << "rectangles created\n";
/*
    bool done[numberOfObjects];
    memset(done, false, sizeof(done));
*/
    /// add the new intersecting/non-intersecting events
    /// also set initial variable values
    for(int i=0; i<nCurrent_lines.size(); i++){
        Line l1 = nCurrent_lines[i];
        int index1 = dict1[l1.grand_id];


/*        done[l1.grand_id] = true;

        vector<int> intersects;
        query(intersects, l1.x_initial+x_min-d_w, l1.y_initial+y_min-d_h, l1.time_initial,
                        l1.x_final+x_min-d_w, l1.y_final+y_min-d_h, l1.time_final, l1.speed);
*/
        for(int j=i+1; j<nCurrent_lines.size(); j++){
/*
        for(int j=0; j<intersects.size(); j++){
            //cout << total_events << endl;
            int object_id = intersects[j];
            if(done[object_id]) continue;

            int line_index = nObject_line_map[object_id];
            Line l2 = nCurrent_lines[line_index];
            /**/
            Line l2 = nCurrent_lines[j];
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
                        total_events = addINIEventsToKDS(l1.grand_id, l2.grand_id, total_events, t1, INT);
                    }
                }
            }
        }
    }

    //cout << total_events << endl;
    //cout << current_time << endl;
    //cout << nCurrent_lines[0].time_final << endl;
    //cout << "new int/non_int events created. " << total_events << ' ' << current_time << '\n'; // << nCurrent_lines[0].time_final << endl;

    /// considering all samples are taken at same time, so one line change event for all current lines
    if(nCurrent_lines.size()) total_events = addLineEventsToKDS(total_events, current_time, nCurrent_lines[0].time_final);

    //cout << "next line change events added\n";

    current_objects.clear();
    current_lines.clear();
    current_trajectories.clear();
    object_line_map.clear();

    //cout << "\n\n" << current_objects.size() << "\n" << current_lines.size() << "\n" << current_trajectories.size() << "\n"
      //  << object_line_map.size() << "\n";

    current_objects = nCurrent_objects;
    current_lines = nCurrent_lines;
    current_trajectories = nCurrent_trajectories;
    object_line_map = nObject_line_map;

    //cout << "DS'S updated\n";

    //cout << "\n\n" << current_objects.size() << "\n" << current_lines.size() << "\n" << current_trajectories.size() << "\n"
      //  << object_line_map.size() << "\n";

    if(!disappeares){ ///no inSolution object/s disappeared so solution unchanged
        cout << "solution unchanged\n";
        nmaxrs = current_maxrs;
        changed = false;
        return total_events;
    }

    if(current_lines.size() == 0){
        nmaxrs.lobj.clear();
        nmaxrs.countmax = 0;
        changed = true;
        return total_events;
    }

    cout << "maxrs recomputation required\n";

    /// perform the maxrs as some inSolution object/s have disappeared
    double ncountmax = current_maxrs.countmax - disappeared;

    vector<Object> objects;
    for(int i=0; i<current_lines.size(); i++){
        Line l = current_lines[i];
        int index = dict1[l.grand_id];
        MovingObject mo = saved[index];

        if(mo.int_num + mo.weight <= ncountmax && mo.inSolution == false) continue; ///object pruning
        Object temp(mo.cur_x, mo.cur_y, mo.weight);
        objects.push_back(temp);
    }

    if(objects.size() == 0){
        for(int i=0; i<current_lines.size(); i++){
            Line l = current_lines[i];
            int index = dict1[l.grand_id];
            MovingObject mo = saved[index];

            Object temp(mo.cur_x, mo.cur_y, mo.weight);
            objects.push_back(temp);
        }
    }

    Window *opt_window = process_maxrs(area, coverage, objects);
    cout << "score: " << opt_window->score << endl;

    /// Find the objects within MaxRS solution
    double x_co = (opt_window->l + opt_window->r) / 2.0, y_co = opt_window->h;
    Rectangle rect(max(0.0, x_co - d_w), max(0.0, y_co - d_h), min(area.width, x_co + d_w), min(area.height, y_co + d_h));

    vector<int> lobj;
    nmaxrs.Set(current_time, 100000, lobj, opt_window->score);

    ///update previous solution objects
    for(int i=0; i<current_maxrs.lobj.size(); i++){
        int oid = current_maxrs.lobj[i], index = dict1[oid];
        saved[index].inSolution = false;
    }

    ///update new solution objects
    for(int i=0; i<current_lines.size(); i++){
        Line l = current_lines[i];
        int index = dict1[l.grand_id];
        MovingObject mo = saved[index];

        //cout << "Ya hala\n";
        restrict_precision(mo.cur_x);
        restrict_precision(mo.cur_y);

        if(isWithin(mo.cur_x, mo.cur_y, rect)){
            saved[index].inSolution = true;
            nmaxrs.lobj.push_back(l.grand_id);
        }
    }

    changed = true;
    return total_events;
}

long long handle_INT_Event(Event event, vector<Line> &current_lines, map<int, int> &object_line_map, long long total_events, double current_time,
                        CoMaxRes &current_maxrs, CoMaxRes &nmaxrs, bool &changed) ///DO
{
    //cout << "total_events: " << total_events << endl;
    int oid1 = event.oid1, oid2 = event.oid2;

    /// update rectangle graph, add edge
    adjMatrix[oid1][oid2] = true;
    adjMatrix[oid2][oid1] = true;

    int index1 = dict1[oid1], index2 = dict1[oid2];

    /// add each other's weight
    saved[index1].int_num += saved[index2].weight;
    saved[index2].int_num += saved[index1].weight;

    //puts("Status updaed");

    /// add self weights
    double maxp1 = saved[index1].int_num + saved[index1].weight, maxp2 = saved[index2].int_num + saved[index2].weight;

    /// Setup the non-intersecting event, if it exists
    int line_index1 = object_line_map[oid1], line_index2 = object_line_map[oid2];
    Line l1 = current_lines[line_index1], l2 = current_lines[line_index2];

    setCurrentLoc(current_lines[line_index1], saved[index1], current_time);
    setCurrentLoc(current_lines[line_index2], saved[index2], current_time);

    bool hasint;
    double t1, t2;
    computeEventTime(l1, l2, saved[index1].cur_x, saved[index1].cur_y, saved[index2].cur_x, saved[index2].cur_y, d_w, d_h, current_time, hasint, t1, t2);
    t2 += SAFETY;

    if(hasint && t2 >= current_time && t2 < min(l1.time_final,l2.time_final)){
        /// create non-intersecting event
        total_events = addINIEventsToKDS(oid1, oid2, total_events, t2, NON_INT);
        /// print "Adding New Non-int Event at: ",t2
    }

    //puts("NON_INT event created");

    /// EVENT PRUNING
    if(maxp1 <= current_maxrs.countmax || maxp2 <= current_maxrs.countmax){
        nmaxrs = current_maxrs;
        changed = false;
        return total_events;
    }

    puts("Event not pruned");

    /// check if exactly one of oid1/oid2 is inSolution
    if((saved[index1].inSolution && !saved[index2].inSolution) || (!saved[index1].inSolution && saved[index2].inSolution)){
        ///if yes, keep nobj1 as the object inSolution

        //noid1, noid2 = oid1, oid2
        //nobj1, nobj2 = obj1, obj2

        int noid1 = oid1, noid2 = oid2;
        MovingObject nobj1 = saved[index1], nobj2 = saved[index2];

        if(saved[index2].inSolution){
            //noid1, noid2 = oid2, oid1     /// if oid2 is inSolution, swap oid1, oid2
            //nobj1, nobj2 = obj2, obj1
            swap(noid1, noid2);
            swap(nobj1, nobj2);
        }

        /// check if noid2 is now neighbour to all objects inSolution
        bool allAdjacent = true;
        for(int i=0; i<current_maxrs.lobj.size(); i++){
            int oid = current_maxrs.lobj[i];
            if(adjMatrix[noid2][oid] == false){
                allAdjacent = false;
                break;
            }
        }

        /// if yes, nobj2 is just added to the maxrs solution
        if(allAdjacent){
            /// print("LEMMA 5 WORKED")
            /// print (current_maxrs.lobj)

            /// check to see if nobj2 is obj2 (as a swap was done)
            if(oid2 == noid2) saved[index2].inSolution = true;
            else saved[index1].inSolution = true;

            /// create new list of objects inSolution
            vector<int> nlobj = current_maxrs.lobj;
            nlobj.push_back(noid2);

            /// print(nlobj)

            /// new solution score
            double ncountmax = current_maxrs.countmax + nobj2.weight;

            /// create new solution object
            nmaxrs.Set(current_time, 100000, nlobj, ncountmax);
            changed = true;
            return total_events;
        }
        /// if not all adjacent codes below will be executed
    }

    puts("Didn't fall into lemma 5");

    /// else, i.e. oid1, oid2 both are not inSolution, or oid2 is not adjacent to all inSolution objects
    /// If there is a new solution, then oid1, oid2 must be part of that solution.
    /// So, we  only choose all the neighbors of o1, o2 (including o1,o2) to perform the maxrs algo

    vector<Object> objects;

    for(int i=0; i<current_lines.size(); i++){
        Line l = current_lines[i];
        /// only take objects oid1, oid2 and objects that are both adjacent to oid1, oid2 (according to paper)
        /// the 'and' condition is necessary to let oid1, oid2,  pass
        /// because in adjMatrix, adjMatrix[oid1][oid1] == False, same for oid2
        if((adjMatrix[oid1][l.grand_id] == false || adjMatrix[oid2][l.grand_id] == false) && (l.grand_id != oid1 && l.grand_id != oid2))
            continue;

        int index = dict1[l.grand_id];
        //MovingObject mo = saved[index];

        setCurrentLoc(current_lines[i], saved[index], current_time);

        // obj2 = Object(mo.cur_x, mo.cur_y, mo.weight)
        // objects2.append(obj2)
        /// FIRST STAGE of OBJECT PRUNING
        if((saved[index].int_num + saved[index].weight) <= current_maxrs.countmax && saved[index].inSolution == false) continue;
        Object obj = Object(saved[index].cur_x, saved[index].cur_y, saved[index].weight);
        // why the and condition is necessary ??

        objects.push_back(obj);
    }

    /*
    cout << "computing maxrs with " << objects.size() << " objects.\n";
    for(int i=0; i<objects.size(); i++){
        cout << objects[i].x << ' ' << objects[i].y << ' ' << objects[i].weight << endl;
    }
    */

    //cout << "area: " << area.width << ' ' << area.height << endl;
    //cout << "coverage: " << coverage.width << ' ' << coverage.height << endl;

    //cout << "running maxrs...\n";

    Window *opt_window = process_maxrs(area, coverage, objects);

    /// GETTING segmentation fault here!!!!!!!!
    cout << "Score: " << opt_window->score << endl;

    /// if the resulting score is not greater than current score, then solution remains same
    if(opt_window->score <= current_maxrs.countmax){
        nmaxrs = current_maxrs;
        changed = false;
        return total_events;
    }

    /// we have got a better, new solution
    double x_co = (opt_window->l + opt_window->r) / 2.0, y_co = opt_window->h;
    Rectangle rect(max(0.0, x_co - d_w), max(0.0, y_co - d_h), min(area.width, x_co + d_w), min(area.height, y_co + d_h));

    vector<int> nlobj;

    for(int i=0; i<current_lines.size(); i++){
        Line l = current_lines[i];
        int index = dict1[l.grand_id];

        restrict_precision(saved[index].cur_x);
        restrict_precision(saved[index].cur_y);

        if(isWithin(saved[index].cur_x, saved[index].cur_y, rect)){
            nlobj.push_back(l.grand_id);
        }
    }

    /// set last solution objects insolution false
    for(int i=0; i<current_maxrs.lobj.size(); i++){
        int oid = current_maxrs.lobj[i], index = dict1[oid];
        saved[index].inSolution = false;
    }

    /// set new solution objects insolution true
    for(int i=0; i<nlobj.size(); i++){
        int oid = nlobj[i], index = dict1[oid];
        saved[index].inSolution = true;
    }

    /// Create new current_maxrs object
    nmaxrs.Set(current_time, 100000, nlobj, opt_window->score);
    changed = true;
    return total_events;
}

long long handle_NON_INT_Event(Event event, vector<Line> &current_lines, map<int, int> &object_line_map, long long total_events, double current_time,
                            CoMaxRes &current_maxrs, CoMaxRes &nmaxrs, bool &changed) ///OD
{
    //cout << "total_events: " << total_events << endl;
    int oid1 = event.oid1, oid2 = event.oid2;

    /// update the rectangle graph, remove edge
    adjMatrix[oid1][oid2] = false;
    adjMatrix[oid2][oid1] = false;

    int index1 = dict1[oid1], index2 = dict1[oid2];

    // subtract each other's weight
    saved[index1].int_num -= saved[index2].weight;
    saved[index2].int_num -= saved[index1].weight;
    /// No intersecting event next, as it is straight line trajectory
    /// Do nothing about it

    /// EVENT PRUNING
    if(saved[index1].inSolution == false || saved[index2].inSolution == false){
        nmaxrs = current_maxrs;
        changed = false;
        return total_events;
    }

    //cout << "event not pruned\n";

    /// otherwise recompute MaxRS
    /// first, for pruning objects, compute new possible countmax
    double ncountmax = current_maxrs.countmax - min(saved[index1].weight, saved[index2].weight);
    //cout << "ncountmax: " << ncountmax << endl;

    vector<Object> objects;
    // objects2=[]
    for(int i=0; i<current_lines.size(); i++){
        Line l = current_lines[i];
        int index = dict1[l.grand_id];

        /// FIRST STAGE of OBJECT PRUNING
        setCurrentLoc(current_lines[i], saved[index], current_time);
        // obj2 = Object(mo.cur_x, mo.cur_y, mo.weight)
        // objects2.append(obj2)
        //cout << "id: " << saved[index].object_id << " offers: " << saved[index].int_num + saved[index].weight << endl;

        if((saved[index].int_num + saved[index].weight) < ncountmax) continue;

        //cout << "taken: " << saved[index].object_id << endl;

        Object obj(saved[index].cur_x, saved[index].cur_y, saved[index].weight);
        objects.push_back(obj);
    }

    //cout << "preliminary size: " << objects.size() << endl;

    if(objects.size() == 0){
        for(int i=0; i<current_lines.size(); i++){
            Line l = current_lines[i];
            int index = dict1[l.grand_id];

            /// FIRST STAGE of OBJECT PRUNING
            //setCurrentLoc(current_lines[i], saved[index], current_time);
            // obj2 = Object(mo.cur_x, mo.cur_y, mo.weight)
            // objects2.append(obj2)
            Object obj(saved[index].cur_x, saved[index].cur_y, saved[index].weight);
            objects.push_back(obj);
        }
    }

    /*
    cout << "computing maxrs with " << objects.size() << " objects.\n";
    for(int i=0; i<objects.size(); i++){
        cout << objects[i].x << ' ' << objects[i].y << ' ' << objects[i].weight << endl;
    }
    */


    /// recompute maxrs
    // opt_window=process_maxrs(area, coverage, objects2)
    Window *opt_window = process_maxrs(area, coverage, objects);
    double x_co = (opt_window->l + opt_window->r) / 2.0, y_co = opt_window->h;
    Rectangle rect(max(0.0, x_co - d_w), max(0.0, y_co - d_h), min(area.width, x_co + d_w), min(area.height, y_co + d_h));

    cout << "score: " << opt_window->score << endl;

    /// We already know there must be a change, so no need to check if changed
    vector<int> nlobj;
    for(int i=0; i<current_lines.size(); i++){
        Line l = current_lines[i];
        int index = dict1[l.grand_id];

        restrict_precision(saved[index].cur_x);
        restrict_precision(saved[index].cur_y);

        if(isWithin(saved[index].cur_x, saved[index].cur_y, rect)) nlobj.push_back(l.grand_id);
    }

    //cout << "check score with size: " << nlobj.size() << endl;

    /// set last solution objects insolution false
    for(int i=0; i<current_maxrs.lobj.size(); i++){
        int oid = current_maxrs.lobj[i], index = dict1[oid];
        saved[index].inSolution = false;
    }

    /// set new solution objects insolution true
    for(int i=0; i<nlobj.size(); i++){
        int oid = nlobj[i], index = dict1[oid];
        saved[index].inSolution = true;
    }

    /// Create new current_maxrs object
    nmaxrs.Set(current_time, 100000, nlobj, opt_window->score);
    changed = true;
    return total_events;
}

