import utm
import csv
import time
import heapq
import py_treap
import collections
from datetime import datetime as dt
#from scipy.spatial.distance import cdist

from kds_greedy import greedy_event_queue

import sys
from objects import *
from maxrs import *

'''
from bitarray import bitarray
from Queue import PriorityQueue
from Queue import Queue
from collections import deque

from scipy.optimize import fsolve
import math
import random as rd

import matplotlib.pyplot as plt
from sympy import *
import scipy.optimize
import sympy
from sympy.abc import L
from sympy.abc import x
from sympy import Plane, Point3D
from sympy.geometry import Line3D, Segment3D
from spherical_geometry.polygon import SphericalPolygon
from matplotlib.lines import Line2D
'''


STEP = 1
r_w = 1000
r_h = 1000
a_w = 100
a_h = 100
coverage = Area(r_h, r_w)
area = Area(a_h, a_w)
fmt = '%Y-%m-%d %H:%M:%S'
fmt2 = '%m/%d/%Y %H:%M:%S'
first_date_s = '2006-4-10 4:58:14'
f_time = dt.strptime(first_date_s, fmt)


############################################ Supporting Functions #########################################################################


def distance(x1, y1, x2, y2):
    #XA = np.array([[x1, y1]])
    #XB = np.array([[x2, y2]])
    #Y = cdist(XA, XB, 'euclidean')
    #return Y[0][0]
    dx, dy = x1 - x2, y1 - y2
    return (dx*dx + dy*dy)**0.5


def display(dict1):
    fw = open("output.txt", 'w')
    totaltraj = 0
    totalobj = 0
    totalline = 0
    maxt = 0
    for key, value in dict1.items():
        # print (key, value)
        totalobj = totalobj + 1
        fw.write("  < MovingObject>\n")
        fw.write("  MovingObject ID:" + str(value.object_id) + "\n")
        fw.write("  No. Of Trajectories:" + str(len(value.trajectories)) + "\n")
        if len(value.trajectories) > maxt:
            maxt = len(value.trajectories)
        for trajec in value.trajectories:
            totaltraj = totaltraj + 1
            fw.write("    < trajectory >\n")
            fw.write("    trajectory ID:" + str(trajec.trajectory_id) + "\n")
            fw.write("    No. Of Lines:" + str(trajec.count) + "\n")
            for line in trajec.path:
                totalline = totalline + 1
                fw.write("      < line >\n")
                fw.write("      line ID:" + str(line.line_id) + "\n")
                fw.write("      grand ID:" + str(line.grand_id) + "\n")
                fw.write("      parent ID:" + str(line.parent_id) + "\n")
                fw.write("      x1: " + str(line.x_initial) + "\n")
                fw.write("      y1: " + str(line.y_initial) + "\n")
                fw.write("      t1: " + str(line.time_initial) + "\n")
                fw.write("      x2:" + str(line.x_final) + "\n")
                fw.write("      y2: " + str(line.y_final) + "\n")
                fw.write("      t2: " + str(line.time_final) + "\n")
                fw.write("      speed : " + str(line.speed) + "\n")
                fw.write("      di : " + str(line.di) + "\n")
                fw.write("      < /line>\n")
            fw.write("    < /trajectory >\n")
        fw.write("  < /MovingObject>\n")
        # if key == 'participant_id':
        # print 'hey'
        # else:
        # value.displayMovingObject()
    print str(totalobj) + " " + str(totaltraj) + " " + str(totalline) + " " + str(maxt)


def getTimeInSeconds(timestr):
    c_time = dt.strptime(timestr, fmt)
    diff = c_time - f_time
    return (diff.days * 86400) + diff.seconds


def addToKDS(kds, event):
    kds.push(event)


def addLineEventsToKDS(kds, line, total_events, current_time):
    if line.time_final > current_time:
        e = Event(total_events, Event.NEW_SAMPLE, line.grand_id, None, line.time_final)
        addToKDS(kds, e)
        total_events += 1
    return total_events


def addINIEventsToKDS(kds, oid1, oid2, total_events, event_time, event_type):
    e = Event(total_events, event_type, oid1, oid2, event_time)
    addToKDS(kds, e)
    total_events += 1
    return total_events


def getMax(x2, x4):
    if x2 >= x4:
        return x2
    else:
        return x4


def getMin(x1, x3):
    if x1 <= x3:
        return x1
    else:
        return x3


def hasOverlap(l1, l2, d_w, d_h):
    xmax_1 = getMax(l1.x_initial + d_w, l1.x_final + d_w)
    xmin_1 = getMin(l1.x_initial - d_w, l1.x_final - d_w)
    ymax_1 = getMax(l1.y_initial + d_h, l1.y_final + d_h)
    ymin_1 = getMin(l1.y_initial - d_h, l1.y_final - d_h)

    xmax_2 = getMax(l2.x_initial + d_w, l2.x_final + d_w)
    xmin_2 = getMin(l2.x_initial - d_w, l2.x_final - d_w)
    ymax_2 = getMax(l2.y_initial + d_h, l2.y_final + d_h)
    ymin_2 = getMin(l2.y_initial - d_h, l2.y_final - d_h)

    if xmax_1 < xmin_2 or xmax_2 < xmin_1:
        return False
    if ymax_1 < ymin_2 or ymax_2 < ymin_1:
        return False
    return True


def computeEventTime(l1, l2, p1x, p1y, p2x, p2y, d_w, d_h, current_time):
    v1x = (l1.x_final - l1.x_initial) / (l1.time_final - l1.time_initial)
    v1y = (l1.y_final - l1.y_initial) / (l1.time_final - l1.time_initial)
    v2x = (l2.x_final - l2.x_initial) / (l2.time_final - l2.time_initial)
    v2y = (l2.y_final - l2.y_initial) / (l2.time_final - l2.time_initial)

    minvalid = max(l2.time_initial, l1.time_initial)
    maxvalid = min(l2.time_final, l1.time_final)

    tminx = 100000.00
    tmaxx = -100000.00
    tmina = 100000.00
    tminb = 100000.00
    tmaxa = -100000.00
    tmaxb = -100000.00

    tminy = 100000.00
    tmaxy = -100000.00
    tminf = 100000.00
    tmine = 100000.00
    tmaxf = -100000.00
    tmaxe = -100000.00

    if v1x == v2x and v1y == v2y:
        if isIntersecting(Rectangle(p1x - d_w, p1y - d_h, p1x + d_w, p1y + d_h), Rectangle(p2x - d_w, p2y - d_h, p2x + d_w, p2y + d_h)):
            return True, minvalid, maxvalid
        else:
            return False, -1, -1
    if v1x == v2x:
        if abs(p1x - p2x) > (2 * d_w):
            return False, -1, -1
        tmaxx = 1000000
        tminx = -1000000
    else:
        if v1x - v2x > 0:
            tmina = (p2x - p1x) / (v1x - v2x)
            tmaxa = (p2x - p1x + (2 * d_w)) / (v1x - v2x)
            tminb = (p2x - p1x - (2 * d_w)) / (v1x - v2x)
            tmaxb = (p2x - p1x) / (v1x - v2x)
        else:
            tmina = (p2x - p1x + (2 * d_w)) / (v1x - v2x)
            tmaxa = (p2x - p1x) / (v1x - v2x)
            tminb = (p2x - p1x) / (v1x - v2x)
            tmaxb = (p2x - p1x - (2 * d_w)) / (v1x - v2x)
        tmaxx = max(tmina, tmaxa, tminb, tmaxb)
        tminx = min(tmina, tmaxa, tminb, tmaxb)

    if v1y == v2y:
        if abs(p1y - p2y) > (2 * d_h):
            return False, -1, -1
        tmaxy = 1000000
        tminy = -1000000
    else:
        if v1y - v2y > 0:
            tminf = (p2y - p1y) / (v1y - v2y)
            tmaxf = (p2y - p1y + (2 * d_h)) / (v1y - v2y)
            tmine = (p2y - p1y - (2 * d_h)) / (v1y - v2y)
            tmaxe = (p2y - p1y) / (v1y - v2y)
        else:
            tminf = (p2y - p1y + (2 * d_h)) / (v1y - v2y)
            tmaxf = (p2y - p1y) / (v1y - v2y)
            tmine = (p2y - p1y) / (v1y - v2y)
            tmaxe = (p2y - p1y - (2 * d_h)) / (v1y - v2y)
        tmaxy = max(tminf, tmaxf, tmine, tmaxe)
        tminy = min(tminf, tmaxf, tmine, tmaxe)

    # print tmaxx,",", tminx,",", tmaxy,",", tminy
    if tmaxx < tminy or tmaxy < tminx:
        return False, -1, -1
    finalt1 = 100000.00
    finalt2 = -100000.00

    if tminx >= tminy and tmaxx <= tmaxy:
        finalt1 = tminx
        finalt2 = tmaxx
    elif tminx <= tminy and tmaxx >= tmaxy:
        finalt1 = tminy
        finalt2 = tmaxy
    elif tminx <= tminy and tmaxx <= tmaxy:
        finalt1 = tminy
        finalt2 = tmaxx
    elif tminx >= tminy and tmaxx >= tmaxy:
        finalt1 = tminx
        finalt2 = tmaxy

    finalt1 += current_time
    finalt2 += current_time

    if finalt2 <= minvalid:
        return False, -1, -1
    elif finalt1 >= maxvalid:
        return False, -1, -1

    if finalt1 <= minvalid:
        finalt1 = minvalid
    if finalt2 >= maxvalid:
        finalt2 = maxvalid
    return True, finalt1, finalt2


def setCurrentLoc(l, obj, current_time):    # current location of object found by linear interpolation
    if l.time_initial >= l.time_final:
        raise ValueError('Degenerate Line: time_initial>=time_final')
    x = (((current_time - l.time_initial) / (l.time_final - l.time_initial)) * (l.x_final - l.x_initial)) + l.x_initial
    obj.cur_x = x

    y = (((current_time - l.time_initial) / (l.time_final - l.time_initial)) * (l.y_final - l.y_initial)) + l.y_initial
    obj.cur_y = y


############################################ Event Handling ###############################################################################
def handleEvent(e, current_lines, current_objects, total_events, kds, dict1, iteration, current_time, object_line_map, current_maxrs, d_w,
                d_h, adjMatrix):
    total_events = total_events - 1

    # print("HANDLE EVENT")

    # New Sample Point Event
    if e.event_type == Event.NEW_SAMPLE:    # line change event
        # print("Line Change Event")

        # ADD ONE LINE CHANGE EVENT FOR NEXT SAMPLE HERE

        # FOR EACH line IN current_lines DO WHAT NEEDS TO BE DONE IF THERE IS NEXT SAMPLE (FOR THIS OBJECT) OR NOT
        # I.E KEEP/REMOVE FROM current_lines, UPDATE adjMatrix, neighbour weight sum

        # RUN AN O(n^2) LOOP HERE (LIKE THE ONE IN MAIN)
        # for i in range (len(current_lines))
        #       l1 = current_lines[i]
        #       for j in range(i+1, len(current_lines))
        #           l2 = current_lines[j]
        #           ......
        #           ........

        oid = e.oid1
        cur_line = object_line_map[oid]
        current_lines.remove(cur_line)
        next_line = cur_line.line_id + 1    # next_line is the next line's id

        if next_line < len(dict1[oid].trajectories[iteration].path):     # if there is a next line
            n_line = dict1[oid].trajectories[iteration].path[next_line]  # n_line is the next line

            # LINE CHANGE EVENT BEING ADDED
            total_events = addLineEventsToKDS(kds, n_line, total_events, current_time)  # add next line change event to kds

            current_lines.append(n_line)
            object_line_map[oid] = n_line

            cmo = dict1[n_line.grand_id]
            setCurrentLoc(n_line, cmo, current_time)
            n_line.rect = Rectangle(max(0, cmo.cur_x - d_w), max(0, cmo.cur_y - d_h),
                                    min(area.width, cmo.cur_x + d_w), min(area.height, cmo.cur_y + d_h))

            for l2 in current_lines:
                if l2.grand_id != n_line.grand_id:  # l!=n_line
                    mo = dict1[l2.grand_id]
                    setCurrentLoc(l2, mo, current_time)
                    l2.rect = Rectangle(max(0, mo.cur_x - d_w), max(0, mo.cur_y - d_h),
                                        min(area.width, mo.cur_x + d_w), min(area.height, mo.cur_y + d_h))

                    if isIntersecting(n_line.rect, l2.rect):
                        # do something
                        # dict1[l1.grand_id].int_num+=dict1[l2.grand_id].weight
                        # dict1[l2.grand_id].int_num+=dict1[l1.grand_id].weight
                        # find the non-intersecting event
                        hasint, t1, t2 = computeEventTime(n_line, l2, cmo.cur_x, cmo.cur_y, mo.cur_x, mo.cur_y, d_w, d_h, current_time)
                        t2 += 0.001
                        if hasint == True and t2 >= current_time and t2 < n_line.time_final and t2 < l2.time_final:
                            # create non-intersecting event
                            total_events = addINIEventsToKDS(kds, n_line.grand_id, l2.grand_id, total_events, t2, Event.NON_INT)

                            # print "Adding New Non-int Event at: ",t2
                            # print l1.x_initial-d_w,l1.x_initial+d_w, l1.y_initial-d_h, l1.y_initial+d_h, l1.time_initial,
                            # l1.x_final-d_w,l1.x_final+d_w, l1.y_final-d_h, l1.y_final+d_h, l1.time_final
                            # print l2.x_initial-d_w,l2.x_initial+d_w, l2.y_initial-d_h, l2.y_initial+d_h, l2

                    # if non-intersecting, see when it will become intersecting next
                    # if that time is less than t_final of both l1 and l2, insert in kds
                    # do other necessary processing for non-intersecting
                    else:
                        # find the intersecting event time if there is any
                        if hasOverlap(n_line, l2, d_w, d_h):
                            hasint, t1, t2 = computeEventTime(n_line, l2, cmo.cur_x, cmo.cur_y, mo.cur_x, mo.cur_y, d_w, d_h, current_time)
                            if hasint and t1 > current_time and t1 < n_line.time_final and t1 < l2.time_final:
                                # create intersecting event
                                total_events = addINIEventsToKDS(kds, n_line.grand_id, l2.grand_id, total_events, t1, Event.INT)

                                # print "Adding New Int Event at: ",t1
                                # print l1.x_initial-d_w,l1.x_initial+d_w, l1.y_initial-d_h, l1.y_initial+d_h, l1.time_initial,
                                # l1.x_final-d_w,l1.x_final+d_w, l1.y_final-d_h, l1.y_final+d_h, l1.time_final
                                # print l2.x_initial-d_w,l2.x_initial+d_w, l2.y_initial-d_h, l2.y_initial+d_h, l2.time_initial,
                                # l2.x_final-d_w,l2.x_final+d_w, l2.y_final-d_h, l2.y_final+d_h, l2.time_fina

        else:
            # there is no next line, the object disappeares
            # First check with which objects it has intersection
            # Then adjust int_num of those objects accordingly
            cmo = dict1[cur_line.grand_id]
            setCurrentLoc(cur_line, cmo, current_time)
            cur_line.rect = Rectangle(max(0, cmo.cur_x - d_w), max(0, cmo.cur_y - d_h),
                                      min(area.width, cmo.cur_x + d_w), min(area.height, cmo.cur_y + d_h))
            for l2 in current_lines:
                mo = dict1[l2.grand_id]
                setCurrentLoc(l2, mo, current_time)
                l2.rect = Rectangle(max(0, mo.cur_x - d_w), max(0, mo.cur_y - d_h),
                                    min(area.width, mo.cur_x + d_w), min(area.height, mo.cur_y + d_h))
                if isIntersecting(cur_line.rect, l2.rect):
                    # remove the weight of the object and remove edge from adjMatrix
                    adjMatrix[mo.object_id][cmo.object_id] = False
                    adjMatrix[cmo.object_id][mo.object_id] = False
                    mo.int_num -= cmo.weight

            if cmo.inSolution and len(current_lines) >= 1:
                ncountmax = current_maxrs.countmax - cmo.weight
                objects = []    # objects on which the new maxrs will be calculated
                # objects2=[]
                for l in current_lines:
                    mo = dict1[l.grand_id]
                    if (mo.int_num + mo.weight) <= ncountmax and mo.inSolution == False:    # object pruning
                        continue
                    # FIRST STAGE of OBJECT PRUNING
                    obj = Object(mo.cur_x, mo.cur_y, mo.weight)
                    objects.append(obj)

                # print "New Sample Recomputation Event : ", current_time, ", ", len(current_lines),", ", len(objects), ", ",e.event_type
                if len(objects) == 0:
                    for l in current_lines:
                        mo = dict1[l.grand_id]
                        obj = Object(mo.cur_x, mo.cur_y, mo.weight)
                        objects.append(obj)     # all the objects taken !!

                # recompute maxrs
                # opt_window=process_maxrs(area, coverage, objects2)
                opt_window = process_maxrs(area, coverage, objects)
                x_co = (opt_window.l + opt_window.r) / 2.0
                y_co = opt_window.h
                rect = Rectangle(max(0, x_co - d_w), max(0, y_co - d_h),
                                 min(area.width, x_co + d_w), min(area.height, y_co + d_h))

                # We already know there must be a change, so no need to check if changed
                nlobj = []
                for l in current_lines:
                    mo = dict1[l.grand_id]
                    if isWithin(mo.cur_x, mo.cur_y, rect):
                        nlobj.append(l.grand_id)

                # set last solution objects insolution false
                for oid in current_maxrs.lobj:
                    mo = dict1[oid]
                    mo.inSolution = False

                # set new solution objects insolution true
                for oid in nlobj:
                    mo = dict1[oid]
                    mo.inSolution = True

                # Create new current_maxrs object
                n_maxrs = CoMaxRes(current_time, 100000, nlobj, opt_window.score)

                current_objects.remove(cur_line.grand_id)
                current_trajectories.remove(dict1[cur_line.grand_id].trajectories[iteration])
                del object_line_map[cur_line.grand_id]

                return total_events, n_maxrs, True
                # The trajectory and object is finished for this iteration

            current_objects.remove(cur_line.grand_id)
            current_trajectories.remove(dict1[cur_line.grand_id].trajectories[iteration])
            del object_line_map[cur_line.grand_id]

        return total_events, current_maxrs, False

    # Non-Intersecting Event    # OD event
    elif e.event_type == Event.NON_INT:
        # print("OD")
        # Handle the non-intersecting events
        oid1 = e.oid1
        oid2 = e.oid2

        # update the rectangle graph, remove edge
        adjMatrix[oid1][oid2] = False
        adjMatrix[oid2][oid1] = False

        obj1 = dict1[oid1]
        obj2 = dict1[oid2]

        # subtract each other's weight
        obj1.int_num -= obj2.weight
        obj2.int_num -= obj1.weight
        # No intersecting event next, as it is straight line trajectory
        # Do nothing about it

        # EVENT PRUNING
        if obj1.inSolution == False or obj2.inSolution == False:
            return total_events, current_maxrs, False

        # otherwise recompute MaxRS
        # first, for pruning objects, compute new possible countmax
        ncountmax = 0.0
        if obj1.weight <= obj2.weight:
            ncountmax = current_maxrs.countmax - obj1.weight
        else:
            ncountmax = current_maxrs.countmax - obj2.weight

        objects = []
        # objects2=[]
        for l in current_lines:
            mo = dict1[l.grand_id]
            # FIRST STAGE of OBJECT PRUNING
            setCurrentLoc(l, mo, current_time)
            # obj2 = Object(mo.cur_x, mo.cur_y, mo.weight)
            # objects2.append(obj2)
            obj = Object(mo.cur_x, mo.cur_y, mo.weight)
            if (mo.int_num + mo.weight) <= ncountmax:
                continue
            objects.append(obj)

        # print "Recomputation Event : ", current_time, ", ", len(current_lines),", ", len(objects), ", ",e.event_type
        if len(objects) == 0:
            for l in current_lines:
                mo = dict1[l.grand_id]
                obj = Object(mo.cur_x, mo.cur_y, mo.weight)
                objects.append(obj)

        # recompute maxrs
        # opt_window=process_maxrs(area, coverage, objects2)
        opt_window = process_maxrs(area, coverage, objects)
        x_co = (opt_window.l + opt_window.r) / 2.0
        y_co = opt_window.h
        rect = Rectangle(max(0, x_co - d_w), max(0, y_co - d_h),
                         min(area.width, x_co + d_w), min(area.height, y_co + d_h))

        # We already know there must be a change, so no need to check if changed
        nlobj = []
        for l in current_lines:
            mo = dict1[l.grand_id]
            if isWithin(mo.cur_x, mo.cur_y, rect):
                nlobj.append(l.grand_id)

        # set last solution objects insolution false
        for oid in current_maxrs.lobj:
            mo = dict1[oid]
            mo.inSolution = False

        # set new solution objects insolution true
        for oid in nlobj:
            mo = dict1[oid]
            mo.inSolution = True

        # Create new current_maxrs object
        n_maxrs = CoMaxRes(current_time, 100000, nlobj, opt_window.score)
        return total_events, n_maxrs, True

    # Intersecting Event
    elif e.event_type == Event.INT:     # DO event
        # print("DO")
        # Handle the intersecting events
        oid1 = e.oid1
        oid2 = e.oid2

        # update rectangle graph, add edge
        adjMatrix[oid1][oid2] = True
        adjMatrix[oid2][oid1] = True

        obj1 = dict1[oid1]
        obj2 = dict1[oid2]

        # add each other's weight
        obj1.int_num += obj2.weight
        obj2.int_num += obj1.weight

        # add self weights
        maxp1 = obj1.int_num + obj1.weight
        maxp2 = obj2.int_num + obj2.weight

        # Setup the non-intersecting event, if it exists
        l1 = object_line_map[oid1]
        l2 = object_line_map[oid2]
        setCurrentLoc(l1, obj1, current_time)
        setCurrentLoc(l2, obj2, current_time)
        hasint, t1, t2 = computeEventTime(l1, l2, obj1.cur_x, obj1.cur_y, obj2.cur_x, obj2.cur_y, d_w, d_h, current_time)
        t2 += 0.001

        if hasint and t2 >= current_time and t2 < l1.time_final and t2 < l2.time_final:
            # create non-intersecting event
            total_events = addINIEventsToKDS(kds, l1.grand_id, l2.grand_id, total_events, t2, Event.NON_INT)
            # print "Adding New Non-int Event at: ",t2

        # EVENT PRUNING
        if maxp1 <= current_maxrs.countmax or maxp2 <= current_maxrs.countmax:
            return total_events, current_maxrs, False

        # ASHIK
        # check if exactly one of oid1/oid2 is inSolution
        if (obj1.inSolution and not obj2.inSolution) or (not obj1.inSolution and obj2.inSolution):
            # if yes, keep nobj1 as the object inSolution

            noid1, noid2 = oid1, oid2
            nobj1, nobj2 = obj1, obj2

            if obj2.inSolution:
                noid1, noid2 = oid2, oid1     # if oid2 is inSolution, swap oid1, oid2
                nobj1, nobj2 = obj2, obj1

            # check if noid2 is now neighbour to all objects inSolution
            allAdjacent = True
            for oid in current_maxrs.lobj:
                if adjMatrix[noid2][oid] == False:
                    allAdjacent = False
                    break

            # if yes, nobj2 is just added to the maxrs solution
            if allAdjacent:
                # print("LEMMA 5 WORKED")

                # print (current_maxrs.lobj)

                # check to see if nobj2 is obj2 (as a swap was done)
                if oid2 == noid2: obj2.inSolution = True
                else: obj1.inSolution = True

                # create new list of objects inSolution
                nlobj = current_maxrs.lobj;
                nlobj.append(noid2);

                # print(nlobj)

                # new solution score
                ncountmax = current_maxrs.countmax + nobj2.weight

                # create new solution object
                n_maxrs = CoMaxRes(current_time, 100000, nlobj, ncountmax)
                return total_events, n_maxrs, True

                # if not all adjacent codes below will be executed

        # else, i.e. oid1, oid2 both are not inSolution, or oid2 is not adjacent to all inSolution objects
        # If there is a new solution, then oid1, oid2 must be part of that solution.
        # So, we  only choose all the neighbors of o1, o2 (including o1,o2) to perform the maxrs algo

        objects = []
        # objects2=[]

        for l in current_lines:
            # ASHIK
            # only take objects oid1, oid2 and objects that are both adjacent to oid1, oid2 (according to paper)
            # the 'and' condition is necessary to let oid1, oid2,  pass
            # because in adjMatrix, adjMatrix[oid1][oid1] == False, same for oid2
            if (adjMatrix[oid1][l.grand_id] == False or adjMatrix[oid2][l.grand_id] == False) and (l.grand_id != oid1 and l.grand_id != oid2):
                continue

            mo = dict1[l.grand_id]
            # FIRST STAGE of OBJECT PRUNING
            setCurrentLoc(l, mo, current_time)
            # obj2 = Object(mo.cur_x, mo.cur_y, mo.weight)
            # objects2.append(obj2)

            obj = Object(mo.cur_x, mo.cur_y, mo.weight)
            if (mo.int_num + mo.weight) <= current_maxrs.countmax and mo.inSolution == False: continue
            # why the and condition is necessary ??

            objects.append(obj)

        # recompute maxrs
        # print "Recomputation Event : ", current_time, ", ", len(current_lines),", ", len(objects), ", ",e.event_type
        # print area.width,", ",area.height
        # print coverage.width, ", ",coverage.height
        # for o in objects:
        # print o.x,", ", o.y,", ",o.weight

        # opt_window=process_maxrs(area, coverage, objects2)
        opt_window = process_maxrs(area, coverage, objects)

        # if the resulting score is not greater than current score, then solution remains same
        if opt_window.score <= current_maxrs.countmax:
            return total_events, current_maxrs, False

        # we have got a better, new solution
        x_co = (opt_window.l + opt_window.r) / 2.0
        y_co = opt_window.h
        rect = Rectangle(max(0, x_co - d_w), max(0, y_co - d_h),
                         min(area.width, x_co + d_w), min(area.height, y_co + d_h))

        nlobj = []
        # changed = False, don't need this as certainly changed
        for l in current_lines:
            mo = dict1[l.grand_id]
            if isWithin(mo.cur_x, mo.cur_y, rect):
                # if mo.inSolution == False:
                    # changed = True
                nlobj.append(l.grand_id)

        # if changed == False:
            # return total_events, current_maxrs, False
        # else:
        # set last solution objects insolution false
        for oid in current_maxrs.lobj:
            mo = dict1[oid]
            mo.inSolution = False

        # set new solution objects insolution true
        for oid in nlobj:
            mo = dict1[oid]
            mo.inSolution = True

        # Create new current_maxrs object
        n_maxrs = CoMaxRes(current_time, 100000, nlobj, opt_window.score)
        return total_events, n_maxrs, True


def getOpposite(f, t1, t2):
    if (f(t1) <= 0 and f(t2) >= 0) or (f(t1) >= 0 and f(t2) <= 0):
        return (t1, t2)
    else:
        if (f(t1) <= 0 and f((t1 + t2) / 2.0) >= 0) or (f(t1) >= 0 and f((t1 + t2) / 2.0) <= 0):
            return (t1, (t1 + t2) / 2.0)
        elif (f(t2) <= 0 and f((t1 + t2) / 2.0) >= 0) or (f(t2) >= 0 and f((t1 + t2) / 2.0) <= 0):
            return (t2, (t1 + t2) / 2.0)
        else:
            return (t1, t2)


def isIntersecting(r1, r2):
    x_int = False
    y_int = False
    if (r1.x1 >= r2.x1 and r1.x1 <= r2.x2) or (r1.x2 >= r2.x1 and r1.x2 <= r2.x2) or (r2.x1 >= r1.x1 and r2.x1 <= r1.x2):
        x_int = True
    if (r1.y1 >= r2.y1 and r1.y1 <= r2.y2) or (r1.y2 >= r2.y1 and r1.y2 <= r2.y2) or (r2.y1 >= r1.y1 and r2.y1 <= r1.y2):
        y_int = True
    if x_int == True and y_int == True:
        return True
    return False


def isWithin(x, y, rect):
    if (x >= rect.x1 and x <= rect.x2) and (y >= rect.y1 and y <= rect.y2):
        return True
    return False


def readFromBike(num):
    datapoints = []
    csvfile = open('data/gps_points_smoothed_sorted.csv', 'rb')
    csvreader = csv.reader(csvfile, delimiter=',')

    # ASHIK: This var is used to assign id to objects
    nextID = 0

    for row in csvreader:
        '''print ', '.join(row)'''
        if row[0] == 'participant_id':
            continue
        datatime = int(row[4])
        latitude = float(row[2])
        longitude = float(row[3])
        trip_id = int(row[1])

        # ASHIK: handling the mapping here
        participant_id = int(row[0])
        if participant_id in real_id_to_object_id:  # if already in dictionary
            participant_id = real_id_to_object_id[participant_id]  # get already assigned id
        else:
            real_id_to_object_id[participant_id] = nextID  # else save new id
            object_id_to_real_id.append(participant_id)  # save the mapping
            participant_id = nextID  # assign new id
            nextID += 1  # increment for further use

        act_speed = float(row[5])
        dp = DataPoint(participant_id, trip_id, latitude, longitude, datatime, act_speed)
        datapoints.append(dp)

    # flow_queue.append(dp)
    # datapoints.sort(key=lambda x: x.time, reverse=False)
    return nextID, datapoints


def readFromMS():
    datapoints = []
    csvfile = open('data/all-data-ms.csv', 'rb')
    csvreader = csv.reader(csvfile, delimiter=' ')

    # ASHIK: This var is used to assign id to objects
    nextID = 0

    for row in csvreader:
        '''print ', '.join(row)'''
        datatime = int(row[4])
        latitude = float(row[2])
        longitude = float(row[3])
        trip_id = int(row[1])

        # ASHIK: handling the mapping here
        participant_id = int(row[0])
        if participant_id in real_id_to_object_id:  # if already in dictionary
            participant_id = real_id_to_object_id[participant_id]  # get already assigned id
        else:
            real_id_to_object_id[participant_id] = nextID  # else save new id
            object_id_to_real_id.append(participant_id)  # save the mapping
            participant_id = nextID  # assign new id
            nextID += 1  # increment for further use

        act_speed = 20.0
        dp = DataPoint(participant_id, trip_id, latitude, longitude, datatime, act_speed)
        datapoints.append(dp)
    # flow_queue.append(dp)
    # datapoints.sort(key=lambda x: x.time, reverse=False)
    return nextID, datapoints


# num = NO. of files containing input dataset
# files names are assumed to be: name_0, name_1 ... and so on (for example 3984.txt_0, ...)
def readFromMNGT(num):
    datapoints = []

    baseFileName = '../gps_points_smoothed_sorted.csv';

    # ASHIK: This var is used to assign id to objects
    nextID = 0

    for i in range(num):
        name = baseFileName + '_' + str(i)
        csvfile = open(name, 'rb')
        csvreader = csv.reader(csvfile, delimiter=',')
        mp = {}

        # this map will be used for all the files, so clear it every time
        real_id_to_object_id.clear()

        for row in csvreader:
            '''print ', '.join(row)'''
            if row[0] == 'participant_id':
                continue

            latitude = float(row[2])
            longitude = float(row[3])
            #val = utm.from_latlon(latitude, longitude)
            #latitude = val[0]
            #longitude = val[1]
            trip_id = 1    # only one trajectory for every object

            # ASHIK: handling the mapping here
            participant_id = int(row[0])
            if participant_id in real_id_to_object_id:  # if already in dictionary
                participant_id = real_id_to_object_id[participant_id]   # get already assigned id
            else:
                real_id_to_object_id[participant_id] = nextID     # else save new id
                # object_id_to_real_id.append(participant_id)       # save the mapping
                participant_id = nextID     # assign new id
                nextID += 1                 # increment for further use

            # if participant_id not in mp:
            # mp[participant_id]= rd.uniform(5, 20)
            # datatime = int(row[1])*mp[participant_id]
            datatime = int(row[4]) * 10
            act_speed = 20.0
            dp = DataPoint(participant_id, trip_id, latitude, longitude, datatime, act_speed)
            datapoints.append(dp)
            # if participant_id<10:
            # print dp.participant_id, ' ', dp.trip_id, ' ', dp.time, ' ', dp.latitude, ' ', dp.longitude

        # flow_queue.append(dp)
    # datapoints.sort(key=lambda x: x.time, reverse=False)
    return nextID, datapoints


################################ Main Experiment Processing ###############################################################################
# with open('data/gps_points_smoothed_sorted.csv', 'rb') as csvfile:
# '''with open('gps_points_smoothed.csv', 'rb') as csvfile:'''

if __name__ == "__main__":
    dict1 = collections.OrderedDict()
    dict2 = {}

    # ASHIK: real id is participant id in data set
    real_id_to_object_id = {}    # ASHIK: This dictionary maps: participant_id -> object id, not valid for MNGT due to multiple files
    object_id_to_real_id = []   # ASHIK: This list maps: object id -> real participant_id, not valid for MNGT due to multiple files

    numberOfObjects, datapoints = readFromMNGT(1)  # sending NO. of files as parameter

    # print(numberOfObjects, len(datapoints))
    # sys.exit("DONE")

    # NO. of objects = NO. of ids
    # setup the adjacency matrix of the rectangle graph
    # numberOfObjects = len(object_id_to_real_id)
    adjMatrix = np.array([[False for i in range(numberOfObjects)] for j in range(numberOfObjects)], bool)

    for dp in datapoints:
        # print dict1
        if dp.participant_id in dict1:
            # print "object already present"
            dict2[dp.participant_id] += 1
            if dict2[dp.participant_id] != STEP:  # STEP == 1 always, if think from start makes sense, though weird
                continue
            else:
                dict2[dp.participant_id] = 0
                dict1[dp.participant_id].addPointsToTrajectory(dp.trip_id, dp.latitude, dp.longitude, dp.time, dp.act_speed)
        else:
            # print "object not present"
            moving_object_1 = MovingObject(dp.participant_id)
            moving_object_1.addPointsToTrajectory(dp.trip_id, dp.latitude, dp.longitude, dp.time, dp.act_speed)
            dict1[dp.participant_id] = moving_object_1
            dict2[dp.participant_id] = 0

    # for key, value in dict1.items():
    #    print len(value.trajectories)
    # exit()

    '''
    for key, value in dict1.items() :
      for trajec in value.trajectories:
        print trajec.trajectory_id
    '''
    # Plot graph to see how the data points are distributed
    # for iteration in range(27):
    #  fig, ax = plt.subplots()
    #  for key, value in dict1.items():
    #      if iteration<len(value.trajectories):
    #          X=[]
    #          Y=[]
    #          trj=value.trajectories[iteration]
    #          for l in trj.path:
    #              X.append(l.x_initial)
    #              Y.append(l.y_initial)
    #              #linestyles = ['-', '--', '-.', ':']
    #              #ax.plot(X, Y, linestyle='None', marker='o', markersize=5)
    #              ax.plot(X, Y)
    #  ax.yaxis.set_ticks_position('left') # this one is optional but I still recommend it...
    #  ax.xaxis.set_ticks_position('bottom')
    #  ax.set_title("Trajectories in Dataset", y=1.05)
    #  ax.set_ylabel("Longitude (N)", labelpad=10)
    #  ax.set_xlabel("Latitude (E)",labelpad=10)
    #  plt.show()
    #  plt.cla()
    #  plt.clf()
    #
    '''
    time_iter = []

    # read from the file the completion time for each iteration
    with open('data/completion_time.csv', 'rb') as csvfile2:
        csvreader2 = csv.reader(csvfile2, delimiter=' ')
        for row in csvreader2:
            time_iter.append(float(row[2]))
    '''

    display(dict1)
    allQuery = []

    times = []

    ###################################### Main working loop #############################################################################
    for iteration in range(1):
        print iteration
        
        total_events = 0
        current_time = 0.0
        next_event = 100000.0
        next_query = 100000.0
        current_trajectories = []
        current_lines = []
        current_objects = []
        # current_queries=[]
        object_line_map = {}
        isProcessed = {}

        # Setting-up an overall result variable
        comaxrs = []

        # setup the lines, trajectories, etc.
        for key, value in dict1.items():
            if iteration < len(value.trajectories):                                       # check if the object has this many trajectories
                current_trajectories.append(value.trajectories[iteration])                # this trajectory will be used in this iteration
                current_lines.append(value.trajectories[iteration].path[0])               # add first lines as current lines
                object_line_map[value.object_id] = value.trajectories[iteration].path[0]  # keep a map of object->line
                current_objects.append(value.object_id)

        # for l in current_lines: print l.grand_id

        print "Trajectories: " + str(len(current_trajectories))
        print "Lines: " + str(len(current_lines))

        # reset the object variables
        for key, value in dict1.items():    # value is a MovingObject here
            value.cur_x = 0
            value.cur_y = 0
            value.int_num = 0
            value.inSolution = False

        # setup queries for this iteration at random times
        # avg_td=10.0
        # total_time=time_iter[iteration]
        # q_no=int(total_time/avg_td)
        # for i in range(q_no):
        #    qtime=rd.random()*total_time
        #    q=Query((i+1)*(iteration+1), qtime)
        #    heapq.heappush(current_queries, (qtime, q))
        # heapq.heapify(current_queries)
        # print current_queries

        # Setup the total area and coverage
        # Find the extreme values, and set accordingly
        x_max = 0           # -INF
        x_min = 100000000   # +INF
        y_max = 0
        y_min = 100000000
        d_w = coverage.width / 2.0   # r_w / 2
        d_h = coverage.height / 2.0  # r_h / 2

        for trj in current_trajectories:
            for l in trj.path:
                if l.x_initial < x_min: x_min = l.x_initial
                if l.y_initial < y_min: y_min = l.y_initial
                if l.x_initial > x_max: x_max = l.x_initial
                if l.y_initial > y_max: y_max = l.y_initial

            l = trj.path[len(trj.path) - 1]
            if l.x_final < x_min: x_min = l.x_final
            if l.y_final < y_min: y_min = l.y_final
            if l.x_final > x_max: x_max = l.x_final
            if l.y_final > y_max: y_max = l.y_final

        print "x_max: ", x_max, ", x_min: ", x_min, ", y_max: ", y_max, ", y_min: ", y_min

        for trj in current_trajectories:
            for l in trj.path:
                l.x_initial -= x_min    # translation of point
                l.x_initial += d_w      # to accomodate the dual rectangle
                l.y_initial -= y_min
                l.y_initial += d_h
                l.x_final -= x_min
                l.x_final += d_w
                l.y_final -= y_min
                l.y_final += d_h

                # shouldn't this be like the previous one ?? NO!! it's correct
                # l=trj.path[len(trj.path)-1]
                # l.x_final-=x_min
                # l.x_final+=d_w
                # l.y_final=y_min
                # l.y_final+=d_h

        print "x_max: ", x_max, ", x_min: ", x_min, ", y_max: ", y_max, ", y_min: ", y_min

        area = Area(y_max - y_min + r_h, x_max - x_min + r_w)

        # for trj in current_trajectories:
        # X=[]
        # Y=[]
        # for l in trj.path:
        # X.append(l.x_initial)
        # Y.append(l.y_initial)
        # plt.plot(X, Y, 'b-', linewidth=1.5)
        # plt.xlabel('Latitude', fontsize=16)
        # plt.ylabel('Longitude', fontsize=16)
        # plt.title('Trajectories in Dataset', fontsize=16)
        # plt.show()
        # plt.clf()

        kds = greedy_event_queue(5, 0.25)

        # LINE CHANGE EVENT BEING ADDED
        # INSTEAD OF LOOP, ADD ONLY ONE LINE CHANGE EVENT HERE
        for line in current_lines:
            total_events = addLineEventsToKDS(kds, line, total_events, current_time)    # add line change events in kds for current lines

        # Create the current rectangles
        # setup current coordinates for objects
        for l in current_lines:
            l.rect = Rectangle(max(0, l.x_initial - d_w), max(0, l.y_initial - d_h),
                               min(area.width, l.x_initial + d_w), min(area.height, l.y_initial + d_h))
            # draw_rectangle(l.rect,0.3)
            dict1[l.grand_id].cur_x = l.x_initial
            dict1[l.grand_id].cur_y = l.y_initial

        # plt.axis([-5,area.width + 5,-5,area.height + 5])
        # plt.show()
        # plt.clf()

        # add the initial intersecting/non-intersecting events
        # also set initial variable values
        for i in range(len(current_lines)):
            l1 = current_lines[i]
            for j in range(i + 1, len(current_lines), 1):
                l2 = current_lines[j]

                # test if currently intersecting
                # if intersecting, see when it will become non-intersecting next
                # if that time is less than t_final of both l1 and l2, insert in kds
                # do other necessary processing for intersecting
                if isIntersecting(l1.rect, l2.rect):
                    # do something

                    adjMatrix[l1.grand_id][l2.grand_id] = True  # add edge in rectangle graph
                    adjMatrix[l2.grand_id][l1.grand_id] = True

                    dict1[l1.grand_id].int_num += dict1[l2.grand_id].weight  # keeping track of neighbour weight sum
                    dict1[l2.grand_id].int_num += dict1[l1.grand_id].weight

                    # find the non-intersecting event
                    hasint, t1, t2 = computeEventTime(l1, l2, l1.x_initial, l1.y_initial, l2.x_initial, l2.y_initial, d_w, d_h,
                                                      current_time)
                    t2 += 0.001
                    if hasint and t2 >= current_time and t2 < l1.time_final and t2 < l2.time_final:
                        # create non-intersecting event
                        total_events = addINIEventsToKDS(kds, l1.grand_id, l2.grand_id, total_events, t2, Event.NON_INT)

                        # print "Adding New Non-int Event at: ",t2
                        # print l1.x_initial-d_w,l1.x_initial+d_w, l1.y_initial-d_h, l1.y_initial+d_h, l1.time_initial,
                        # l1.x_final-d_w,l1.x_final+d_w, l1.y_final-d_h, l1.y_final+d_h, l1.time_final
                        # print l2.x_initial-d_w,l2.x_initial+d_w, l2.y_initial-d_h, l2.y_initial+d_h, l2.time_initial,
                        # l2.x_final-d_w,l2.x_final+d_w, l2.y_final-d_h, l2.y_final+d_h, l2.time_final

                # if non-intersecting, see when it will become intersecting next
                # if that time is less than t_final of both l1 and l2, insert in kds
                # do other necessary processing for non-intersecting
                else:
                    adjMatrix[l1.grand_id][l2.grand_id] = False  # no edge in rectangle graph
                    adjMatrix[l2.grand_id][l1.grand_id] = False

                    # find the intersecting event time if there is any
                    if hasOverlap(l1, l2, d_w, d_h):
                        hasint, t1, t2 = computeEventTime(l1, l2, l1.x_initial, l1.y_initial, l2.x_initial, l2.y_initial, d_w, d_h,
                                                          current_time)
                        if hasint and t1 > current_time and t1 < l1.time_final and t1 < l2.time_final:
                            # create intersecting event
                            total_events = addINIEventsToKDS(kds, l1.grand_id, l2.grand_id, total_events, t1, Event.INT)

                            # print "Adding New Int Event at: ",t1
                            # print l1.x_initial-d_w,l1.x_initial+d_w, l1.y_initial-d_h, l1.y_initial+d_h, l1.time_initial,
                            # l1.x_final-d_w,l1.x_final+d_w, l1.y_final-d_h, l1.y_final+d_h, l1.time_final
                            # print l2.x_initial-d_w,l2.x_initial+d_w, l2.y_initial-d_h, l2.y_initial+d_h, l2.time_initial,
                            # l2.x_final-d_w,l2.x_final+d_w, l2.y_final-d_h, l2.y_final+d_h, l2.time_final

        # Perform the initial MaxRS
        objects = []
        for l in current_lines:
            mo = dict1[l.grand_id]
            obj = Object(mo.cur_x, mo.cur_y, mo.weight)
            objects.append(obj)

        opt_window = process_maxrs(area, coverage, objects)

        # Find the objects within MaxRS solution
        x_co = (opt_window.l + opt_window.r) / 2.0
        y_co = opt_window.h
        rect = Rectangle(max(0, x_co - d_w), max(0, y_co - d_h),
                         min(area.width, x_co + d_w), min(area.height, y_co + d_h))

        lobj = []
        current_maxrs = CoMaxRes(current_time, 100000, lobj, opt_window.score)
        for l in current_lines:
            mo = dict1[l.grand_id]
            if isWithin(mo.cur_x, mo.cur_y, rect):
                mo.inSolution = True
                current_maxrs.lobj.append(l.grand_id)

        print "Preliminary Result: ", current_maxrs.t1, ", ", current_maxrs.t2, ", ", len(current_maxrs.lobj), ", ", current_maxrs.countmax

        # While there are events, keep processing

        # next_query=current_queries[0][0]
        # if len(current_lines) <153:
        # break;

        #print("NO. of entries in kds: ", len(kds))
        events_processed = 0;

        t1 = time.clock()
        while not kds.empty():
            next_event = kds.pop()
            
            # process the events
            current_time = next_event[0].event_time
            for h in next_event:
                events_processed += 1
                total_events, nmaxrs, changed = handleEvent(h, current_lines, current_objects, total_events, kds, dict1, iteration,
                                                            current_time, object_line_map, current_maxrs, d_w, d_h, adjMatrix)
                if changed:
                    #print current_time
                    tempobj = []
                    for lo in current_maxrs.lobj:
                        tempobj.append(lo)
                    tempmaxrs = CoMaxRes(current_maxrs.t1, current_time, tempobj, current_maxrs.countmax)
                    comaxrs.append(tempmaxrs)
                    current_maxrs = nmaxrs

        print("NO. of events processed: ", events_processed)

        t2 = time.clock()
        print t2 - t1
        times.append(t2 - t1)

        # t1=time.clock()
        # for i in range(100):
        #  objects=[]
        #  for l in current_lines:
        #      mo = dict1[l.grand_id]
        #      obj = Object(mo.cur_x, mo.cur_y, mo.weight)
        #      objects.append(obj)
        #  opt_window=process_maxrs(area, coverage, objects)
        #  #Find the objects within MaxRS solution
        #  x_co=(opt_window.l + opt_window.r)/2.0
        #  y_co=opt_window.h
        #  rect=Rectangle(max(0,x_co-d_w),max(0,y_co - d_h),
        #                          min(area.width,x_co+d_w),min(area.height,y_co+d_h))
        #  lobj=[]
        #  current_maxrs=CoMaxRes(current_time, 100000, lobj, opt_window.score)
        #  for l in current_lines:
        #      mo = dict1[l.grand_id]
        #      if isWithin(mo.cur_x, mo.cur_y, rect):
        #          mo.inSolution = True
        #          current_maxrs.lobj.append(l.grand_id)
        # t2=time.clock()
        # print t2-t1
        # times.append(t2-t1)
        # print str(iteration)+" "+str(len(current_lines))+" "+str(current_time)

        tempobj = []

        for lo in current_maxrs.lobj:
            tempobj.append(lo)

        tempmaxrs = CoMaxRes(current_maxrs.t1, current_time, tempobj, current_maxrs.countmax)
        comaxrs.append(tempmaxrs)

        print "Final Result:    "
        for c in comaxrs:
            print "<", c.t1, ", ", c.t2, ", ", str(c.lobj), ", ", c.countmax, ">"

    print "Average Runtime: ", sum(times) / float(len(times))

    ############################# Plot the graph #####################################################################################
    # X=[]
    # Y=[]
    # P=[]
    # Q=[]
    # A=[]
    # B=[]
    ##Accumulate brute data
    # total_time=0.0
    # total_query=0.0
    # start=7
    # count=0
    # for i in range(5,50):
    #  count+=1
    #  for q in allQuery:
    #      if q.total_beads==i:
    #          total_time+=q.brute_ptime
    #          total_query+=1
    #  if total_query !=0 and count==3:
    #      X.append(start)
    #      start=start+3
    #      count=0
    #      avgPtime=(total_time)/(total_query)
    #      Y.append(avgPtime)
    #
    ##Accumulate our data
    # total_time=0.0
    # total_query=0.0
    # start=7
    # count=0
    # for i in range(5,50):
    #  count+=1
    #  for q in allQuery:
    #      if q.total_beads==i:
    #          total_time+=q.our_ptime
    #          total_query+=1
    #  if total_query !=0 and count==3:
    #      P.append(start)
    #      start=start+3
    #      count=0
    #      avgPtime=(total_time)/(total_query)
    #      Q.append(avgPtime)
    # print total_query
    #
    ##line1=plt.plot(X, Y, 'b-', linewidth=1.5, label='Brute Force')
    ##line2=plt.plot(P, Q,'g-', linewidth=3, label='Delaunay Proximity')
    ##plt.legend(loc='best')
    ##plt.xlabel('No. of Objects', fontsize=16)
    ##plt.ylabel('Avg.Processing Time (ms)', fontsize=16)
    ##plt.title('Avg. Processing Time Comparison', fontsize=16)
    ##plt.show()
