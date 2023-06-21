import requests

HOST = "127.0.0.1"
PORT = "8080"

class fakeDevice():

    def __init__(self, host, port):
        self.url = host
        self.port = port

    def activeDevices(self):
        r = requests.get("http://" + self.url + ":" + self.port + "/devices")
        print(r.text)

    def findDevice(self, deviceID):
        r = requests.get("http://" + self.url + ":" + self.port + "/devices/" + deviceID)
        print(r.text)

    def activeUsers(self):
        r = requests.get("http://" + self.url + ":" + self.port + "/users")
        print(r.text)

    def findUser(self, userID):
        r = requests.get("http://" + self.url + ":" + self.port + "/users/" + userID)
        print(r.text)

    def activeServices(self):
        r = requests.get("http://" + self.url + ":" + self.port + "/services")
        print(r.text)

    def findService(self, serviceID):
        r = requests.get("http://" + self.url + ":" + self.port + "/services/" + serviceID)
        print(r.text)

    def getBroker(self):
        r = requests.get("http://" + self.url + ":" + self.port + "/messagebroker")
        print(r.text)

if __name__ == "__main__":

    client = fakeDevice(HOST, PORT)

    menu = "Available commands:\n 1) Retrieve message broker info\n 2) Find active devices\n 3) Find a specific device " \
           "using ID\n 4) Find active users\n 5) Find a specific user using ID\n 6) Find active services\n 7) Find " \
           "a specific service using ID \n q) exit\n"

    print(menu)

    while True:
        try:
            i = input("Insert command: ")
            if i == "1":
                client.getBroker()
            elif i == "2":
                client.activeDevices()
            elif i == "3":
                client.findDevice(input("Insert device ID to search: "))
            elif i == "4":
                client.activeUsers()
            elif i == "5":
                client.findUser(input("Insert user ID to search: "))
            elif i == "6":
                client.activeServices()
            elif i == "7":
                client.findService(input("Insert service ID to search: "))
            elif i == "q":
                break
            else:
                print("Invalid command, try again.")
        except Exception as e:
            print(e)

