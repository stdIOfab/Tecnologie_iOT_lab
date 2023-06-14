from Exercise_2 import *

if __name__ == "__main__":
    l = Line(3, 2)
    print(l)
    a = Point(0, 1)
    b = Point(2, 2)
    l.line_from_points(a, b)
    print(l)
    l = Line(1, 0)
    a = Point(1, 5)
    print(l.distance(a))
    m = Line(-1, 1)
    i = l.intersection(m)
    print(i)
