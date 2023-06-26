import os
import sys
import json
import time
import threading
import requests
from MyMQTT import MyMQTT


class Loop(threading.Thread):

    def __init__(self, timeLoop, id, description, endpoint, broker, port):
        threading.Thread.__init__(self)
        self.time_ = time
        self.broker = broker
        self.port = port
        self.request_json = {
            "id": id,
            "description": description,
            "endpoint": endpoint
        }

        self.myMqttClient = MyMQTT(id, broker, port, self)
        self.myMqttClient.start()

    def run(self):
        while True:
            self.myMqttClient.myPublish(endpoint[0], (json.dumps(self.request_json)))
            time.sleep(self.time)


class Service():
    exposed = True

    def __init__(self, id, broker, port):
        # self.values_arduino_yun = list()

        self.messageBroker = broker
        self.port = port

        self.myMqttClient = MyMQTT(id, self.messageBroker, self.port, self)
        self.myMqttClient.start()
        # self.myMqttClient.mySubscribe("/tiot/16/GET/devices/+/response")
        # self.myMqttClient.mySubscribe("/tiot/16/service/val/luce/+")
        # self.info_device("Yun_16")
        # self.info_device("bright_01")

    def registerService(self):
        sub_form = {}
        sub_form["id"] = "TiOT2Service"
        sub_form["description"] = "This service only subscribes to a topic in order to read data from a device"
        sub_form["endpoints"]["MQTT"]["PUBLISH"] = {}
        sub_form["endpoints"]["MQTT"]["SUBSCRIBE"] = {}
        sub_form["endpoints"]["REST"]["POST"] = {}
        sub_form["endpoints"]["REST"]["GET"] = {}
        sub_form["timestamp"] = time.time()
        try:
            response = requests.post(self.body["subscriptions"]["REST"]["service"], json.dumps(sub_form))
        except requests.exceptions.ConnectionError:
            print("Unable to add the service to the catalog")
            exit(1)

    #def info_device(self, id_):
#
    #    self.myMqttClient.myPublish(f"/tiot/16/GET/devices/{id_}")

    def notify(self, topic, msg):

        topic_list = topic.split("/")
        len_ = len(topic_list)
        type_ = ""

        if len_ > 4:
            type_ = topic_list[4]

        #if type_ == "devices":
        #    jsonb = json.loads(msg)
        #    if jsonb["id"] == "bright_01":
        #        self.myMqttClient.mySubscribe(jsonb["endpoint"][0])
        #    else:
        #        self.topic_luce = json.loads(msg)["endpoint"][0]

        if type_ == "val":
            dati = {'bn': 'Yun', 'e': [{'n': 'led', 't': None, 'v': int(topic_list[6]), 'u': None}]}
            self.myMqttClient.myPublish(endpoint[0], json.dumps(dati))

        #if topic_list[4] == "brightness":
        #    valLuce = json.loads(msg)["e"][0]["v"]
        #    if valLuce < 200:
        #        dati = {'bn': 'Yun', 'e': [{'n': 'led', 't': None, 'v': 1, 'u': None}]}
        #        self.myMqttClient.myPublish("/tiot/16/yun/lampadina", json.dumps(dati))
        #    else:
        #        dati = {'bn': 'Yun', 'e': [{'n': 'led', 't': None, 'v': 0, 'u': None}]}
        #        self.myMqttClient.myPublish("/tiot/16/yun/lampadina", json.dumps(dati))


if __name__ == "__main__":
    id = "LedTIoT_2"
    description = "Service MQTT"
    endpoint = ["/tiot/16/service/val/luce/+"]

    service = Service(id, "test.mosquitto.org", 8080 )

    loop_request = Loop(15, id=id, description=description, endpoint=endpoint, broker="test.mosquitto.org", port=8080)
    loop_request.start()