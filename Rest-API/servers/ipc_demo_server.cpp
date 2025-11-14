// IPC Demo Server - Demonstrează Priority Queue între procese
// Acest server creează PriorityQueue în shared memory și procesează mesajele

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <atomic>
#include "ipc/priorityqueue.hpp"

// Structura mesajului (trebuie să fie identică cu cea din client)
struct IPCMessage {
    char text[256];
    int sender_pid;
};

std::atomic<bool> running{true};

void signal_handler(int signum) {
    std::cout << "\n[Server] Received signal " << signum << ", shutting down...\n";
    running = false;
}

int main() {
    // Setup signal handling
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║     IPC Demo Server - Priority Queue          ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";

    try {
        // Creează Priority Queue în shared memory (creator=true)
        std::cout << "[Server] Creating Priority Queue in shared memory...\n";
        PriorityQueue<IPCMessage> queue("/rest_api_pq", 100, true);
        std::cout << "[Server] ✓ Priority Queue created successfully!\n";
        std::cout << "[Server] Waiting for messages from clients...\n";
        std::cout << "[Server] Press Ctrl+C to stop\n\n";

        int processed_count = 0;

        // Main processing loop
        while (running) {
            try {
                // Try to dequeue with timeout (non-blocking check)
                PriorityMessage<IPCMessage> msg;

                // Check if queue has messages (non-blocking)
                if (!queue.is_empty()) {
                    msg = queue.dequeue();

                    // Process message
                    std::string priority_str;
                    switch (msg.flag) {
                        case MessageFlags::URGENT: priority_str = "🚨 URGENT"; break;
                        case MessageFlags::HIGH:   priority_str = "⚠️  HIGH"; break;
                        case MessageFlags::NORMAL: priority_str = "ℹ️  NORMAL"; break;
                        case MessageFlags::LOW:    priority_str = "📋 LOW"; break;
                    }

                    std::cout << "[Server] " << priority_str
                              << " | From PID: " << msg.data.sender_pid
                              << " | Seq: " << msg.sequence_number
                              << "\n         Message: " << msg.data.text << "\n";

                    processed_count++;
                } else {
                    // No messages, sleep briefly
                    usleep(100000); // 100ms
                }

            } catch (const std::exception& e) {
                if (running) {
                    std::cerr << "[Server] Error processing message: " << e.what() << "\n";
                }
            }
        }

        std::cout << "\n[Server] Processed " << processed_count << " messages total\n";
        std::cout << "[Server] Shutting down gracefully...\n";

    } catch (const std::exception& e) {
        std::cerr << "[Server] Fatal error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "[Server] Goodbye!\n";
    return 0;
}
