// cppimport
#include "data_types.h"
#include "poor_man_callbacker.h"

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include <iostream>
#include <sstream>
#include <unordered_map>

namespace py = pybind11;

static std::string timePointAsString(const my_clock_t::time_point &tp)
{
    std::time_t t = my_clock_t::to_time_t(tp);
    std::string ts = std::ctime(&t);
    ts.resize(ts.size() - 1);
    return ts;
}

class DecentralisedBidder
{
public:
    DecentralisedBidder(const std::string &name) : name_(name)
    {
        place_bid_functor_ = [](const BidPlacementData *)
        { std::cerr << "ERROR: No place_bid_functor set yet!" << std::endl; };
    }

    std::function<void(const BidPlacementData *)> place_bid_functor_;

    void set_place_bid_functor(std::function<void(const BidPlacementData *)> functor)
    {
        place_bid_functor_ = functor;
    }

    const BidPlacementData *on_new_auction(AuctionIncomingDataPtr auction)
    {
        auto existing_auction = active_auction_bid_.find(auction->task_id);
        if (existing_auction != active_auction_bid_.end())
        {
            // // old colliding auction task_id??
            active_auction_bid_.erase(existing_auction);
        }

        // TODO random sleep

        callbacker.callback_in(
            rand_() * 0.2,
            [this, auction]()
            {
                double my_bid_val = rand_() * 10;

                // see if there is a better existing bid.
                auto it = active_auction_bid_.find(auction->task_id);
                if (it != active_auction_bid_.end() && it->second.bid_value >= my_bid_val)
                {
                    return;
                }

                my_current_bid_[auction->task_id] =
                    BidPlacementData{auction->task_id, name(), my_bid_val, my_clock_t::now()};

                // return &my_current_bid_[auction->task_id];

                place_bid_functor_(&my_current_bid_[auction->task_id]);
            }
        );

        return nullptr;
    }

    const BidPlacementData *on_new_bid(BidPlacementDataPtr incoming_bid)
    {
        // store history
        {
            auto it = active_auction_bid_.find(incoming_bid->task_id);
            if (
              // no existing history
              it == active_auction_bid_.end()
              // previously stored history had lower bid
            || it->second.bid_value < incoming_bid->bid_value
            )
            {
                // store new history
                active_auction_bid_[incoming_bid->task_id] = *incoming_bid;
            }
            // active_auction_bid_[auction->task_id] =
            //     BidPlacementData{auction->task_id, name(), rand_() * 10, my_clock_t::now()};
        }

        if (incoming_bid->bidder_id == name())
            return nullptr;

        auto bid_it = my_current_bid_.find(incoming_bid->task_id);
        if (bid_it == my_current_bid_.end())
        {
            // we have no interest in this auction.
            return nullptr;
        }
        BidPlacementData *my_previous_bid = &(bid_it->second);

        if (my_previous_bid->bid_value <= incoming_bid->bid_value)
        {
            // my previous bid was not better than this new bid.
            return nullptr;
        }

        // the bid is lower than our previous bid?
        // TODO: recompute bids value if the bid time is long (where we might have
        // finished a different task by now?)

        // re-bid with the same amount.
        my_previous_bid->timestamp = my_clock_t::now();

        place_bid_functor_(my_previous_bid);

        return nullptr;
    }

    void poor_man_callback_via_polling()
    {
        callbacker.tick();
    }

    std::string name() const
    {
        return name_;
    }

protected:
    std::string name_;
    std::unordered_map<std::string, BidPlacementData> my_current_bid_;
    std::unordered_map<std::string, BidPlacementData> active_auction_bid_;
    UniformRandom rand_;

    PoorManCallbacker callbacker;
};

int square(int x)
{
    return x * x;
}

PYBIND11_MODULE(decentralised_bidding, m)
{
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
    py::class_<AuctionIncomingData, AuctionIncomingDataPtr>(m, "AuctionIncomingData")
        .def(py::init<const std::string &, time_point, time_point>())
        .def_readwrite("task_id", &AuctionIncomingData::task_id)
        .def_readwrite("creation_time", &AuctionIncomingData::creation_time)
        .def_readwrite("expire_time", &AuctionIncomingData::expire_time)
        .def(
            "__repr__",
            [](const AuctionIncomingData &auction)
            {
                // generate your string here;
                std::stringstream ss;

                ss << "<AuctionIncomingData task_id:" << auction.task_id << " created@"
                   << timePointAsString(auction.creation_time) << " expire@ "
                   << timePointAsString(auction.expire_time) << ">";
                return ss.str();
            }
        );

    py::class_<BidPlacementData, BidPlacementDataPtr>(m, "BidPlacementData")
        .def(py::init<const std::string &, const std::string &, double, time_point>())
        .def_readwrite("task_id", &BidPlacementData::task_id)
        .def_readwrite("bidder_id", &BidPlacementData::bidder_id)
        .def_readwrite("bid_value", &BidPlacementData::bid_value)
        .def_readwrite("timestamp", &BidPlacementData::timestamp)
        .def(
            "__repr__",
            [](const BidPlacementData &bid)
            {
                // generate your string here;
                std::stringstream ss;

                ss << "<BidPlacementData " << bid.task_id  //
                   << " bidder:" << bid.bidder_id          //
                   << " val: " << bid.bid_value <<         //
                    " @ " << timePointAsString(bid.timestamp) << ">";
                return ss.str();
            }
        );

    py::class_<DecentralisedBidder>(m, "DecentralisedBidder")
        .def(py::init<const std::string &>())
        .def(
            "on_new_auction", &DecentralisedBidder::on_new_auction,
            py::return_value_policy::reference_internal
        )
        .def(
            "on_new_bid", &DecentralisedBidder::on_new_bid,
            py::return_value_policy::reference_internal
        )
        .def("poor_man_callback_via_polling", &DecentralisedBidder::poor_man_callback_via_polling)
        .def("set_place_bid_functor", &DecentralisedBidder::set_place_bid_functor)
        .def(
            "__repr__",
            [](const DecentralisedBidder &bidder)
            {
                // generate your string here;
                std::stringstream ss;

                ss << "<DecentralisedBidder name:" << bidder.name() << ">";
                return ss.str();
            }
        );
}

/*
<%
setup_pybind11(cfg)
%>
*/