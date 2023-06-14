from math import pi


class Circle:
    def __init__(self, radius):
        self.radius = radius

    def __str__(self):
        return f"A Circle of radius: {self.radius}"

    def surface(self):
        return pi * self.radius ** 2

    def perimeter(self):
        return pi * self.radius * 2

class Cylinder(Circle):
    def __init__(self, radius, height):
        super().__init__(radius)
        self.height = height

    def __str__(self):
        return f"A Cylinder of radius: {self.radius} and height: {self.height}"

    def surface(self):
        return 2 * super().surface() + self.height * super().perimeter()

    def volume(self):
        return super().surface() * self.height

