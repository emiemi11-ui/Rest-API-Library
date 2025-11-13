// Client care demonstrează IPC prin Priority Queue
#include <iostream>
#include "ipc/priorityqueue.hpp"
#include <unistd.h>
#include <cstring>

struct IPCMessage {
    char text[256];
    int sender_pid;
};

int main() {
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║     IPC Demo Client                   ║\n";
    std::cout << "╚════════════════════════════════════════╝\n\n";

    try {
        // Conectare la Priority Queue shared între procese
        PriorityQueue<IPCMessage> queue("/rest_api_pq", 100, false);

        IPCMessage msg;
        msg.sender_pid = getpid();

        // Trimite mesaj URGENT
        strcpy(msg.text, "URGENT: Database connection lost!");
        queue.enqueue(msg, MessageFlags::URGENT);
        std::cout << "[Client] Sent URGENT message\n";

        sleep(1);

        // Trimite mesaj HIGH priority
        strcpy(msg.text, "HIGH: New user registration");
        queue.enqueue(msg, MessageFlags::HIGH);
        std::cout << "[Client] Sent HIGH priority message\n";

        sleep(1);

        // Trimite mesaj NORMAL
        strcpy(msg.text, "NORMAL: Health check request");
        queue.enqueue(msg, MessageFlags::NORMAL);
        std::cout << "[Client] Sent NORMAL message\n";

        sleep(1);

        // Trimite mesaj LOW
        strcpy(msg.text, "LOW: Background cleanup task");
        queue.enqueue(msg, MessageFlags::LOW);
        std::cout << "[Client] Sent LOW priority message\n";

        std::cout << "\n[Client] All messages sent! Server will process them by priority.\n";
        std::cout << "[Client] Expected order: URGENT → HIGH → NORMAL → LOW\n";

    } catch (const std::exception& e) {
        std::cerr << "[Client] Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
