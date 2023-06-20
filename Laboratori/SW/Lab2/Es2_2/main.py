import os
import paho.mqtt.client as PahoMQTT
import time
import json
import cherrypy
import schedule
from threading import Thread

def inputMenu():
    while True:
        print(command_list)
        user_input = input()
        if user_input == "1":
            print(client_catalog.messageBroker + "\nPort: 5000\n")
        elif user_input == "2":
            client_catalog.GET("devices")
        elif user_input == "2":
            print('Type the deviceID: ')
            id = str(input())
            client_catalog.GET("devices", id)
        elif user_input == "4":
            client_catalog.GET("users")
        elif user_input == "5":
            print('Type the userID: ')
            id = str(input())
            client_catalog.GET("users", id)
        elif user_input == 'q':
            client_catalog.stop()
            cherrypy.engine.exit()
            exit(0)
        else:
            print('Unknown command')


def loopCheck():
    schedule.every(10).seconds.do(client_catalog.removeOld)
    while True:
        schedule.run_pending()
        time.sleep(1)
class CatalogREST:
    exposed = True

    def __init__(self, clientID, broker):
        self.clientID = clientID
        self._baseTopic = 'catalog'
        self._topic = self._baseTopic
        # create an instance of paho.mqtt.client
        self._paho_mqtt = PahoMQTT.Client(self.clientID, False)
        # register the callback
        self._paho_mqtt.on_connect = self.myOnConnect
        # info about registration
        self._infoSub = dict()
        self._infoSub["subscription"] = dict()
        # subscription REST
        self._infoSub["subscription"]["REST"] = dict()
        self._infoSub["subscription"]["REST"]["device"] = broker+"/devices/subscription"
        self._infoSub["subscription"]["REST"]["service"] = broker+"/services/subscription"
        self._infoSub["subscription"]["REST"]["user"] = broker+"/users/subscription"
        # subscription MQTT
        self._infoSub["subscription"]["MQTT"] = dict()
        self._infoSub["subscription"]["MQTT"]["device"] = dict()
        self._infoSub["subscription"]["MQTT"]["device"]["hostname"] = broker
        self._infoSub["subscription"]["MQTT"]["device"]["port"] = "5000"
        self._infoSub["subscription"]["MQTT"]["device"]["topic"] = "tiot/2/catalog/devices/subscription"
        # catalog
        self._registry = dict()
        self._registry['devices'] = dict()
        self._registry['users'] = dict()
        self._registry['services'] = dict()
        # self.messageBroker = 'iot.eclipse.org'
        self.messageBroker =broker
        print(self._registry)

    def start(self):
        #manage connection to broker
        self._paho_mqtt.connect(self.messageBroker, 5000)
        self._paho_mqtt.loop_start()

    def stop(self):
        self._paho_mqtt.loop_stop()
        self._paho_mqtt.disconnect()

    def publish(self, topic, msg):
        print("publishing to %s" % topic)
        self._paho_mqtt.publish(topic, msg, 2)

    def subscribe(self, topic):
        # if needed, you can do some computation or error-check before subscribing
        print("subscribing to %s" % topic)
        self._paho_mqtt.subscribe(topic, 2)
        self._isSubscriber = True
        self._topic = topic

    def myOnConnect (self, paho_mqtt, userdata, flags, rc):
        print ("Connected to %s with result code: %d" % (self.messageBroker, rc))

    def GET(self,*uri,**params):
        if len(uri) == 0 :
            return json.dumps(self._infoSub)
        elif len(uri) == 1 and uri[0] == 'devices' :   # all devices
            self._paho_mqtt.unsubscribe(self._topic)
            self._paho_mqtt.subscribe(self._baseTopic+"/"+"devices/#", 2)
            self._paho_mqtt.publish(self._baseTopic+"/"+"devices", json.dumps(self._registry['devices']), 2)
            print(self._registry['devices'])
            return json.dumps(self._registry['devices'])
        elif len(uri) == 1 and uri[0] == 'users' :   # all users
            self._paho_mqtt.unsubscribe(self._topic)
            self._paho_mqtt.subscribe(self._baseTopic+"/"+"users/#", 2)
            self._paho_mqtt.publish(self._baseTopic+"users/#", json.dumps(self._registry['users']), 2)
            return json.dumps(self._registry['users'])
        elif len(uri) == 1 and uri[0] == 'services' :   # all services
            self._paho_mqtt.unsubscribe(self._topic)
            self._paho_mqtt.subscribe(self._baseTopic+"/"+"services/#", 2)
            self._paho_mqtt.publish(self._baseTopic+"services/#", json.dumps(self._registry['services']), 2)
            return json.dumps(self._registry['services'])
        elif len(uri) == 2 and uri[0] == 'devices' :   # specific device
            self._paho_mqtt.unsubscribe(self._topic)
            self._paho_mqtt.subscribe(self._baseTopic+"/"+"devices/"+uri[1], 2)
            self._paho_mqtt.publish(self._baseTopic+"devices/"+uri[1], json.dumps(self._registry['devices']['dev'+uri[1]]), 2)
            return 'Device {} : {}'.format(uri[1], self._registry['devices']['dev'+uri[1]])
        elif len(uri) == 2 and uri[0] == 'user' :   # specific user
            self._paho_mqtt.unsubscribe(self._topic)
            self._paho_mqtt.subscribe(self._baseTopic+"/"+"users/" + uri[1], 2)
            self._paho_mqtt.publish(self._baseTopic+"users/" + uri[1], json.dumps(self._registry['users']['usr'+uri[1]]), 2)
            return 'User {} : {}'.format(uri[1], self._registry['users']['usr'+uri[1]])
        elif len(uri) == 2 and uri[0] == 'service' :   # specific service
            self._paho_mqtt.unsubscribe(self._topic)
            self._paho_mqtt.subscribe(self._baseTopic+"/"+"services/" + uri[1], 2)
            self._paho_mqtt.publish(self._baseTopic+"services/" + uri[1], json.dumps(self._registry['services']['ser'+uri[1]]), 2)
            return 'Service {} : {}'.format(uri[1], self._registry['services']['ser'+uri[1]])


    def POST(self, *uri):
        req = cherrypy.request.body.read()
        if len(uri) == 2 and uri[0] == 'devices' and uri[1] == 'subscription' :
            deviceData = json.loads(req)
            if deviceData["deviceID"] not in self._registry['devices'] :
                self.register_device(deviceData)
        if len(uri) == 2 and uri[0] == 'users' and uri[1] == 'subscription' :
            userData = json.loads(req)
            if userData["userID"] not in self._registry['users'] :
                self.register_user(userData)
        if len(uri) == 2 and uri[0] == 'services' and uri[1] == 'subscription' :
            serviceData = json.loads(req)
            if serviceData["serviceID"] not in self._registry['services'] :
                self.register_service(serviceData)

    def removeOld(self):
        t = time.time()
        print(self._registry['devices'])
        cancel_list = []
        for dev in self._registry['devices']:
            if t - float(self._registry['devices'][dev]['timestamp']) > 20:
                cancel_list.append(dev)
        for dev in cancel_list:
            self._registry['devices'].pop(dev)

        cancel_list.clear()

        for ser in self._registry['services']:
            if t - float(self._registry['services'][ser]['timestamp']) > 20:
                cancel_list.append(ser)
        for ser in cancel_list:
            self._registry['services'].pop(ser)

    def register_device(self, deviceJson):
        deviceJson['timestamp'] = str(round(time.time(), 2))
        self._registry['devices']['dev'+deviceJson['deviceID']] = dict()
        self._registry['devices']['dev'+deviceJson['deviceID']]['endpoints'] = deviceJson['endpoints']
        self._registry['devices']['dev'+deviceJson['deviceID']]['availableRes'] = deviceJson['availableRes']
        self._registry['devices']['dev'+deviceJson['deviceID']]['timestamp'] = deviceJson['timestamp']

    def register_user(self, userJson):
        self._registry['users']['usr'+userJson['userID']] = dict()
        self._registry['users']['usr'+userJson['userID']]['name'] = userJson['name']
        self._registry['users']['usr'+userJson['userID']]['surname'] = userJson['surname']
        self._registry['users']['usr'+userJson['userID']]['emailAddresses'] = userJson['emailAddresses']

    def register_service(self, serviceJson):
        serviceJson['timestamp'] = str(time.time())
        self._registry['services']['ser'+serviceJson['serviceID']]['description'] = serviceJson['description']
        self._registry['services']['ser'+serviceJson['serviceID']]['endpoints'] = serviceJson['endpoints']
        self._registry['services']['ser'+serviceJson['serviceID']]['timestamp'] = serviceJson['timestamp']


if __name__ == "__main__":
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tool.session.on': True,
            'tools.staticdir.root': os.path.abspath(os.getcwd())

        }}
    client_catalog = CatalogREST("CatalogClient", '127.0.0.1')
    cherrypy.tree.mount(client_catalog, '/', conf)
    cherrypy.engine.start()

    client_catalog.start()
    print('Welcome!\n')

    command_list = 'Type:\n"1" to retrieve info about registering and MQTT broker\n"2" to retrieve all registered devices\n' \
                   '"3" to retrieve a device with a specific deviceID\n"4" to retrieve all registered users\n"5" to retrieve a user with a specific userID\n' \
                   '"q" to quit'

    Thread(target=inputMenu).start()
    Thread(target=loopCheck).start()
