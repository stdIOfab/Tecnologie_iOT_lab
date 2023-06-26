import requests
import json
from MyMQTT import MyMQTT
import time

class MQTTSubscriber:
    def __init__(self, domain, port):
        self.domain = domain
        self.port = port
        try:
            self.body = json.loads(requests.get("http://" + self.domain + ":" + str(self.port) + "/").text)
        except requests.exceptions.ConnectionError:
            print("Unable to connect to the catalog")
            exit(1)
        self.clientID = "MQTT_Subscriber"
        self.broker = self.body["subscriptions"]["MQTT"]["device"]["hostname"]
        self.port = self.body["subscriptions"]["MQTT"]["device"]["port"]
        self.myMqttClient = MyMQTT(self.clientID, self.broker, self.port, self)
    def registerService(self):
        sub_form = {}
        sub_form["id"] = "TiOT2Service"
        sub_form["description"] = "This service only subscribes to a topic in order to read data from a device"
        sub_form["endpoints"] = {}
        sub_form["endpoints"]["MQTT"] = {}
        sub_form["endpoints"]["REST"] = {}
        sub_form["endpoints"]["MQTT"]["PUBLISH"] = {}
        sub_form["endpoints"]["MQTT"]["SUBSCRIBE"] = {}
        sub_form["endpoints"]["REST"]["POST"] = {}
        sub_form["endpoints"]["REST"]["GET"] = {}
        sub_form["timestamp"] = time.time()
        try:
            requests.post(self.body["subscriptions"]["REST"]["service"], json.dumps(sub_form))
        except requests.exceptions.ConnectionError:
            print("Unable to add the service to the catalog")
            exit(1)
    def MQTTsubscribe(self):
        try:
            response = json.loads(requests.get(self.body["subscriptions"]["REST"]["device"]).text)
            id = response["devices"][0]["id"] #Assuming there is only one device
            response = requests.get("{url1}/{url2}".format(url1 = self.body["subscriptions"]["REST"]["device"], url2 = id))
        except Exception:
            print("Unable to connect to the catalog or to retrieve the device id")
            exit(1)
        topic = json.loads(response.text)
        self.myMqttClient.mySubscribe(topic["endpoints"]["MQTT"]["PUBLISH"]["temperature"])
    def notify(self, topic, msg):
        print("Message received: ", str(msg))

if __name__ == "__main__":
    subscriber = MQTTSubscriber("172.20.10.3", 8080)
    subscriber.registerService()
    subscriber.MQTTsubscribe()
    subscriber.myMqttClient.start()
    while True:
        time.sleep(1)










