import math

class Circle :

    def __init__(self, r):
        self._radius = r

    def __str__(self):
        return "A circle of radius {}".format(self._radius)

    def surface(self):
        return (self._radius**2) * math.pi

    def perimeter(self):
        return 2 * math.pi * self._radius

class Cylinder(Circle) :

    def __init__(self, r, h):
        super().__init__(r)
        self._height = h

    def __str__(self):
        return "A cylinder of radius {} and height {}".format(self._radius, self._height)

    def surface(self):
        return (super().surface()*2) + (super().perimeter()*self._height)

    def volume(self):
        return super().surface() * self._height