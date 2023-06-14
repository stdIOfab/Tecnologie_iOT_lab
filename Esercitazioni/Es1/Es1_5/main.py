from Exercise_5 import *

if __name__ == "__main__":
    c = Car("Ferrari")
    print(c)
    c.setSpeed(100)
    print(c)
    c.setSpeed(300)
    print(c)
    try:
        for i in range(8):
            c.gearUp()
            print(c)
    except ValueError as err:
        print("Got value error:", err)
    try:
        for i in range(8):
            c.gearDown()
            print(c)
    except ValueError as err:
        print("Got value error:", err)
