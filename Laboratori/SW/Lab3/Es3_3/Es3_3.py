import requests
import json
from MyMQTT import MyMQTT
import time

class MQTTPublisher:
    def __init__(self, domain, port, serviceName):
        self.domain = domain
        self.port = port
        self.serviceName = serviceName
        self.ledStatus = 0
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

    def MQTTpublish(self):
        try:
            response = json.loads(requests.get(self.body["subscriptions"]["REST"]["device"]).text)
            id = response["devices"][0]["id"]  # Assuming there is only one device
            response = requests.get("{url1}/{url2}".format(url1=self.body["subscriptions"]["REST"]["device"], url2=id))
        except Exception:
            print("Unable to connect to the catalog or to retrieve the device id")
            exit(1)
        topic = json.loads(response.text)

        payload = f'{{"bn": "{self.serviceName}", "e": [{{"n": "led", "u": "null", "v": {self.ledStatus}}}]}}'
        self.myMqttClient.myPublish(topic["endpoints"]["MQTT"]["SUBSCRIBE"]["led"], payload)


    def notify(self, topic, msg):
        print("Message received: ", msg)

    def changeStatus(self):
        if self.ledStatus == 0:
            self.ledStatus = 1
        else:
            self.ledStatus = 0

if __name__ == "__main__":
    pub_client = MQTTPublisher("172.20.10.3", 8080, "service1")
    pub_client.registerService()
    while True:
        time.sleep(2)
        pub_client.MQTTpublish()
        pub_client.changeStatus()
