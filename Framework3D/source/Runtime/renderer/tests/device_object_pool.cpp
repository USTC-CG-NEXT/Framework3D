#include <gtest/gtest.h>

#include "../source/internal/memory/DeviceMemoryPool.hpp"
// SceneTypes
#include <random>

#include "../nodes/shaders/shaders/Scene/SceneTypes.slang"
#include "Logger/Logger.h"

using namespace USTC_CG;

class MemoryPoolTest : public ::testing::Test {
   protected:
    void SetUp() override
    {
        // Code here will be called immediately after the constructor (right
        // before each test).
        RHI::init();
        log::SetMinSeverity(Severity::Warning);
        log::EnableOutputToConsole(true);
    }

    void TearDown() override
    {
        // Code here will be called immediately after each test (right before
        // the destructor).
        pool.destroy();
        pool2.destroy();
        RHI::shutdown();
    }

    DeviceMemoryPool<int> pool;
    DeviceMemoryPool<float> pool2;
};

TEST_F(MemoryPoolTest, allocate)
{
    auto handle = pool.allocate(10);
    EXPECT_TRUE(handle != nullptr);
    EXPECT_EQ(pool.count(), 10);
    EXPECT_EQ(pool.pool_size(), 16);
    EXPECT_EQ(pool.max_memory_offset(), 10 * sizeof(int));

    auto handle2 = pool.allocate(30);
    EXPECT_TRUE(handle2 != nullptr);

    EXPECT_EQ(pool.count(), 40);
    EXPECT_EQ(pool.pool_size(), 64);
    EXPECT_EQ(pool.max_memory_offset(), 40 * sizeof(int));
    handle2 = nullptr;

    EXPECT_EQ(pool.count(), 10);
    EXPECT_EQ(pool.pool_size(), 64);
    EXPECT_EQ(pool.max_memory_offset(), 40 * sizeof(int));

    auto handle3 = pool.allocate(20);
    EXPECT_TRUE(handle3 != nullptr);
    EXPECT_EQ(handle3->offset, 10 * sizeof(int));
    EXPECT_EQ(pool.count(), 30);
    EXPECT_EQ(pool.pool_size(), 64);
    EXPECT_EQ(pool.max_memory_offset(), 40 * sizeof(int));

    auto handle4 = pool.allocate(35);
    EXPECT_EQ(handle4->offset, 40 * sizeof(int));
    EXPECT_TRUE(handle4 != nullptr);
    EXPECT_EQ(pool.count(), 65);
    EXPECT_EQ(pool.pool_size(), 128);
    EXPECT_EQ(pool.max_memory_offset(), 75 * sizeof(int));

    std::vector data(35, 42);
    handle4->write_data(data.data());
}

TEST_F(MemoryPoolTest, fragmetation_cleansing)
{
    auto rng_engine = std::default_random_engine();

    std::vector<DeviceMemoryPool<int>::MemoryHandle> handles;
    for (int i = 0; i < 1000; ++i) {
        auto rng = std::uniform_int_distribution(1, 1000);
        auto float_rng = std::uniform_real_distribution(0.0f, 1.0f);

        int random_Val_int = rng(rng_engine);
        auto handle = pool.allocate(random_Val_int);
        if (float_rng(rng_engine) < 0.5) {
            handles.push_back(handle);
        }
        else {
            handle = nullptr;
        }
    }

    ASSERT_FALSE(pool.sanitize());
    pool.compress();
    ASSERT_TRUE(pool.sanitize());
}

TEST_F(MemoryPoolTest, data_io)
{
    auto handle = pool.allocate(10);
    std::vector<int> data(10, 42);
    handle->write_data(data.data());

    std::vector<int> read_data(10);
    handle->read_data(read_data.data());
    EXPECT_EQ(data, read_data);
}

TEST_F(MemoryPoolTest, multi_threaded_allocation)
{
    auto rng_engine = std::default_random_engine();
    auto float_rng = std::uniform_real_distribution(0.0f, 1.0f);
    std::vector<DeviceMemoryPool<int>::MemoryHandle> int_handles;
    std::vector<DeviceMemoryPool<float>::MemoryHandle> float_handles;

    std::vector<std::thread> threads;
    for (int i = 0; i < 500; ++i) {
        threads.push_back(std::thread(
            [this, &rng_engine, &float_rng, &int_handles, &float_handles]() {
                if (float_rng(rng_engine) < 0.5) {
                    auto handle = pool.allocate(10);
                    std::vector<int> data(10, 42);
                    handle->write_data(data.data());

                    if (float_rng(rng_engine) < 0.5) {
                        int_handles.push_back(handle);
                    }
                    else {
                        handle = nullptr;
                    }
                }
                else {
                    auto handle = pool2.allocate(10);
                    std::vector<float> data(10, 42.0f);
                    handle->write_data(data.data());

                    if (float_rng(rng_engine) < 0.5) {
                        float_handles.push_back(handle);
                    }
                    else {
                        handle = nullptr;
                    }
                }
            }));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "pool1: " << pool.info() << std::endl;
    std::cout << "pool2: " << pool2.info() << std::endl;

    pool.compress();
    pool2.compress();

    std::cout << "pool1: " << pool.info() << std::endl;
    std::cout << "pool2: " << pool2.info() << std::endl;

    ASSERT_TRUE(pool.sanitize());
    ASSERT_TRUE(pool2.sanitize());
}