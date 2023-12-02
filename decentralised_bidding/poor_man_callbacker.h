#pragma once

#include "data_types.h"

#include <chrono>
#include <functional>
#include <list>
#include <thread>

namespace decauc
{

    namespace callback
    {

        class PoorMandFutureHandler : public FutureHandler
        {
        public:
            PoorMandFutureHandler() : cancelled_(false), done_(false)
            {
            }

            bool cancelled() override
            {
                return cancelled_;
            }

            void cancel() override
            {
                cancelled_ = true;
            }

            void set_done()
            {
                done_ = true;
            }

            bool done()
            {
                return done_;
            }

        private:
            bool cancelled_;
            bool done_;
        };

        class PoorManCallbacker : public Callbacker
        {
        public:
            FutureHandler::Ptr callback_in(double delay, Callbacker::callback_t functor) override
            {
                auto handler = std::make_shared<PoorMandFutureHandler>();
                // if (delay < .1)
                // {
                //     // let's just do it right now!
                //     std::this_thread::sleep_for(std::chrono::duration<double>(delay));
                //     functor();
                //     handler->set_done();
                // }
                // else
                // {
                time_point wakeup_at =
                    my_clock_t::now() + std::chrono::duration_cast<my_clock_t::duration>(
                                            std::chrono::duration<double>(delay)
                                        );

                callbacks_.emplace_back(wakeup_at, handler, functor);
                // }
                return handler;
            }

            void tick() override
            {
                for (auto itr = callbacks_.begin(); itr != callbacks_.end(); /*nothing*/)
                {
                    if (std::get<0>(*itr) > my_clock_t::now())
                    {
                        // future not arrived yet
                        ++itr;
                    }
                    else
                    {
                        // it's time to proess this poorman callback.
                        auto future_handler = std::get<1>(*itr);
                        if (future_handler->cancelled())
                        {
                            itr = callbacks_.erase(itr);
                        }
                        else
                        {
                            auto cb = std::get<2>(*itr);
                            if (cb())
                            {
                                // set future as fulfilled
                                future_handler->set_done();
                                itr = callbacks_.erase(itr);
                            }
                        }
                    }
                }
            }

            std::list<std::tuple<time_point, FutureHandler::Ptr, Callbacker::callback_t>>
                callbacks_;
        };

    };  // namespace callback
};      // namespace decauc