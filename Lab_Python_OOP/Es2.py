from math import *
from Es1 import *

class Line :

    def __init__(self, m, q):
        self._coefficients = [m, q]

    def __str__(self):
        return 'y=' + str(self._coefficients[0]) + 'x + ' + str(self._coefficients[1])

    def line_from_points(self, point1 : Point, point2 : Point):
        self._coefficients[0] = ((point2._coordinates[1] - point1._coordinates[1]) / (point2._coordinates[0] - point1._coordinates[0]))
        self._coefficients[1] = (point1._coordinates[1] - ((point2._coordinates[1] - point1._coordinates[1]) / (point2._coordinates[0] - point1._coordinates[0])) * point1._coordinates[0])

    def distance(self, point : Point):
        return ((abs(point._coordinates[1] - (self._coefficients[0]*point._coordinates[0]) - self._coefficients[1])) / sqrt(1 + self._coefficients[0]**2))

    def intersection(self, line) -> Point:
        x = (line._coefficients[1] - self._coefficients[1]) / (self._coefficients[0] - line._coefficients[0])
        y = (self._coefficients[0] * (line._coefficients[1] - self._coefficients[1]) / (self._coefficients[0] - line._coefficients[0])) + self._coefficients[1]
        return Point(x, y)