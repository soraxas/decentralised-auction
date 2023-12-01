#pragma once

#include <chrono>
#include <functional>
#include <list>
#include <thread>

using poor_man_callback_t = std::pair<time_point, std::function<void()>>;

class PoorManCallbacker
{
public:
    void callback_in(double delay, std::function<void()> functor)
    {
        if (delay < .1)
        {
            // let's just do it right now!
            std::this_thread::sleep_for(std::chrono::duration<double>(delay));
            functor();
        }
        else
        {
            time_point wakeup_at =
                my_clock_t::now() + std::chrono::duration_cast<my_clock_t::duration>(
                                        std::chrono::duration<double>(delay)
                                    );

            callbacks_.emplace_back(wakeup_at, functor);
        }
    }

    void tick()
    {
        for (auto itr = callbacks_.begin(); itr != callbacks_.end(); /*nothing*/)
        {
            if (itr->first > my_clock_t::now())
            {
                ++itr;
            }
            else
            {
                // it's time to proess this poorman callback.
                auto cb = itr->second;
                cb();
                itr = callbacks_.erase(itr);
            }
        }
    }

    std::list<poor_man_callback_t> callbacks_;
};