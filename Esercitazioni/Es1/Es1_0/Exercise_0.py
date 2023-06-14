from math import sqrt


class SquareManager:
    def __init__(self, side):
        self.side = side

    def area(self):
        return self.side ** 2

    def perimeter(self):
        return 4 * self.side

    def diagonal(self):
        return sqrt(2) * self.side
