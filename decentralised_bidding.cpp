// cppimport
#include <pybind11/pybind11.h>
#include <sstream>

namespace py = pybind11;

class DecentralisedBidder {
public:
    DecentralisedBidder(const std::string &name):
    name_(name)
     {

    }

    void on_new_auction() {

    }

    std::string name() const {
        return name_;
    }

private:
    std::string name_;
};

int square(int x) {
    return x * x;
}

PYBIND11_MODULE(decentralised_bidding, m) {
    m.def("square", &square);

    py::class_<DecentralisedBidder>(m, "DecentralisedBidder")
    .def(py::init<const std::string&>())
    .def("on_new_auction", &DecentralisedBidder::on_new_auction)
    .def("__repr__", [](const DecentralisedBidder& bidder) {
        // generate your string here;
        std::stringstream ss;

        ss << "<DecentralisedBidder name:" << bidder.name() << ">";
        return ss.str();
    });
    ;
}
/*
<%
setup_pybind11(cfg)
%>
*/