from helpers import event_queue, bidict
import py_treap


class greedy_event_queue(event_queue):
	"""
	Implements greedy variant of event queue
	Parameters: 
		parts: # of parts
		delta: parameter for balancing
	"""

	def __init__(self, parts, delta):
		self.kds = [ py_treap.treap() for i in range(parts) ] # we 'parts' # of kds
		self.tmap = bidict() # keeps track of the kds part with earliest event time
		self.smap = bidict() # keeps track of the size of the kds parts
		self.delta = delta
		for i in range(parts): self.smap.update((0, i), i) # initially all kds parts have size = 0 


	def empty(self):
		return len(self.tmap.kv) <= 0


	def push(self, event):
		# retrieve the smallest kds part
		smallest_part, smallest_part_index = self.smap.getFirst()
		smallest_part_size, smallest_part_count = smallest_part

		# insert event in that part
		if event.event_time in self.kds[smallest_part_index]:
			self.kds[smallest_part_index][event.event_time].append(event)
		else: self.kds[smallest_part_index][event.event_time] = [ event ]

		# update size of that part in smap
		self.smap.delete(smallest_part)
		self.smap.update((smallest_part_size+1, smallest_part_count), smallest_part_index)

		# update tmap if necessary
		selected_part_key = self.tmap.getKey(smallest_part_index)
		if selected_part_key is None: 
			self.tmap.update((event.event_time, smallest_part_count), smallest_part_index)
		elif selected_part_key[0] > event.event_time:
			self.tmap.delete(selected_part_key)
			self.tmap.update((event.event_time, selected_part_key[1]), smallest_part_index)

		self.balance()


	def pop(self):
		if self.empty(): return None

		# retrieve kds part, with earliest event
		earliest_time, earliest_part_index = self.tmap.getFirst()
		earliest_event_time = earliest_time[0]

		# retrieve the events from that part
		earliest_event_kds = self.kds[earliest_part_index]
		earliest_event = earliest_event_kds[earliest_event_time]
		
		# delete event from kds and from tmap
		del self.kds[earliest_part_index][earliest_event_time]

		self.tmap.delete(earliest_time)

		# insert in tmap the new earliest event for that part
		if len(self.kds[earliest_part_index]) > 0:
			new_earliest_event_time = self.kds[earliest_part_index].find_min()
			self.tmap.update((new_earliest_event_time, earliest_time[1]), earliest_part_index)

		# update size of that part in smap
		key_in_smap = self.smap.getKey(earliest_part_index)
		self.smap.delete(key_in_smap)
		self.smap.update((key_in_smap[0]-1, key_in_smap[1]), earliest_part_index)

		return earliest_event


	def balance(self):
		# retrieve the smallest kds part
		smallest_part, smallest_part_index = self.smap.getFirst()
		smallest_part_size, smallest_part_count = smallest_part

		# retrieve the largest kds part
		largest_part, largest_part_index = self.smap.getLast()
		largest_part_size, largest_part_count = largest_part

		if (largest_part_size - smallest_part_size) / largest_part_size < self.delta: return;

		transfer_size = int((largest_part_size - smallest_part_size)/2)
		if transfer_size == 0: return

		inserted_event_times = []

		for i in range(transfer_size):
			last_event_key = self.kds[largest_part_index].find_max()
			last_event = self.kds[largest_part_index][ last_event_key ]

			# delete from largest kds part
			del self.kds[largest_part_index][last_event_key]

			# insert in smallest kds part
			last_event_time = last_event[0].event_time
			inserted_event_times.append(last_event_time)

			if last_event_time in self.kds[smallest_part_index]:
				self.kds[smallest_part_index][last_event_time] += last_event
			else: self.kds[smallest_part_index][last_event_time] = last_event

		# update smap for largest kds part
		largest_part_key = self.smap.getKey(largest_part_index)
		self.smap.delete(largest_part_key)
		self.smap.update((largest_part_key[0] - transfer_size, largest_part_key[1]), largest_part_index)

		# update smap for smallest kds part
		smallest_part_key = self.smap.getKey(smallest_part_index)
		self.smap.delete(smallest_part_key)
		self.smap.update((smallest_part_key[0] + transfer_size, smallest_part_key[1]), smallest_part_index)

		# update tmap for smallest kds part if necessary
		earliest_new_event_time = min(inserted_event_times)
		smallest_part_key = self.tmap.getKey(smallest_part_index)
		if smallest_part_key is None:
			self.tmap.update((earliest_new_event_time, smallest_part_count), smallest_part_index)
		elif smallest_part_key[0] > earliest_new_event_time:
			self.tmap.delete(smallest_part_key)
			self.tmap.update((earliest_new_event_time, smallest_part_count), smallest_part_index)


	def status(self):
		print 'length of tmap: ', len(self.tmap.kv), len(self.tmap.vk)
		self.tmap.status()
		for i in range(len(self.kds)): 
			print len(self.kds[i]) 
		