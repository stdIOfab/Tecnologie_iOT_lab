from math import *

class SquareManager :
    def __init__(self, side):
        self._side = side

    def area(self):
        self._area = self._side ** 2
        return self._area

    def perimeter(self):
        self._perimeter = self._side * 4
        return self._perimeter

    def diagonal(self):
        self._diagonal = self._side * sqrt(2)
        return self._diagonal