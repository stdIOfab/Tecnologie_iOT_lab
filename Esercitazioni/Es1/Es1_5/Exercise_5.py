class Car:
    def __init__(self, name):
        self.__name = name
        self.__speed = 0.0
        self.__gear = 1

    def __str__(self):
        return "Car: %s Speed: %.1f Gear: %d" % (self.__name, self.__speed, self.__gear)

    def setSpeed(self, speed: float):
        if speed < 0:
            self.__speed = 0
        elif speed > 250:
            self.__speed = 250
        else:
            self.__speed = speed

    def gearUp(self):
        if self.__gear == 5:
            raise ValueError("Gear is already at maximum")
        self.__gear += 1

    def gearDown(self):
        if self.__gear == 1:
            raise ValueError("Gear is already at minimum")
        self.__gear -= 1
