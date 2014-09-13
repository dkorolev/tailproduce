#include "src/waitable_atomic.h"

#include <thread>

#include <gtest/gtest.h>
#include <glog/logging.h>

using ::TailProduce::IntrusiveClient;
using ::TailProduce::WaitableIntrusiveObject;
template <typename T> using WaitableAtomic = ::TailProduce::WaitableAtomic<T>;

TEST(WaitableAtomic, Smoke) {
    struct Object : WaitableIntrusiveObject {
        size_t x = 0;
        bool xb = false;
        size_t y = 0;
        bool yb = false;
    };

    WaitableAtomic<Object> object;
    {
        // This scope runs asynchronous operations in two threads other than the main thread.
        WaitableAtomic<WaitableIntrusiveObject> top_level_lock;

        std::thread([&top_level_lock, &object](IntrusiveClient top_level_client) {
                        VLOG(2) << "Running the `++x` thread.";
                        ASSERT_TRUE(bool(top_level_lock.RegisterScopedClient()));
                        while (top_level_client) {
                            VLOG(2) << "++x";
                            ++object.MutableScopedAccessor()->x;
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                        VLOG(2) << "Terminating `++x` in 0.01s.";
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        object.MutableScopedAccessor()->xb = true;
                        VLOG(2) << "The `++x` thread is done, ensuring that spawning another client would not work.";
                        ASSERT_FALSE(bool(top_level_lock.RegisterScopedClient()));
                        VLOG(2) << "The `++x` thread is done, confirmed that spawning another client does not work.";
                    },
                    top_level_lock.RegisterScopedClient()).detach();

        std::thread([&top_level_lock, &object](IntrusiveClient top_level_client) {
                        VLOG(2) << "Running the `++y` thread.";
                        ASSERT_TRUE(bool(top_level_lock.RegisterScopedClient()));
                        while (top_level_client) {
                            VLOG(2) << "++y";
                            object.MutableUse([&top_level_client](Object& o) { ++o.y; });
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                        VLOG(2) << "Terminating `++y` in 0.01s.";
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        object.MutableUse([&top_level_client](Object& o) { o.yb = true; });
                        VLOG(2) << "The `++y` thread is done, ensuring that spawning another client would not work.";
                        ASSERT_FALSE(bool(top_level_lock.RegisterScopedClient()));
                        VLOG(2) << "The `++y` thread is done, confirmed that spawning another client does not work.";
                    },
                    top_level_lock.RegisterScopedClient()).detach();

        VLOG(2) << "Let `++x` and `++y` threads run for 0.025s.";
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        VLOG(2) << "Time for `++x` and `++y` threads to terminate.";

        // This block will only finish when both client threads have terminated.
    }
    VLOG(2) << "Both threads are done.";

    Object copy_of_object(*object.ImmutableScopedAccessor());

    VLOG(2) << "x = " << copy_of_object.x << ", y = " << copy_of_object.y << ".";

    // Both threads should have terminated successfully.
    EXPECT_TRUE(copy_of_object.xb);
    EXPECT_TRUE(copy_of_object.yb);

    // Both threads should have had enough time to increment their counters at least by a bit.
    // Technically, the EXPECT-s below make the test flaky, but the range is generous enough.
    EXPECT_GT(copy_of_object.x, 10);
    EXPECT_LT(copy_of_object.x, 100);
    EXPECT_GT(copy_of_object.y, 10);
    EXPECT_LT(copy_of_object.y, 100);
}
