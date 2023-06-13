class Car(object): 
    def __init__(self, name):
        self._name  = name
        self.__speed = 0.0
        self.__gear = 1
    
    def __str__(self): 
        return f"{self.getName()}, {self.__speed}, {self.__gear}"
    
    def getName (self): 
        return self._name 
    
    def setSpeed (self, speed):
        if (speed > 250): 
            self.__speed = 250.0
        elif(speed < 0):
            self.__speed = 0.0
        else: 
            self.__speed = speed
    
    def gearUp(self):
        if(self.__gear + 1 > 6):
            raise ValueError("gear is already maximum")
        else: 
            self.__gear = self.__gear +1 
        
    def gearDown(self): 
        if(self.__gear - 1 < 1):
            raise ValueError("gear is already minimum")
        else:
            self.__gear = self.__gear - 1
        


        
        