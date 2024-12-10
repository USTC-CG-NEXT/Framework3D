#include <gtest/gtest.h>

#include "../source/internal/memory/DeviceObjectPool.hpp"
// SceneTypes
#include <random>

#include "../nodes/shaders/shaders/Scene/SceneTypes.slang"

using namespace USTC_CG;

class MemoryPoolTest : public ::testing::Test {
   protected:
    void SetUp() override
    {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override
    {
        // Code here will be called immediately after each test (right before
        // the destructor).
        pool.destroy();
        RHI::shutdown();
    }

    DeviceMemoryPool<int> pool;
};

TEST_F(MemoryPoolTest, allocate)
{
    auto handle = pool.allocate(10);
    EXPECT_TRUE(handle != nullptr);
    EXPECT_EQ(pool.size(), 10);
    EXPECT_EQ(pool.pool_size(), 16);
    EXPECT_EQ(pool.max_memory_offset(), 10 * sizeof(int));

    auto handle2 = pool.allocate(30);
    EXPECT_TRUE(handle2 != nullptr);

    EXPECT_EQ(pool.size(), 40);
    EXPECT_EQ(pool.pool_size(), 64);
    EXPECT_EQ(pool.max_memory_offset(), 40 * sizeof(int));
    handle2 = nullptr;

    EXPECT_EQ(pool.size(), 10);
    EXPECT_EQ(pool.pool_size(), 64);
    EXPECT_EQ(pool.max_memory_offset(), 40 * sizeof(int));

    auto handle3 = pool.allocate(20);
    EXPECT_TRUE(handle3 != nullptr);
    EXPECT_EQ(handle3->offset, 10 * sizeof(int));
    EXPECT_EQ(pool.size(), 30);
    EXPECT_EQ(pool.pool_size(), 64);
    EXPECT_EQ(pool.max_memory_offset(), 40 * sizeof(int));

    auto handle4 = pool.allocate(35);
    EXPECT_EQ(handle4->offset, 40 * sizeof(int));
    EXPECT_TRUE(handle4 != nullptr);
    EXPECT_EQ(pool.size(), 65);
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
    pool.gc();
    ASSERT_TRUE(pool.sanitize());
}