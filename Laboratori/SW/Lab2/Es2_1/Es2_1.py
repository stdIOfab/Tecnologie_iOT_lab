import os
import time
import threading
import schedule
import json
import cherrypy

HOST = "127.0.0.1"
HOST_PORT = 8080


class CatalogREST:
    exposed = True
    dataFile = "data.json"
    thread_lock = threading.Lock()

    def __init__(self, domain, port, currentHost, hostPort):
        self.messagebroker = {
            "domain": domain,
            "port": port
        }
        self.hostPort = hostPort
        self.currentHost = currentHost
        self.devices = []
        self.users = []
        self.services = []

        self.readVal()

    def searchVal(self, inputBody, type):
        idVal = inputBody["id"]
        if type == "user":
            for e in self.users:
                if e["id"] == idVal:
                    break
            else:
                self.users.append(inputBody)
        if type == "device":
            for n, e in enumerate(self.devices):
                if e["id"] == idVal:
                    self.devices[n]["timestamp"] = inputBody["timestamp"]
                    break
            else:
                self.devices.append(inputBody)
        if type == "service":
            for n, e in enumerate(self.services):
                if e["id"] == idVal:
                    self.services[n]["timestamp"] = inputBody["timestamp"]
                    break
            else:
                self.services.append(inputBody)

    def readVal(self):
        self.thread_lock.acquire()
        with open(self.dataFile, "r") as f:
            dataDict = json.loads(f.read())

        self.devices = dataDict["devices"]
        self.users = dataDict["users"]
        self.services = dataDict["services"]

        self.thread_lock.release()

    def storeVal(self):
        self.thread_lock.acquire()
        registry = {
            "users": self.users,
            "devices": self.devices,
            "services": self.services
        }

        with open(self.dataFile, "w") as f:
            f.write(f"{json.dumps(registry)}")

        self.thread_lock.release()

    @staticmethod
    def searchID(list_, id):
        for element in list_:
            if element["id"] == id:
                return element
        return None

    def removeOld(self, leasingTime):
        currentTime = time.time()
        if len(self.devices) > 0:
            for i, devInfo in enumerate(self.devices):
                if currentTime - devInfo.get("timestamp") >= leasingTime:
                    self.devices.pop(i)
        if len(self.services) > 0:
            for i, serInfo in enumerate(self.services):
                if currentTime - serInfo.get("timestamp") >= leasingTime:
                    self.services.pop(i)
        self.storeVal()

    def GET(self, *uri, **params):
        if len(uri) == 0:
            return f'"subscriptions":{{' \
                   '"REST":{' \
                   f'    "device":"http://{self.currentHost}:{self.hostPort}/devices/subscription",' \
                   f'    "service": "http://{self.currentHost}:{self.hostPort}/services/subscription"' \
                   f'    "user":"http://{self.currentHost}:{self.hostPort}/users/subscription"}},' \
                   '"MQTT":{' \
                   '"device":{' \
                   f'    "hostname":{self.messagebroker["domain"]},' \
                   f'    "port":{self.messagebroker["port"]},' \
                   '    "topic":"/tiot/2/GET/devices/subscription"}}}'

        if uri[0] == "messagebroker":
            return json.dumps(self.messagebroker, indent=4)

        elif uri[0] == "devices":
            if len(uri) == 1:
                return json.dumps({"devices": self.devices}, indent=4)
            else:
                device = CatalogREST.searchID(self.devices, uri[1])
                if device is None:
                    device = f"No device found with the given ID {uri[1]}"
                else:
                    device = json.dumps(device, indent=4)
                return device

        elif uri[0] == "users":
            if len(uri) == 1:
                return json.dumps({"users": self.users}, indent=4)
            else:
                user = CatalogREST.searchID(self.users, uri[1])
                if user is None:
                    user = f"No user found with the given ID {uri[1]}"
                else:
                    user = json.dumps(user, indent=4)
                return user

        elif uri[0] == "services":
            if len(uri) == 1:
                return json.dumps({"services": self.services}, indent=4)
            else:
                service = CatalogREST.searchID(self.services, uri[1])
                if service is None:
                    service = f"No service found with the given ID {uri[1]}"
                else:
                    service = json.dumps(service, indent=4)
                return service
        else:
            cherrypy.HTTPError(404, "Request not found on this server")

    def POST(self, *uri, **params):
        if len(uri) != 1:
            cherrypy.HTTPError(400, "Bad request")

        if uri[0] == "devices" and uri[1] == "subscription":
            body = json.loads(cherrypy.request.body.read().decode("utf-8"))
            body["timestamp"] = time.time()
            self.searchVal(body, "device")

            self.storeVal()

        elif uri[0] == "users" and uri[1]== "subscription":
            body = json.loads(cherrypy.request.body.read().decode("utf-8"))
            self.searchVal(body, "user")

            self.storeVal()

        elif uri[0] == "services" and uri[1]== "subscription":
            body = json.loads(cherrypy.request.body.read().decode("utf-8"))
            body["timestamp"] = time.time()
            self.searchVal(body, "service")

            self.storeVal()

        else:
            cherrypy.HTTPError(404, "Request not found on this server")

def loopCheck(classObject:CatalogREST, leasingTime=120):
    schedule.every(10).seconds.do(classObject.removeOld, leasingTime)
    while True:
        schedule.run_pending()
        time.sleep(1)


if __name__ == "__main__":
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tools.sessions.on': True,
            'tools.staticdir.root': os.path.abspath(os.getcwd())
        }
    }
    client_catalog = CatalogREST("mqtt.eclipse.org", 1883, HOST, HOST_PORT)
    cherrypy.tree.mount(client_catalog, '/', conf)
    cherrypy.config.update({'server.socket_host': HOST})
    cherrypy.config.update({'server.socket_port': HOST_PORT})
    cherrypy.engine.start()
    loopCheck(client_catalog, 120)
    cherrypy.engine.block()


