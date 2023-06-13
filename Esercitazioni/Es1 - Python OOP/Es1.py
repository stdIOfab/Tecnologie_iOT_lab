from math import *

class Point :

    def __init__(self, x, y):
        self._coordinates = [x, y]

    def __str__(self):
        return str(self._coordinates[0]) + ',' + str(self._coordinates[1])

    def distance(self, point):
        return sqrt((point._coordinates[0] - self._coordinates[0])**2 + (point._coordinates[1] - self._coordinates[1])**2)

    def move(self, x, y):
        self._coordinates[0] += x
        self._coordinates[1] += y