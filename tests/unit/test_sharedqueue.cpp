/**
 * @file test_sharedqueue.cpp
 * @brief Unit tests for SharedQueue IPC component
 *
 * Tests thread-safe queue operations, producer-consumer pattern,
 * and concurrent access for the IPC system.
 */

#include "../../infrastructure/include/ipc/sharedqueue.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <cassert>

using namespace std;

// Test counters
int tests_passed = 0;
int tests_failed = 0;

#define TEST(name) \
    void test_##name(); \
    void run_test_##name() { \
        try { \
            test_##name(); \
            tests_passed++; \
            cout << "[PASS] " << #name << endl; \
        } catch (const exception& e) { \
            tests_failed++; \
            cout << "[FAIL] " << #name << ": " << e.what() << endl; \
        } catch (...) { \
            tests_failed++; \
            cout << "[FAIL] " << #name << ": Unknown error" << endl; \
        } \
    } \
    void test_##name()

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        throw runtime_error("Assertion failed: " #condition); \
    }

#define ASSERT_EQ(actual, expected) \
    if ((actual) != (expected)) { \
        throw runtime_error(string("Expected: ") + to_string(expected) + \
                           ", Actual: " + to_string(actual)); \
    }

// ============================================================================
// TEST CASES
// ============================================================================

TEST(BasicEnqueueDequeue) {
    SharedQueue<int> queue("test_basic", 10);

    // Enqueue
    bool enq_result = queue.enqueue(42);
    ASSERT_TRUE(enq_result);

    // Dequeue
    int value = -1;
    bool deq_result = queue.dequeue(value);
    ASSERT_TRUE(deq_result);
    ASSERT_EQ(value, 42);

    queue.cleanup();
}

TEST(FIFOOrder) {
    SharedQueue<int> queue("test_fifo", 10);

    // Enqueue 5 items
    for (int i = 0; i < 5; i++) {
        bool result = queue.enqueue(i);
        ASSERT_TRUE(result);
    }

    // Dequeue and verify order
    for (int i = 0; i < 5; i++) {
        int value = -1;
        bool result = queue.dequeue(value);
        ASSERT_TRUE(result);
        ASSERT_EQ(value, i);
    }

    queue.cleanup();
}

TEST(QueueEmpty) {
    SharedQueue<int> queue("test_empty", 10);

    // Try to dequeue from empty queue
    int value = -1;
    bool result = queue.dequeue(value, 100); // 100ms timeout
    ASSERT_TRUE(!result); // Should fail

    queue.cleanup();
}

TEST(QueueFull) {
    const int capacity = 5;
    SharedQueue<int> queue("test_full", capacity);

    // Fill the queue
    for (int i = 0; i < capacity; i++) {
        bool result = queue.enqueue(i);
        ASSERT_TRUE(result);
    }

    // Try to enqueue to full queue (should fail without blocking)
    // Note: This depends on implementation - some queues may block
    bool result = queue.enqueue(999, 100); // With timeout
    // Result depends on whether queue blocks or returns false

    queue.cleanup();
}

TEST(ConcurrentProducerConsumer) {
    SharedQueue<int> queue("test_concurrent", 100);
    std::atomic<int> sum{0};
    const int N = 1000;

    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < N; i++) {
            queue.enqueue(i);
        }
    });

    // Consumer thread
    std::thread consumer([&]() {
        for (int i = 0; i < N; i++) {
            int value = -1;
            while (!queue.dequeue(value, 10)) {
                // Retry until we get a value
            }
            sum += value;
        }
    });

    producer.join();
    consumer.join();

    // Sum should be 0 + 1 + ... + (N-1) = N*(N-1)/2
    int expected_sum = N * (N - 1) / 2;
    ASSERT_EQ(sum.load(), expected_sum);

    queue.cleanup();
}

TEST(MultipleProducersConsumers) {
    SharedQueue<int> queue("test_multi", 1000);
    std::atomic<int> sum{0};
    const int NUM_PRODUCERS = 4;
    const int NUM_CONSUMERS = 4;
    const int ITEMS_PER_PRODUCER = 250;

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // Create producers
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producers.emplace_back([&]() {
            for (int j = 0; j < ITEMS_PER_PRODUCER; j++) {
                queue.enqueue(1);
            }
        });
    }

    // Create consumers
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumers.emplace_back([&]() {
            for (int j = 0; j < ITEMS_PER_PRODUCER; j++) {
                int value = -1;
                while (!queue.dequeue(value, 10)) {
                    // Retry
                }
                sum += value;
            }
        });
    }

    // Join all threads
    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    int expected = NUM_PRODUCERS * ITEMS_PER_PRODUCER;
    ASSERT_EQ(sum.load(), expected);

    queue.cleanup();
}

TEST(StressTest) {
    SharedQueue<int> queue("test_stress", 500);
    std::atomic<bool> stop{false};
    std::atomic<int> enqueued{0};
    std::atomic<int> dequeued{0};

    const auto duration = std::chrono::seconds(2);
    const auto start = std::chrono::steady_clock::now();

    // Producer threads
    std::vector<std::thread> producers;
    for (int i = 0; i < 2; i++) {
        producers.emplace_back([&]() {
            int count = 0;
            while (!stop.load()) {
                if (queue.enqueue(count++, 10)) {
                    enqueued++;
                }
            }
        });
    }

    // Consumer threads
    std::vector<std::thread> consumers;
    for (int i = 0; i < 2; i++) {
        consumers.emplace_back([&]() {
            int value;
            while (!stop.load()) {
                if (queue.dequeue(value, 10)) {
                    dequeued++;
                }
            }
        });
    }

    // Run for specified duration
    std::this_thread::sleep_for(duration);
    stop.store(true);

    // Join all threads
    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    // Drain remaining items
    int value;
    while (queue.dequeue(value, 10)) {
        dequeued++;
    }

    cout << "  Enqueued: " << enqueued << ", Dequeued: " << dequeued << endl;
    ASSERT_EQ(enqueued.load(), dequeued.load());

    queue.cleanup();
}

TEST(WrapAround) {
    // Test circular buffer wrap-around
    const int capacity = 5;
    SharedQueue<int> queue("test_wrap", capacity);

    // Fill and empty multiple times
    for (int round = 0; round < 3; round++) {
        // Fill
        for (int i = 0; i < capacity; i++) {
            bool result = queue.enqueue(round * 100 + i);
            ASSERT_TRUE(result);
        }

        // Empty
        for (int i = 0; i < capacity; i++) {
            int value = -1;
            bool result = queue.dequeue(value);
            ASSERT_TRUE(result);
            ASSERT_EQ(value, round * 100 + i);
        }
    }

    queue.cleanup();
}

TEST(TimeoutBehavior) {
    SharedQueue<int> queue("test_timeout", 10);

    auto start = std::chrono::steady_clock::now();

    int value = -1;
    bool result = queue.dequeue(value, 500); // 500ms timeout

    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    ASSERT_TRUE(!result); // Should timeout
    ASSERT_TRUE(elapsed.count() >= 400); // Should wait at least 400ms
    ASSERT_TRUE(elapsed.count() <= 700); // Should not wait more than 700ms

    queue.cleanup();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    cout << "======================================" << endl;
    cout << "  SHAREDQUEUE UNIT TESTS" << endl;
    cout << "======================================" << endl << endl;

    run_test_BasicEnqueueDequeue();
    run_test_FIFOOrder();
    run_test_QueueEmpty();
    run_test_QueueFull();
    run_test_ConcurrentProducerConsumer();
    run_test_MultipleProducersConsumers();
    run_test_StressTest();
    run_test_WrapAround();
    run_test_TimeoutBehavior();

    cout << endl;
    cout << "======================================" << endl;
    cout << "  RESULTS" << endl;
    cout << "======================================" << endl;
    cout << "Passed: " << tests_passed << endl;
    cout << "Failed: " << tests_failed << endl;
    cout << "Total:  " << (tests_passed + tests_failed) << endl;
    cout << "======================================" << endl;

    return tests_failed > 0 ? 1 : 0;
}
