#pragma once

#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <random>

namespace decauc
{

    using my_clock_t = std::chrono::system_clock;
    using time_point = my_clock_t::time_point;

    class UniformRandom
    {
    public:
        UniformRandom(double lower = 0., double upper = 1.)
          : generator_(std::random_device{}()), dist_(lower, upper)
        {
        }

        double operator()()
        {
            return dist_(generator_);
        }

    private:
        std::random_device rd;
        std::mt19937 generator_;
        std::uniform_real_distribution<double> dist_;
    };

    struct AuctionIncomingData
    {
        using Ptr = std::shared_ptr<AuctionIncomingData>;

        std::string task_id;
        time_point creation_time;
        time_point expire_time;
    };

    struct BidPlacementData
    {
        using Ptr = std::shared_ptr<BidPlacementData>;

        std::string task_id;
        std::string bidder_id;
        double bid_value;
        time_point timestamp;
    };


    struct TaskStatusUpdate
    {
        enum Status {
            declare_takeover,
            heart_beat,
            finished,
            failed
        };

        using Ptr = std::shared_ptr<TaskStatusUpdate>;

        std::string task_id;
        time_point timestamp;
        Status stastus;
    };

    struct BidderConfig
    {
        double rand_delay_acting_on_new_auction = 0.2;
        double rand_delay_acting_on_new_bid = 0.05;

        double heart_beat_every = 3.;
        double heart_beat_grace_period = 2.;
    };

    namespace callback
    {

        class FutureHandler
        {
        public:
            using Ptr = std::shared_ptr<FutureHandler>;

            virtual void cancel() = 0;
            virtual bool cancelled() = 0;
            virtual void set_done() = 0;
            virtual bool done() = 0;
        };

        class Callbacker
        {
        public:
            using Ptr = std::shared_ptr<callback::Callbacker>;
            using callback_t = std::function<bool()>;

            virtual FutureHandler::Ptr callback_in(double delay, callback_t functor) = 0;

            virtual void tick()
            {
                /* Do nothing */

                // throw std::runtime_error("[Callbacker] Does not supports tick funtion");
            }
        };
    };  // namespace callback

};  // namespace decauc