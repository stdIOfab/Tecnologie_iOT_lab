import requests
import json
from MyMQTT import MyMQTT
import time

host = "172.20.10.2"
port = 8080

class MQTTPublisher:
    def __init__(self, domain, port):
        self.domain = domain
        self.port = port
        try:
            self.body = json.loads(requests.get("http://" + self.domain + ":" + str(self.port) + "/").text)
        except requests.exceptions.ConnectionError:
            print("Unable to connect to the catalog")
            exit(1)
        self.clientID = "MQTT_Publisher"
        self.broker = self.body["subscriptions"]["MQTT"]["device"]["hostname"]
        self.port = self.body["subscriptions"]["MQTT"]["device"]["port"]
        self.myMqttClient = MyMQTT(self.clientID, self.broker, self.port, self)
        self.myMqttClient.start()

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

    def MQTTpublish(self):
        try:
            response = requests.get(self.body["subscriptions"]["REST"]["device"])
            id = json.loads(response.text)["devices"][0]["id"] #Assuming there is only one device
            response = requests.get(self.body["subscriptions"]["REST"]["device"]+"/"+id)
        except (requests.exceptions.ConnectionError, json.decoder.JSONDecodeError):
            print("Unable to connect to the catalog or to retrieve the device id")
            exit(1)
        topic = json.loads(response.text)["endpoints"]["MQTT"]["PUBLISH"]["temperature"]
        self.myMqttClient.mySubscribe(topic)

    def notify(self, topic, msg):
        print("Message received: ", msg)

if __name__ == "__main__":
    pub_client = MQTTPublisher(host, port)
    pub_client.registerService()
    pub_client.MQTTsubscribe()
