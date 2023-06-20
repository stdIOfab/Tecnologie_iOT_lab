import os
import paho.mqtt.client as PahoMQTT
import time
import json
import cherrypy

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
        self._registry = dict()
        self._registry['devices'] = dict()
        self._registry['users'] = dict()
        self._registry['services'] = dict()
        # self.messageBroker = 'iot.eclipse.org'
        self.messageBroker =broker

    def start(self):
        #manage connection to broker
        self._paho_mqtt.connect(self.messageBroker, 5000)
        self._paho_mqtt.loop_start()

    def stop(self):
        self._paho_mqtt.loop_stop()
        self._paho_mqtt.disconnect()

    def publish(self, topic, msg):
        print("publishing to %s" % (topic))
        self._paho_mqtt.publish(topic, msg, 2)

    def subscribe(self, topic):
        # if needed, you can do some computation or error-check before subscribing
        print("subscribing to %s" % (topic))
        self._paho_mqtt.subscribe(topic, 2)
        self._isSubscriber = True
        self._topic = topic

    def myOnConnect (self, paho_mqtt, userdata, flags, rc):
        print ("Connected to %s with result code: %d" % (self.messageBroker, rc))

    def GET(self,*uri,**params):
        if len(uri) == 0 :
            return 'REST APIs or MQTT info for subscribing'
        elif len(uri) == 1 and uri[0] == 'devices' :   # all devices
            self._paho_mqtt.unsubscribe(self._topic)
            self._paho_mqtt.subscribe(self._baseTopic+"/"+"devices/#", 2)
            self._paho_mqtt.publish(self._baseTopic+"/"+"devices", json.dumps(self._registry['devices']), 2)
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
            self._paho_mqtt.publish(self._baseTopic+"devices/"+uri[1], json.dumps(self._registry['devices'][uri[1]]), 2)
            return 'Device {} : {}'.format(uri[1], self._registry['devices'][uri[1]])
        elif len(uri) == 2 and uri[0] == 'user' :   # specific user
            self._paho_mqtt.unsubscribe(self._topic)
            self._paho_mqtt.subscribe(self._baseTopic+"/"+"users/" + uri[1], 2)
            self._paho_mqtt.publish(self._baseTopic+"users/" + uri[1], json.dumps(self._registry['users'][uri[1]]), 2)
            return 'User {} : {}'.format(uri[1], self._registry['users'][uri[1]])
        elif len(uri) == 2 and uri[0] == 'service' :   # specific service
            self._paho_mqtt.unsubscribe(self._topic)
            self._paho_mqtt.subscribe(self._baseTopic+"/"+"services/" + uri[1], 2)
            self._paho_mqtt.publish(self._baseTopic+"services/" + uri[1], json.dumps(self._registry['services'][uri[1]]), 2)
            return 'Service {} : {}'.format(uri[1], self._registry['services'][uri[1]])


    def POST(self, *uri):
        self._req = cherrypy.request.body.read()
        if len(uri) == 2 and uri[0] == 'devices' and uri[1] == 'subscription' :
            deviceData = json.loads(self._req)
            self.register_device(deviceData)
        if len(uri) == 2 and uri[0] == 'users' and uri[1] == 'subscription' :
            userData = json.loads(self._req)
            self.register_user(userData)
        if len(uri) == 2 and uri[0] == 'services' and uri[1] == 'subscription' :
            serviceData = json.loads(self._req)
            self.register_service(serviceData)

    def register_device(self, deviceJson):
        deviceJson['timestamp'] = str(round(time.time(), 2))
        self._registry['devices'][deviceJson['deviceID']] = (deviceJson['endpoints'], deviceJson['availableRes'], deviceJson['timestamp'])

    def register_user(self, userJson):
        self._registry['users'][userJson['userID']] = (userJson['name'], userJson['surname'], userJson['emailAddresses'])

    def register_service(self, serviceJson):
        serviceJson['timestamp'] = str(time.time())
        self._registry['users'][serviceJson['serviceID']] = (serviceJson['description'], serviceJson['endpoints'], serviceJson['timestamp'])


if __name__ == "__main__":
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tool.session.on': True,
            'tools.staticdir.root': os.path.abspath(os.getcwd())

        }}
    cherrypy.tree.mount(CatalogREST("CatalogClient", '127.0.0.1'), '/', conf)
    cherrypy.engine.start()

    client_catalog = CatalogREST("CatalogClient", '127.0.0.1')
    client_catalog.start()
    client_catalog.stop()

