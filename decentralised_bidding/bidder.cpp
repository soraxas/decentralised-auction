// cppimport
#include "data_types.h"
#include "poor_man_callbacker.h"

#include <iostream>
#include <sstream>
#include <unordered_map>

namespace decauc
{
    inline void clear_map(
        std::unordered_map<std::string, BidPlacementData> &constainer, const std::string &task_id
    )
    {
        for (auto itr = constainer.begin(); itr != constainer.end(); /*nothing*/)
        {
            if (itr->second.task_id == task_id)
            {
                itr = constainer.erase(itr);
            }
            else
            {
                ++itr;
            }
        }
    }

    class DecentralisedBidder
    {
    public:
        DecentralisedBidder(
            const std::string &name, callback::PoorManCallbacker::Ptr callbacker = nullptr
        )
          : name_(name)
        {
            place_bid_setter([](const BidPlacementData *)
                             { std::cerr << "ERROR: No place_bid_functor set yet!" << std::endl; });

            if (!callbacker)
            {
                // creating a default callbacker
                callbacker = std::make_shared<callback::PoorManCallbacker>();
            }

            callbacker_ = std::move(std::make_shared<callback::PoorManCallbacker>());
        }

        double compute_bid_value(AuctionIncomingData::Ptr auction)
        {
            return rand_() * 10;
        }

        void on_new_auction(AuctionIncomingData::Ptr auction)
        {
            auto existing_auction = active_auction_bid_.find(auction->task_id);
            if (existing_auction != active_auction_bid_.end())
            {
                // TODO: clean old expired auction.
                // old colliding auction task_id??
                active_auction_bid_.erase(existing_auction);
                clear_old_history(auction->task_id);
            }

            auto future_handler = callbacker_->callback_in(
                rand_() * config_.rand_delay_acting_on_new_auction,
                [this, auction]()
                {
                    double my_bid_val = compute_bid_value(auction);

                    // see if there is a better existing bid.
                    auto it = active_auction_bid_.find(auction->task_id);
                    if (it != active_auction_bid_.end() && it->second.bid_value >= my_bid_val)
                    {
                        // no need to do anything
                        return true;
                    }

                    // check if this might collide with my previous bid.

                    my_current_bid_[auction->task_id] =
                        BidPlacementData{auction->task_id, name(), my_bid_val, my_clock_t::now()};

                    // return &my_current_bid_[auction->task_id];

                    place_bid_functor_(&my_current_bid_[auction->task_id]);
                    return true;
                }
            );
        }

        void on_new_bid(BidPlacementData::Ptr incoming_bid)
        {
            // store history
            {
                const BidPlacementData *prev_better_bid =
                    search_better_bid(active_auction_bid_, incoming_bid.get());

                if (!prev_better_bid)
                {
                    // store new history
                    active_auction_bid_[incoming_bid->task_id] = *incoming_bid;
                }
            }

            if (incoming_bid->bidder_id == name())
            {
                // do not act on one self's bid.
                return;
            }

            /*
            auto bid_it = my_current_bid_.find(incoming_bid->task_id);
            if (bid_it == my_current_bid_.end())
            {
                // we have no interest in this auction.
                return nullptr;
            }
            BidPlacementData *my_previous_bid = &(bid_it->second);

                // my current bid was not better than this new incoming bid.
                return nullptr;
            }
            */

            BidPlacementData *my_previous_bid =
                search_better_bid(my_current_bid_, incoming_bid.get());

            if (!my_previous_bid)
            {
                // I'm not interested in this bid.
                // TODO: revisit this
                return;
            }

            // the bid is lower than our previous bid?
            // TODO: recompute bids value if the bid time is long (where we might have
            // finished a different task by now?)

            // re-bid with the same amount.
            // the previous one should updates our pointed value.

            // std::cerr << "re-bidding : by" << name() << " in response to "
            //           << incoming_bid->bidder_id << "... " << my_previous_bid->bid_value << " > "
            //           << incoming_bid->bid_value << std::endl;

            auto future_handler = callbacker_->callback_in(
                rand_() * config_.rand_delay_acting_on_new_bid,
                [this, my_previous_bid, incoming_bid]()
                {
                    auto my_existing_bid = search_better_bid(my_current_bid_, my_previous_bid);
                    if (my_existing_bid != nullptr &&
                        my_existing_bid->timestamp > incoming_bid->timestamp)
                    {
                        // This check is to ensure that OTHER callback had not already perform this
                        // re-bidding. This avoid multiple duplicates callback from firing.
                        return true;
                    }

                    // TODO: recompute bid value?
                    // my_previous_bid->bid_value = compute_bid_value(...);
                    my_previous_bid->timestamp = my_clock_t::now();

                    place_bid_functor_(my_previous_bid);
                    // my_current_bid_[my_new_bid->task_id] = *my_new_bid;

                    return true;
                }
            );

            // bid_placement_handlers_.emplace_back(future_handler, my_previous_bid);
        }

        void tick()
        {
            callbacker_->tick();
        }

        std::string name() const
        {
            return name_;
        }

        void place_bid_setter(std::function<void(const BidPlacementData *)> functor)
        {
            place_bid_functor_ = functor;
        }

        std::function<void(const BidPlacementData *)> place_bid_getter()
        {
            return place_bid_functor_;
        }

    protected:
        inline static BidPlacementData *search_better_bid(
            std::unordered_map<std::string, BidPlacementData> &bid_mapping,
            const BidPlacementData *reference_bid
        )
        {
            // 1. no existing history
            // 2. previously stored history had lower bid
            // However, if previous bid had equal to the newer bid, compare the timestamp
            // where the bid with earlier timestamp wins.
            // PLS do not fake timestamp. [Which might be a problem in decentralised system...]

            auto prev_bid_it = bid_mapping.find(reference_bid->task_id);
            if (prev_bid_it == bid_mapping.end())
                return nullptr;

            const BidPlacementData &prev_bid = prev_bid_it->second;
            if (prev_bid.bid_value < reference_bid->bid_value)
                // worse bidding value
                return nullptr;
            if (prev_bid.bid_value == reference_bid->bid_value &&
                prev_bid.timestamp > reference_bid->timestamp)
                // they of same value, but the new bid is newer
                return nullptr;

            return &prev_bid_it->second;
        }

        void clear_old_history(const std::string &task_id)
        {
            clear_map(my_current_bid_, task_id);
            clear_map(active_auction_bid_, task_id);
        }

        // std::list<std::pair<callback::FutureHandler::Ptr, const BidPlacementData *>>
        //     bid_placement_handlers_;
        std::function<void(const BidPlacementData *)> place_bid_functor_;

        std::string name_;
        std::unordered_map<std::string, BidPlacementData> my_current_bid_;
        std::unordered_map<std::string, BidPlacementData> active_auction_bid_;
        UniformRandom rand_;

        callback::Callbacker::Ptr callbacker_;
        BidderConfig config_;
    };

};  // namespace decauc
