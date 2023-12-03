// cppimport
#include "bidder.cpp"

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

namespace decauc
{

    namespace py = pybind11;

    static std::string timePointAsString(const my_clock_t::time_point &tp)
    {
        std::time_t t = my_clock_t::to_time_t(tp);
        std::string ts = std::ctime(&t);
        ts.resize(ts.size() - 1);
        return ts;
    }

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
        py::class_<AuctionIncomingData, AuctionIncomingData::Ptr>(m, "AuctionIncomingData")
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

        py::class_<BidPlacementData, BidPlacementData::Ptr>(m, "BidPlacementData")
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
            .def(py::init<const std::string &>(), py::arg("name"))
            .def(
                "on_new_auction", &DecentralisedBidder::on_new_auction,
                py::return_value_policy::reference_internal
            )
            .def(
                "on_new_bid", &DecentralisedBidder::on_new_bid,
                py::return_value_policy::reference_internal
            )
            .def("tick", &DecentralisedBidder::tick)
            .def_property(
                "place_bid_functor", &DecentralisedBidder::place_bid_getter,
                &DecentralisedBidder::place_bid_setter
            )
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

};  // namespace decauc

/*
<%
setup_pybind11(cfg)
%>
*/


