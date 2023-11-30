// cppimport
#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>

#include <iostream>
#include <sstream>

#include <chrono>
#include <unordered_map>

#include <random>

namespace py = pybind11;

using time_point = std::chrono::time_point<std::chrono::system_clock>;

struct AuctionIncomingData {
  std::string task_id;
  time_point creation_time;
  time_point expire_time;
};

struct BidPlacementData {
  std::string task_id;
  std::string bidder_id;
  double bid_value;
  time_point timestamp;
};

static std::string
timePointAsString(const std::chrono::system_clock::time_point &tp) {
  std::time_t t = std::chrono::system_clock::to_time_t(tp);
  std::string ts = std::ctime(&t);
  ts.resize(ts.size() - 1);
  return ts;
}

double rand0to1() { return ((double)rand()) / RAND_MAX; }

class DecentralisedBidder {
public:
  DecentralisedBidder(const std::string &name) : name_(name) {}

  const BidPlacementData *on_new_auction(const AuctionIncomingData &auction) {
    // if (auto existing_auction = example.find(auction.task_id);
    // existing_auction != example.end()) {
    // }
    //     // old colliding auction task_id??
    //     std::cout << "Found " << search->first << ' ' << search->second <<
    //     '\n';
    // else
    //     std::cout << "Not found\n";

    // TODO random sleep

    current_bid_[auction.task_id] =
        BidPlacementData{auction.task_id, name(), rand0to1() * 10,
                         std::chrono::system_clock::now()};

    return &current_bid_[auction.task_id];
  }

  const BidPlacementData *on_new_bid(const BidPlacementData &incoming_bid) {
    if (incoming_bid.bidder_id == name())
      return nullptr;

    auto bid_it = current_bid_.find(incoming_bid.task_id);
    if (bid_it == current_bid_.end()) {
      // we have no interest in this auction.
      return nullptr;
    }
    BidPlacementData *my_previous_bid = &(bid_it->second);

    if (my_previous_bid->bid_value <= incoming_bid.bid_value) {
      // my previous bid was not better than this new bid.
      return nullptr;
    }

    // the bid is lower than our previous bid?
    // TODO: recompute bids value if the bid time is long (where we might have
    // finished a different task by now?)

    // re-bid with the same amount.
    my_previous_bid->timestamp = std::chrono::system_clock::now();

    return my_previous_bid;
  }

  std::string name() const { return name_; }

private:
  std::string name_;
  std::unordered_map<std::string, BidPlacementData> current_bid_;
};

int square(int x) { return x * x; }

PYBIND11_MODULE(decentralised_bidding, m) {
  m.def("square", &square);
  // struct AuctionIncomingData {
  //     std::string task_id;
  //     time_point creation_time;
  //     time_point expire_time;
  // };

  // struct BidPlacementData {
  //     std::string task_id;
  //     std::string bidder_id;
  //     double bid_value;
  //     time_point timestamp;
  // };
  py::class_<AuctionIncomingData>(m, "AuctionIncomingData")
      .def(py::init<const std::string &, time_point, time_point>())
      .def_readwrite("task_id", &AuctionIncomingData::task_id)
      .def_readwrite("creation_time", &AuctionIncomingData::creation_time)
      .def_readwrite("expire_time", &AuctionIncomingData::expire_time)
      .def("__repr__", [](const AuctionIncomingData &auction) {
        // generate your string here;
        std::stringstream ss;

        ss << "<AuctionIncomingData task_id:" << auction.task_id << " created@"
           << timePointAsString(auction.creation_time) << " expire@ "
           << timePointAsString(auction.expire_time) << ">";
        return ss.str();
      });

  py::class_<BidPlacementData>(m, "BidPlacementData")
      .def(py::init<const std::string &, const std::string &, double,
                    time_point>())
      .def_readwrite("task_id", &BidPlacementData::task_id)
      .def_readwrite("bidder_id", &BidPlacementData::bidder_id)
      .def_readwrite("bid_value", &BidPlacementData::bid_value)
      .def_readwrite("timestamp", &BidPlacementData::timestamp)
      .def("__repr__", [](const BidPlacementData &bid) {
        // generate your string here;
        std::stringstream ss;

        ss << "<BidPlacementData task_id:" << bid.task_id
           << " bidder_id:" << bid.bidder_id << " bid_val: " << bid.bid_value
           << " @ " << timePointAsString(bid.timestamp) << ">";
        return ss.str();
      });

  py::class_<DecentralisedBidder>(m, "DecentralisedBidder")
      .def(py::init<const std::string &>())
      .def("on_new_auction", &DecentralisedBidder::on_new_auction,
           py::return_value_policy::reference_internal)
      .def("on_new_bid", &DecentralisedBidder::on_new_bid,
           py::return_value_policy::reference_internal)
      .def("__repr__", [](const DecentralisedBidder &bidder) {
        // generate your string here;
        std::stringstream ss;

        ss << "<DecentralisedBidder name:" << bidder.name() << ">";
        return ss.str();
      });
}
/*
<%
setup_pybind11(cfg)
%>
*/