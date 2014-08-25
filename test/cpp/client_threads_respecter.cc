#include "src/client_threads_respecter.h"

#include <thread>

#include <gtest/gtest.h>
#include <glog/logging.h>

using CTR = ::TailProduce::ClientThreadsRespecter;

TEST(ClientThreadsRespecter, Smoke) {
    size_t x = 0;
    bool xb = false;

    size_t y = 0;
    bool yb = false;

    {
        CTR ctr;

        std::thread([&ctr, &x, &xb]() {
                        {
                            auto client = ctr.RegisterScopedClient();
                            while (client) {
                                VLOG(2) << "++x";
                                ++x;
                                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                            }
                            VLOG(2) << "Terminating `++x` in 0.1s.";
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            xb = true;
                            VLOG(2) << "Terminated `++x`.";
                        }
                        VLOG(2) << "The `++x` thread is done, ensuring that spawning another client would not work.";
                        ASSERT_THROW(auto client = ctr.RegisterScopedClient(),
                                     ::TailProduce::AlreadyInTearDownModeException);
                        VLOG(2) << "The `++x` thread is done, confirmed that spawning another client does not work.";
                    }).detach();

        std::thread([&ctr, &y, &yb]() {
                        std::function<void(const CTR::Client&)> lambda;
                        VLOG(2) << "Running the `++y` thread.";
                        EXPECT_TRUE(ctr.RunClientCode(lambda = [&y, &yb](const CTR::Client& client) {
                            while (client) {
                                VLOG(2) << "++y";
                                ++y;
                                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                            }
                            VLOG(2) << "Terminating `++y` in 0.1s.";
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            yb = true;
                            VLOG(2) << "Terminated `++y`.";
                        }));
                        VLOG(2) << "The `++y` thread is done, ensuring that spawning another client would not work.";
                        EXPECT_FALSE(
                            ctr.RunClientCode(lambda = [](const CTR::Client& client) { ASSERT_TRUE(false); }));
                        VLOG(2) << "The `++y` thread is done, confirmed that spawning another client does not work.";
                    }).detach();

        VLOG(2) << "Let `++x` and `++y` threads run for 0.25s.";
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        VLOG(2) << "Time for `++x` and `++y` threads to terminate.";
    }
    VLOG(2) << "Both threads terminated successfully, x = " << x << ", y = " << y << ".";

    // Both threads should have terminated successfully.
    EXPECT_TRUE(xb);
    EXPECT_TRUE(yb);

    // Both threads should have had enough time to increment their counters at least by a bit.
    // Technically, the EXPECT-s below make the test flaky, but the range is generous enough.
    EXPECT_GT(x, 10);
    EXPECT_LT(x, 100);
    EXPECT_GT(y, 10);
    EXPECT_LT(y, 100);
}
