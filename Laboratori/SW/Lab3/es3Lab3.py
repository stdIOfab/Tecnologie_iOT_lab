import paho.mqtt.client as mqtt 
import time
import json 
import requests 

global led

mqtt_broker_add = "127.0.0.1"
mqtt_port = 1883 
mqtt_topic = "len/switch"

#indirizzo ip del catalog da mettere
ip = "" 
port = 80
catalog_device = f"http://{ip}:{port}/device"
catalog_subscribe =  f"http://{ip}:{port}/subscribe"
catalog_register = f"http://{ip}:{port}/register"

#controllare attributi per l'arduino 

def get_sub():
    check = requests.get(catalog_subscribe)

    if check == 200 :
        print("sottoscrivibile")
    else: 
        print("non sottoscrivibile")



def device_reg():

    specific_device = {
        "ID" : "ArduinoGroup2",
        "Service" : "mqtt_publisher",
        "Topic": [mqtt_topic],

    }
    check = requests.post(catalog_register, json = specific_device)
    if check==200 : 
        print("dispositivo registato")
    else: 
        print("dispositivo non registato")


def on_my_connect(client,userdata,rc): 
    print("connessione con stato: " + str(rc))
    client.subscribe(mqtt_topic)

def on_my_message(client,userdata, msg): 
    payload = msg.payload.decode("utf-8")
    print(f"messagio ricevuto: {payload}")


client = mqtt.Client()
client.on_connect = on_my_connect
client.message = on_my_message
client.connect(mqtt_broker_add, mqtt_port)

device_reg()

get_sub()



def switch_led(): 
    led = not led

    if led : 
        state = "on"
        client.publish(mqtt_topic,state)

    else: 
        print("off")
        client.publish(mqtt_topic,state)
    

client.loop_start()

try:

    while True: 
     switch_led()
     time.sleep(15)

except KeyboardInterrupt:     
    pass


client.disconnect()
client.loop_stop()








