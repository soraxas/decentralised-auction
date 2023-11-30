from dataclasses import dataclass
from dataclasses_json import dataclass_json
import datetime

# Define Variables
MQTT_HOST = "localhost"
MQTT_PORT = 1883
MQTT_KEEPALIVE_INTERVAL = 45
MQTT_TOPIC_AUCTION = "auction"
MQTT_TOPIC_AUCTION_BID = "auction/bid"


@dataclass_json
@dataclass
class AuctionIncomingData:
    task_id: str
    creation_time: datetime.datetime
    expire_time: datetime.datetime


@dataclass_json
@dataclass
class BidPlacementData:
    task_id: str
    bidder_id: str
    bid_value: float
    timestamp: datetime.datetime
