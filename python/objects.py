import numpy as np

# FOR MAKING TIME SAMPLING UNIFORM
delta_t = 100


# Corresponds to the line-segment for each object, consists of a start time and end time
class Line:
    def __init__(self, grand_id, parent_id, line_id, x1, y1, t1, x2, y2, t2, speed=20.0, rect=None):
        self.parent_id = parent_id
        self.grand_id = grand_id
        self.line_id = line_id
        self.x_initial = float(x1)
        self.y_initial = float(y1)
        self.time_initial = float(t1)
        self.x_final = float(x2)
        self.y_final = float(y2)
        self.time_final = float(t2)
        self.rect = rect
        self.di = np.sqrt(np.power(x2 - x1, 2) + np.power(y2 - y1, 2))
        self.speed = speed

    def displayLine(self):
        print "      < line >"
        print "      x1: " + str(self.x_initial)
        print "      y1: " + str(self.y_initial)
        print "      t1: " + str(self.time_initial)
        print "      x2:" + str(self.x_final)
        print "      y2: " + str(self.y_final)
        print "      t2: " + str(self.time_final)
        print "      speed : " + str(self.speed)
        print "      di : " + str(self.di)
        print "      < /line >"


'''----------------------------------------'''


# Trajectory is a collection of lines. An object can have many trajectories..
class Trajectory:
    def __init__(self, parent_id, trajectory_id, x_last=0, y_last=0, t_last=0):
        self.parent_id = parent_id
        self.trajectory_id = trajectory_id
        self.path = []
        self.x_last = x_last
        self.y_last = y_last
        self.t_last = t_last
        self.offset = 0
        self.count = 0

    def addLineToTrajectory(self, x1, y1, t1, speed=20):
        calcspeed = 0.0
        if (self.x_last == 0 and self.y_last == 0 and self.t_last == 0) == False:
            t1 = t1 - self.offset

            # MAKING TIME SAMPLING UNIFORM
            # (x1,y1,t1) is the new time sample
            # t1 = self.t_last + delta_t  # UNCOMMENT THIS LINE

            distance = ((np.sqrt(np.power(self.x_last - x1, 2) + np.power(self.y_last - y1, 2))) / 1000.0) / 1.6093  # dist measured in mile
            td = t1 - self.t_last
            if td > 0:
                calcspeed = (distance / float(td)) * 3600
        else:
            self.offset = t1
            t1 = t1 - self.offset

        if self.t_last != t1:   # starts to add lines from 2nd point
            line = Line(self.parent_id, self.trajectory_id, self.count, self.x_last, self.y_last, self.t_last, x1, y1, t1,
                        (calcspeed * 1000 * 1.6093) / 3600)
            self.path.append(line)
            self.count = self.count + 1

        self.x_last = x1
        self.y_last = y1
        self.t_last = t1

    def displayTrajectory(self):
        print "    < trajectory >"
        print "    trajectory ID:" + self.trajectory_id
        for line in self.path:
            line.displayLine()
        print "    < /trajectory >"


'''-------------------------------------------'''


### Moving Object class
class MovingObject:
    """ this class repersents moving object and its trajectory"""

    def __init__(self, oid, weight=1, inSolution=False, int_num=0, x1=0.0, y1=0.0):
        self.object_id = oid
        self.trajectories = []       # a moving object may have several trajectories
        self.weight = weight
        self.inSolution = inSolution
        self.int_num = int_num
        self.cur_x = float(x1)
        self.cur_y = float(y1)

    def checkTrajectoryId(self, trajectory_id):
        for trajec in self.trajectories:
            if trajec.trajectory_id == trajectory_id:
                return True
            else:
                return False

    def addTrajectoryToMovingObject(self, trajectory_id):
        trajectory = Trajectory(self.object_id, trajectory_id)
        self.trajectories.append(trajectory)

    def addPointsToTrajectory(self, trajectory_id, x, y, t, speed=20):
        ''' print 'in addPointsToTrajectory function' '''
        check = 0
        for trajec in self.trajectories:
            if trajec.trajectory_id == trajectory_id:
                trajec.addLineToTrajectory(x, y, t, speed)
                check = 1
                '''print 'check-1' '''
        if check == 0:
            '''print 'check-0' '''
            trajectory = Trajectory(self.object_id, trajectory_id)
            self.trajectories.append(trajectory)
            trajectory.addLineToTrajectory(x, y, t, speed)

    def displayMovingObject(self):
        print "  < MovingObject>"
        print "  MovingObject ID:" + self.object_id
        print "  MovingObject Weight:" + self.weight
        print "  MovingObject inSolution:" + self.inSolution
        print "  MovingObject int_num:" + self.int_num
        print "  MovingObject current position:(" + self.cur_x + "," + self.cur_y + ")"
        for trajec in self.trajectories:
            trajec.displayTrajectory()
        print "  < /MovingObject>"


'''--------------------------------------------'''


class DataPoint:
    def __init__(self, participant_id, trip_id, latitude, longitude, time, act_speed):
        self.participant_id = participant_id
        self.trip_id = trip_id
        self.latitude = latitude
        self.longitude = longitude
        self.time = time
        self.act_speed = act_speed
        return


'''----------------------------------------'''


class Event:
    NEW_SAMPLE = 1
    INT = 2      # intersection
    NON_INT = 3  # non-intersection

    def __init__(self, event_id, event_type, oid1, oid2, event_time):
        self.event_id = event_id
        self.event_type = event_type
        self.oid1 = oid1
        self.oid2 = oid2
        self.event_time = event_time
        return


'''----------------------------------------'''


# These classes are not needed for you (Farabi and Imtiaz) ##############################

class Query:
    def __init__(self, q_id, time):
        self.time = time
        self.brute_ptime = -1.0
        self.our_ptime = -1.0
        self.oid = -1.0
        self.avg_neighbors = 1.0
        self.total_beads = 0
        return


'''----------------------------------------'''


# the result class for comaxrs
class CoMaxRes:
    def __init__(self, t1, t2, lobj, countmax):
        self.t1 = float(t1)
        self.t2 = float(t2)
        self.lobj = lobj
        self.countmax = float(countmax)


'''----------------------------------------'''


# the result class for comaxrs
class StatMaxRes:
    def __init__(self, x, y, countmax):
        self.x = float(x)
        self.y = float(y)
        self.countmax = float(countmax)


'''----------------------------------------'''


# object result class
class ObjectRes:
    def __init__(self, te=0.0, pe=0.0, ie=0.0, pie=0.0, ne=0.0, pne=0.0):
        self.te = te
        self.pe = pe
        self.ie = ie
        self.pie = pie
        self.ne = ne
        self.pne = pne
        self.to = []
        self.io = []
        self.no = []
        self.error = []


'''----------------------------------------'''


# object result class
class TimeRes:
    def __init__(self):
        self.te = []
        self.diff = []


'''----------------------------------------'''


# object result class
class ScoreRes:
    def __init__(self):
        self.to = []
        self.te = 0.0
        self.pe = 0.0


'''----------------------------------------'''


# object result class
class SizeRes:
    def __init__(self):
        self.to = []
        self.te = 0.0
        self.pe = 0.0
