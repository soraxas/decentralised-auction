
import cppimport.import_hook
import decentralised_bidding #This will pause for a moment to compile the module
print(decentralised_bidding.square(9))
81

bidder = decentralised_bidding.DecentralisedBidder("alex")

print(bidder)

exit()


import paho.mqtt.client as mqtt
import json

import consts

from icecream import ic
import bid
import datetime
import string
import sys
import random

def get_random_string(length):
    # choose from all lowercase letter
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(length))
    print("Random string of length", length, "is:", result_str)

if len(sys.argv) > 1:
    name = sys.argv[1]
else:
    name = get_random_string(8)

bidder = bid.DecentralisedBidding(name)
print(name)

# auction = bid.AuctionIncomingData(
#     "1",
#     datetime.datetime.now(),
#     datetime.datetime.now() + datetime.timedelta(seconds=50),
# )

# ic(bid.AuctionIncomingData.from_json(auction.to_json()))

# exit()

# bid_val = bidder.on_new_auction(auction)

# print(bid_val)

# exit()


# Define on_publish event function
def on_publish(client, userdata, mid):
    print("Message Published...")


def on_connect(client, userdata, flags, rc):
    client.subscribe(consts.MQTT_TOPIC_AUCTION)
    client.subscribe(consts.MQTT_TOPIC_AUCTION_BID)


def on_message(client, userdata, msg):

    if name == 'alex':
        payload = json.loads(
            msg.payload
        )  # you can use json.loads to convert string to json
        print((payload))  # then you can check the value

    if msg.topic == consts.MQTT_TOPIC_AUCTION:
        # print((msg.payload))
        payload = json.loads(msg.payload)
        auction = consts.AuctionIncomingData.from_json(payload)

        # print(auction)
        bid_val = bidder.on_new_auction(auction)
        # print(bid_val)
        client.publish(consts.MQTT_TOPIC_AUCTION_BID, json.dumps(bid_val.to_json()))

    if msg.topic == consts.MQTT_TOPIC_AUCTION_BID:
        payload = json.loads(msg.payload)
        # print(payload)

        incoming_bid = consts.BidPlacementData.from_json(payload)


        bid_val = bidder.on_new_bid(incoming_bid)

        if bid_val is not None:
            pass
            client.publish(consts.MQTT_TOPIC_AUCTION_BID, json.dumps(bid_val.to_json()))


        # print((msg.topic))
        # print((msg.payload))  # <- do you mean this payload = {...} ?
        payload = json.loads(
            msg.payload
        )  # you can use json.loads to convert string to json
        # print((payload))  # then you can check the value
        # # client.disconnect() # Got message then disconnect


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
