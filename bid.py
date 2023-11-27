"""
Must send a heart-beat every X second. Otherwise, presume dead.

"""

from dataclasses import dataclass

HEART_BEAT_EVERY = 3
HEART_BEAT_GRACE_PERIOD = 3
CHECK_ON_EXISTING_AUCTION = 3
RANDOM_DELAY_WHEN_BIDDING = 0.2

import numpy as np
import time
import datetime

from dataclasses_json import dataclass_json


from consts import AuctionIncomingData, BidPlacementData


class DecentralisedBidding:
    def __init__(self, bid):
        self.bid = bid
        self.auction_history = {}
        self.my_current_bid = {}
        self.ongoing_auction = {}
        # self.

    def on_new_auction(self, auction: AuctionIncomingData):
        if auction.task_id in self.auction_history:
            # old colliding auction task_id??
            self.auction_history.pop(auction.task_id)

        # CALL BELOW IN:
        time.sleep(RANDOM_DELAY_WHEN_BIDDING)

        self.auction_history[auction.task_id] = auction

        self.ongoing_auction[auction.task_id] = auction

        # print(self.auction_history)

        my_bid = BidPlacementData(
            task_id=auction.task_id,
            uid=self.bid,
            bid_value=np.random.rand() * 10,
            bid_timestamp=datetime.datetime.now(),
        )

        self.my_current_bid[auction.task_id] = my_bid
        return my_bid

    def on_new_bid(self, incoming_bid: BidPlacementData):
        if incoming_bid.uid == self.bid:
            return

        if incoming_bid.task_id not in self.my_current_bid:
            # we have no interest in this auction.
            return

        if self.my_current_bid[incoming_bid.task_id].bid_value <= incoming_bid.bid_value:
            return
        # the bid is lower than our previous bid?
        # TODO: recompute bids value if the bid time is long (where we might have finished a different task by now?)

        # re-bid with the same amount.
        prev_bid = self.my_current_bid[incoming_bid.task_id]
        prev_bid.bid_timestamp = datetime.datetime.now()

        self.my_current_bid[incoming_bid.task_id] = prev_bid

        return prev_bid

        print(incoming_bid)

        pass

    def on_won_auction(
        self,
    ):
        pass
        # bid fulfilled?

    def fulfill_auction(self, auction_id):
        self.auction_history.pop(auction_id, None)
        # send auction fulfilled message
