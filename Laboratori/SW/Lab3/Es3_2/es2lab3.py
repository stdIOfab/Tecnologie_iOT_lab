import paho.mqtt.client as mqtt
import requests 

broker_address = "127.0.0.1"
port = 1883

url = ""

def get_end_point():
    response = requests.get(url +"device/{deviceID}")
    
    if response == 200: 
        data = response.json()
        endpoint = data["mqtt_endpoint"]

        return endpoint
    else: 
        return "NULL"


def on_my_connect(client, userdata, flag, rc): 
    print("Connesso" +str(rc))
    client.subscribe(get_end_point())


def on_my_message(client, userdata, msg):
    temp = float(msg.payload.decode("utf-8"))
    client.publish("temperatura: " , str(temp))


client = mqtt.Client(); 

client.on_connect = on_my_connect
client.on_message = on_my_message

client.connect(broker_address, port, 60)

client.loop_start()

response_reg = requests.post(url+ "/services" , json= {"service_type" : "mqtt_subscriber"})

if response_reg == 201: 
    print("aggiunto")
else: 
    print("non aggiunto")


response_sub = requests.get(url +"/subscriptions")

if response_reg== 200: 
    sub_data = response_sub.json()
    print("subscribe:", sub_data)

else: 
    print("nessuna subscribe")

while True: 
    pass








