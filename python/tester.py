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


from kds_greedy import greedy_event_queue
a = greedy_event_queue(3, 0.25)
es = [ Event(i, 1,1,1, i) for i in range(10)  ]
for e in reversed(es): a.push(e)

while not a.empty():
    x = a.pop()
    print x.event_time