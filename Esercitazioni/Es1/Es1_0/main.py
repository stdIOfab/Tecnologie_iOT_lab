from Exercise_0 import *

if __name__ == "__main__":
    sm = SquareManager(3)
    print(f"The area of the square with side {sm.side} = {sm.area()}")
    print(f"The perimeter of the square with side {sm.side} = {sm.perimeter()}")
    print(f"The diagonal of the square with side {sm.side} = {sm.diagonal():.3f}")
