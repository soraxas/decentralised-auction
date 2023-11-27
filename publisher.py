import paho.mqtt.client as mqtt
import json

import datetime

import consts

from icecream import ic


new_auction = consts.AuctionIncomingData(
    "1",
    datetime.datetime.now(),
    datetime.datetime.now() + datetime.timedelta(seconds=50),
)

ic(consts.AuctionIncomingData.from_json(new_auction.to_json()))


# Define on_publish event function
def on_publish(client, userdata, mid):
    print("Message Published...")


def on_connect(client, userdata, flags, rc):
    # client.subscribe(MQTT_TOPIC)
    client.publish(consts.MQTT_TOPIC_AUCTION, json.dumps(new_auction.to_json()))


def on_message(client, userdata, msg):
    print((msg.topic))
    print((msg.payload))  # <- do you mean this payload = {...} ?
    payload = json.loads(
        msg.payload
    )  # you can use json.loads to convert string to json
    print((payload["sepalWidth"]))  # then you can check the value
    # client.disconnect() # Got message then disconnect


# Initiate MQTT Client
mqttc = mqtt.Client()

# Register publish callback function
mqttc.on_publish = on_publish
mqttc.on_connect = on_connect
mqttc.on_message = on_message

# Connect with MQTT Broker
mqttc.connect(consts.MQTT_HOST, consts.MQTT_PORT, consts.MQTT_KEEPALIVE_INTERVAL)

# Loop forever
mqttc.loop_forever()
