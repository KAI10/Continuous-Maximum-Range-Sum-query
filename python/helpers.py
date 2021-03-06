from abc import ABCMeta, abstractmethod
import py_treap

class event_queue():
	"""Abstract class for all types of event queues"""
	__metaclass__ = ABCMeta
	
	@abstractmethod
	def push(self): 
		pass

	@abstractmethod
	def pop(self):
		pass

	@abstractmethod
	def empty(self):
		pass


class bidict:
    def __init__(self):
        self.kv = py_treap.treap()
        self.vk = dict()

    def update(self, key, value):
        self.kv[key] = value
        self.vk[value] = key

    def delete(self, key):
        value = self.kv[key]
        del self.kv[key]
        del self.vk[value]

    def getValue(self, key):
        if key in self.kv: return self.kv[key]
        return None

    def getKey(self, value):
        return self.vk.get(value, None)

    def getFirst(self):
        key = self.kv.find_min()
        return (key, self.kv[key])

    def getLast(self):
        key = self.kv.find_max()
        return (key, self.kv[key])

    def status(self):
        for key, value in self.kv.items(): print key[0], value