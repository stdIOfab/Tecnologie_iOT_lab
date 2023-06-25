from MyMQTT import MyMQTT
import time
import json


class DevIoTPublisher():

    def __init__(self, broker, port, clientID, endpoint, resources):
        self.broker = broker
        self.port = port
        self.clientID = clientID
        self.endpoint = endpoint
        self.resources = resources
        self.myMqttClient = MyMQTT(self.clientID, self.broker, self.port, self)
        self.myMqttClient.start()

    def register_or_refresh(self):
        dataDev = {"id": self.clientID, "endpoint": self.endpoint, "resource": self.resources}
        while True:
            t = time.time()
            dataDev["timestamp"] = t
            self.myMqttClient.myPublish("/tiot/2/catalog/subscription/devices/subscription/" + dataDev['id'] + '/response', json.dumps(dataDev))
            time.sleep(60)

if __name__ == "__main__":
    d = DevIoTPublisher("test.mosquitto.org", 1883, "publisher_tiot_2", ["endpoint1", "endpoint2"], "temp")
    d.register_or_refresh()