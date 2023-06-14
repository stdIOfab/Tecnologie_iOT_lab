from math import sqrt


class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __str__(self):
        return f"{self.x}, {self.y}"

    def move(self, dx, dy):
        self.x += dx
        self.y += dy

    def distance(self, other):
        return sqrt((self.x - other.x) ** 2 + (self.y - other.y) ** 2)


class Line:
    def __init__(self, slope, intercept):
        self.slope = slope
        self.intercept = intercept

    def __str__(self):
        return f"y = {self.slope}x + {self.intercept}"

    def line_from_points(self, a, b):
        self.slope = (b.y - a.y) / (b.x - a.x)
        self.intercept = a.y - self.slope * a.x

    def distance(self, point):
        return abs(self.slope * point.x - point.y + self.intercept) / sqrt(self.slope ** 2 + 1)

    def intersection(self, other):
        x = (other.intercept - self.intercept) / (self.slope - other.slope)
        y = self.slope * x + self.intercept
        return Point(x, y)
