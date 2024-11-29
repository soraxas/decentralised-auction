# Decentralised Bidder

This repository contains the implementation of a decentralised bidding system where bidders place and manage bids asynchronously in response to auctions and incoming bid data. The bidding process is managed by the `DecentralisedBidder` class which handles placing, updating, and evaluating bids based on incoming data from auctions.

## Overview

- **DecentralisedBidder**: A class that represents an individual bidder in a decentralised bidding system.
- **AuctionIncomingData**: Struct that contains auction-related information such as task ID, creation time, and expiration time.
- **BidPlacementData**: Struct that represents the details of a bid, including the task ID, bidder ID, bid value, and timestamp.
- **Callback Mechanism**: Uses a callback system for handling bid placement delays and actions asynchronously.
- **UniformRandom**: Helper class that generates random values in a specified range for simulating randomness in bidding behavior.
  
## Features

- **Bid Placement**: Place bids with a random value based on incoming auction data.
- **Re-bidding**: Automatically re-bid if a competitor bids lower than your current bid, respecting bid timestamps to avoid multiple callbacks.
- **Callback System**: Uses callbacks to simulate delays when acting on new auctions and new bids.
- **Bid History Management**: Keeps track of active bids and handles expired auctions.
  
## Requirements

- **C++11 or higher**: The code uses modern C++ features such as `std::shared_ptr`, lambdas, and `std::function`.
- **CMake**: For building the project (if you wish to expand it).

## Code Breakdown

### Key Components

1. **`AuctionIncomingData`**:
   Contains information about an auction, including the task ID, creation time, and expiration time.

   ```cpp
   struct AuctionIncomingData {
       std::string task_id;
       time_point creation_time;
       time_point expire_time;
   };
   ```

2. **`BidPlacementData`**:
   Contains information about a bid, including the task ID, bidder ID, bid value, and timestamp.

   ```cpp
   struct BidPlacementData {
       std::string task_id;
       std::string bidder_id;
       double bid_value;
       time_point timestamp;
   };
   ```

3. **`DecentralisedBidder`**:
   The main class representing a bidder. It includes methods to handle new auctions, new bids, and perform re-bidding if necessary. It also provides a callback system to simulate delays in the bidding process.

   ```cpp
   class DecentralisedBidder {
   public:
       DecentralisedBidder(const std::string &name, callback::PoorManCallbacker::Ptr callbacker = nullptr);
       void on_new_auction(AuctionIncomingData::Ptr auction);
       void on_new_bid(BidPlacementData::Ptr incoming_bid);
       void tick();
       std::string name() const;
       void place_bid_setter(std::function<void(const BidPlacementData *)> functor);
       std::function<void(const BidPlacementData *)> place_bid_getter();
   private:
       void clear_old_history(const std::string &task_id);
       static BidPlacementData *search_better_bid(
           std::unordered_map<std::string, BidPlacementData> &bid_mapping,
           const BidPlacementData *reference_bid
       );
       // Other member variables for storing bid data, callback handlers, etc.
   };
   ```

4. **Callback System**:
   - `callback::Callbacker`: Interface for handling delayed callbacks.
   - `callback::FutureHandler`: Represents a future task that can be cancelled or checked for completion.

### Usage Example

```cpp
#include "decauc.h"

int main() {
    // Initialize bidder
    decauc::DecentralisedBidder bidder("Bidder1");

    // Set the bid placement functor (callback function for placing bids)
    bidder.place_bid_setter([](const decauc::BidPlacementData *bid) {
        std::cout << "Placing bid: " << bid->bid_value << " for task " << bid->task_id << std::endl;
    });

    // Create an auction and simulate bidding process
    auto auction = std::make_shared<decauc::AuctionIncomingData>();
    auction->task_id = "task_123";
    auction->creation_time = std::chrono::system_clock::now();
    auction->expire_time = auction->creation_time + std::chrono::seconds(60);

    bidder.on_new_auction(auction);

    // Simulate a new bid
    auto incoming_bid = std::make_shared<decauc::BidPlacementData>();
    incoming_bid->task_id = "task_123";
    incoming_bid->bidder_id = "Bidder2";
    incoming_bid->bid_value = 50.0;
    incoming_bid->timestamp = std::chrono::system_clock::now();

    bidder.on_new_bid(incoming_bid);

    // Call tick to process any pending callbacks
    bidder.tick();

    return 0;
}
```

### Detailed Explanation

- **`clear_map`**: A helper function that clears bids from a map for a specific task ID.
- **`DecentralisedBidder`**:
  - `on_new_auction`: Handles incoming auctions by either placing a new bid or skipping if an existing bid already satisfies the conditions.
  - `on_new_bid`: Handles incoming bids, re-bids if necessary, and stores bid history.
  - `tick`: Triggers the execution of any pending callbacks.
  - **Callbacks**: Used to introduce delays in bidding actions (`callback_in`).
