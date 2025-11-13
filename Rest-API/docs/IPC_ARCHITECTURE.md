# 📡 Arhitectura IPC (Inter-Process Communication)

## Prezentare Generală

Aplicația folosește **multiple mecanisme IPC** pentru comunicare între procese:

1. **Shared Memory** - Memorie partajată între procese
2. **Semafoare POSIX** - Sincronizare acces la resurse partajate
3. **Priority Queue** - Coadă cu priorități în memorie partajată
4. **Sockets TCP** - Comunicare client-server

---

## 1. SHARED MEMORY (Memorie Partajată)

### Descriere
- Permite proceselor să acceseze aceeași zonă de memorie
- Folosește `shm_open()` și `mmap()` (POSIX)
- Performanță maximă (nu copiază date între procese)

### Implementare
```
Fișier: include/ipc/sharedmemory.hpp
Fișier: src/ipc/sharedmemory.cpp
```

### Exemplu de Folosire
```cpp
// Procesul CREATOR
SharedMemory shm("/my_shared_mem", 4096, true);
int* shared_data = static_cast<int*>(shm.get_ptr());
*shared_data = 42;

// Procesul CONSUMER
SharedMemory shm("/my_shared_mem", 4096, false);
int* shared_data = static_cast<int*>(shm.get_ptr());
std::cout << "Value: " << *shared_data << "\n"; // Output: 42
```

### Avantaje
- ✓ Foarte rapid (zero-copy)
- ✓ Ideal pentru date mari
- ✓ Standard POSIX

### Dezavantaje
- ✗ Necesită sincronizare manuală
- ✗ Risk de race conditions

---

## 2. SEMAFOARE POSIX

### Descriere
- Mecanism de sincronizare pentru acces la resurse partajate
- Două tipuri: **named** (între procese) și **unnamed** (între thread-uri)
- Operații atomice: `wait()` și `post()`

### Implementare
```
Fișier: include/sync/semaphore.hpp
Fișier: src/sync/semaphore.cpp
```

### Tipuri de Semafoare

#### Named Semaphore (între procese)
```cpp
Semaphore sem("/my_sem", 1);  // Inițial value = 1 (mutex)
sem.wait();  // Decrementează (lock)
// ... secțiune critică ...
sem.post();  // Incrementează (unlock)
```

#### Unnamed Semaphore (între thread-uri)
```cpp
Semaphore sem(1);  // Inițial value = 1
sem.wait();
// ... secțiune critică ...
sem.post();
```

### Use Cases
1. **Mutex** - value = 1 (mutual exclusion)
2. **Counting semaphore** - value = N (resource pool)
3. **Signal** - value = 0 (notification între procese)

---

## 3. SHARED QUEUE (Coadă în Memorie Partajată)

### Descriere
- Coadă FIFO thread-safe între procese
- Combină Shared Memory + Semaphore
- Circular buffer pentru eficiență

### Implementare
```
Fișier: include/ipc/sharedqueue.hpp
```

### Structură Internă
```
┌─────────────────────────────────────┐
│  QueueHeader (Shared Memory)        │
│  - head: int                        │
│  - tail: int                        │
│  - size: int                        │
│  - capacity: int                    │
├─────────────────────────────────────┤
│  Elements[capacity] (Circular)      │
│  [0] [1] [2] ... [capacity-1]       │
└─────────────────────────────────────┘
        ↑                    ↑
        │                    │
    Semaphore            Semaphore
    (mutex)         (items_available)
```

### Exemplu de Folosire
```cpp
// Master Process (Producer)
SharedQueue<int> queue("/task_queue", 100, true);
queue.enqueue(42);  // Thread-safe

// Worker Process (Consumer)
SharedQueue<int> queue("/task_queue", 100, false);
int task = queue.dequeue();  // Blocking, thread-safe
```

### Sincronizare
1. **Mutex semaphore** - Protejează accesul la structură
2. **Wait pe dequeue** - Dacă coada e goală
3. **Check pe enqueue** - Dacă coada e plină

---

## 4. PRIORITY QUEUE cu FLAGS

### Descriere
- Coadă cu **priorități** în memorie partajată
- Folosește **HEAP** pentru sortare
- **FLAGS** pentru clasificare mesaje (URGENT/HIGH/NORMAL/LOW)
- FIFO în cadrul aceleiași priorități

### Implementare
```
Fișier: include/ipc/priorityqueue.hpp
```

### Flags Disponibile
```cpp
enum class MessageFlags : uint8_t {
    NORMAL = 0,   // Prioritate normală
    HIGH = 1,     // Prioritate înaltă
    URGENT = 2,   // Urgent (procesare imediată)
    LOW = 3       // Prioritate joasă
};
```

### Structură Internă
```
┌─────────────────────────────────────┐
│  PriorityMessage                    │
│  - flag: MessageFlags               │
│  - sequence_number: uint32_t        │
│  - data: T                          │
└─────────────────────────────────────┘
            ↓
    ┌────────────────┐
    │  MAX-HEAP      │
    │  [URGENT]      │  ← Root (highest priority)
    │  /        \    │
    │ [HIGH]  [HIGH] │
    │  /  \    /  \  │
    │[NORM][LOW][NORM]│
    └────────────────┘
```

### Exemplu de Folosire
```cpp
// Producer
PriorityQueue<Task> pq("/prio_queue", 100, true);
pq.enqueue(task1, MessageFlags::NORMAL);
pq.enqueue(task2, MessageFlags::URGENT);  // Va fi procesată PRIMA
pq.enqueue(task3, MessageFlags::HIGH);

// Consumer
auto msg = pq.dequeue();  // Returnează URGENT primul
// Procesează msg.data cu prioritate msg.flag
```

### Algoritm de Sortare
1. **Heap Property**: Parent >= Children (max-heap)
2. **Heapify Up**: La inserare, element "bubble up"
3. **Heapify Down**: La ștergere, element "bubble down"
4. **FIFO în prioritate**: Sequence number pentru ordine

---

## 5. ARHITECTURA COMPLETĂ IPC

```
┌─────────────────────────────────────────────────────┐
│                  MASTER PROCESS                     │
│  ┌──────────────────────────────────────────────┐  │
│  │  • Accept connections                         │  │
│  │  • Load balancing                            │  │
│  │  • Distribute tasks prin Priority Queue      │  │
│  └──────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
                          │
        ┌─────────────────┼─────────────────┐
        │                 │                 │
        ↓                 ↓                 ↓
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│  WORKER 1    │  │  WORKER 2    │  │  WORKER N    │
│              │  │              │  │              │
│ ThreadPool   │  │ ThreadPool   │  │ ThreadPool   │
│ (8 threads)  │  │ (8 threads)  │  │ (8 threads)  │
└──────────────┘  └──────────────┘  └──────────────┘
        │                 │                 │
        └─────────────────┼─────────────────┘
                          │
                          ↓
        ┌─────────────────────────────────────┐
        │      SHARED RESOURCES (IPC)         │
        ├─────────────────────────────────────┤
        │ • Priority Queue (task distribution)│
        │ • Shared Memory (metrics, state)    │
        │ • Semaphores (synchronization)      │
        │ • Request Stack (LIFO processing)   │
        └─────────────────────────────────────┘
```

### Fluxul unui Request

```
1. Client → Socket → Master Process
2. Master → Prioritize → Priority Queue (Shared Memory)
3. Worker.dequeue() → Semaphore.wait() → Get task
4. Worker → ThreadPool → Process request
5. Worker → Response → Socket → Client
```

---

## 6. DEMONSTRAȚIE PRACTICĂ

### Pornire Server
```bash
./rest_api 8080 4  # Port 8080, 4 workers
```

### Test IPC cu Client
```bash
# Terminal 1: Server
./rest_api

# Terminal 2: Client simplu
./simple_client 127.0.0.1 8080

# Terminal 3: Load test
./load_test_client 10 100  # 10 threads, 100 req/thread

# Terminal 4: IPC demo
./ipc_demo_client  # Trimite mesaje cu priorități diferite
```

### Observare Comportament
1. Mesajele URGENT sunt procesate PRIMA
2. Multiple workers procesează în paralel
3. Semafoarele previn race conditions
4. Shared memory permite comunicare rapidă

---

## 7. METRICI ȘI MONITORING

### Metrici Priority Queue
```cpp
// În cod:
std::cout << "Queue size: " << queue.get_size() << "\n";
```

### Metrici Semafoare
```cpp
// Debugging semaphore state
std::cout << "Semaphore operations successful\n";
```

---

## 8. BEST PRACTICES

### DO:
✓ Folosește semafoare pentru sincronizare
✓ Verifică întotdeauna return values
✓ Cleanup la exit (sem_unlink, shm_unlink)
✓ Folosește RAII wrappers
✓ Testează cu multiple procese

### DON'T:
✗ Nu accesa shared memory fără semaphore
✗ Nu uita să dai post() după wait()
✗ Nu lăsa resurse "leaked" (sem_unlink lipsă)
✗ Nu presupune că ordinea e garantată fără sincronizare
✗ Nu ignora erori (check errno!)

---

## 9. TROUBLESHOOTING

### Problem: "Semaphore failed"
```bash
# Verifică semafoare existente
ls -la /dev/shm/sem.*

# Șterge semafoare vechi
rm /dev/shm/sem.*
```

### Problem: "Shared memory failed"
```bash
# Verifică shared memory
ls -la /dev/shm/

# Șterge shared memory vechi
rm /dev/shm/*
```

### Problem: "Deadlock"
- Verifică că fiecare `wait()` are `post()` corespunzător
- Folosește `try_wait()` cu timeout
- Adaugă logging pentru debugging

---

## 10. REFERINȚE

- POSIX Semaphores: `man sem_overview`
- Shared Memory: `man shm_overview`
- Priority Queue: Cormen et al., "Introduction to Algorithms"
- Sockets: Stevens, "UNIX Network Programming"

---

## 11. STACK-based Request Processing

### Descriere
- Folosește structură STACK (LIFO - Last In First Out)
- Requests recente sunt procesate primele
- Beneficii pentru cache locality

### Implementare
```
Fișier: include/core/requeststack.hpp
Fișier: src/core/requeststack.cpp
```

### Argumentare Tehnică
```
STACK (LIFO) vs QUEUE (FIFO):

AVANTAJE STACK:
- Cache locality: Requests recente au date în cache
- Temporal locality: Utilizatori activi primesc răspunsuri rapide
- Simplicitate: Operații push/pop O(1)

DEZAVANTAJE:
- Posibil starvation pentru requests vechi
- Necesită mecanisme anti-aging
```

### Exemplu de Folosire
```cpp
RequestStack stack(1000);  // Max 1000 requests

// Producer (Master)
stack.push(request, client_fd, MessageFlags::HIGH);

// Consumer (Worker)
auto entry = stack.pop();  // LIFO - cel mai recent request
handle_request(entry.request, entry.client_fd);
```

### Statistics
```cpp
auto stats = stack.get_stats();
std::cout << "Stack size: " << stats.current_size << "\n";
std::cout << "Total pushed: " << stats.total_pushed << "\n";
std::cout << "Total popped: " << stats.total_popped << "\n";
std::cout << "Rejected (full): " << stats.total_rejected << "\n";
```

---

## 12. COMPARAȚIE STRUCTURI DE DATE

### Priority Queue vs Shared Queue vs Stack

| Feature | Priority Queue | Shared Queue | Stack |
|---------|---------------|--------------|-------|
| Ordine procesare | Prioritate + FIFO | FIFO | LIFO |
| Complexitate enqueue | O(log n) | O(1) | O(1) |
| Complexitate dequeue | O(log n) | O(1) | O(1) |
| Use case | Requests critice | Task distribution | Recent requests |
| Sincronizare | 2 semafoare | 2 semafoare | mutex + CV |
| Shared memory | Da | Da | Nu (in-process) |

### Când folosim fiecare:

**Priority Queue**:
- Requests cu priorități diferite (URGENT, HIGH, NORMAL, LOW)
- Sisteme critice unde ordinea contează
- SLA (Service Level Agreement) diferite per request

**Shared Queue**:
- Task distribution uniform între workers
- Load balancing simplu
- Când toate tasks au aceeași prioritate

**Stack**:
- Procesare requests recente (cache-friendly)
- Requests de la utilizatori activi
- Situații unde LIFO are sens business

---

## 13. DIAGRAMĂ FLUXURI DETALIATE

### Flux Priority Queue

```
CLIENT REQUEST
    ↓
DETERMINE PRIORITY (Urgent/High/Normal/Low)
    ↓
ENQUEUE cu FLAG
    ↓
┌────────────────────────────┐
│  Semaphore.wait() (mutex)  │
├────────────────────────────┤
│  Insert in Heap            │
│  Heapify Up                │
│  sequence_number++         │
├────────────────────────────┤
│  Semaphore.post() (mutex)  │
│  Semaphore.post() (items)  │
└────────────────────────────┘
    ↓
WORKERS (multiple procese)
    ↓
DEQUEUE
    ↓
┌────────────────────────────┐
│  Semaphore.wait() (items)  │ ← Blocking dacă gol
│  Semaphore.wait() (mutex)  │
├────────────────────────────┤
│  Extract Root (max prior.) │
│  Heapify Down              │
├────────────────────────────┤
│  Semaphore.post() (mutex)  │
└────────────────────────────┘
    ↓
PROCESS REQUEST
    ↓
SEND RESPONSE
```

### Flux Stack

```
CLIENT REQUEST (recent)
    ↓
PUSH to Stack
    ↓
┌────────────────────────────┐
│  mutex.lock()              │
├────────────────────────────┤
│  stack.push_back(request)  │ ← LIFO
│  timestamp = now()         │
├────────────────────────────┤
│  mutex.unlock()            │
│  cv.notify_one()           │
└────────────────────────────┘
    ↓
WORKER THREAD
    ↓
POP from Stack
    ↓
┌────────────────────────────┐
│  cv.wait() until !empty    │
│  mutex.lock()              │
├────────────────────────────┤
│  entry = stack.back()      │ ← Most recent
│  stack.pop_back()          │
├────────────────────────────┤
│  mutex.unlock()            │
└────────────────────────────┘
    ↓
PROCESS (recent request first)
```

---

## 14. CONCLUZIE

Arhitectura IPC a acestui proiect demonstrează:

✓ **Înțelegere profundă** a mecanismelor POSIX IPC
✓ **Implementare practică** de structuri de date avansate (Heap)
✓ **Sincronizare complexă** cu semafoare multiple
✓ **Production-ready code** cu error handling și monitoring
✓ **Scalabilitate** prin multi-processing și shared memory

### Concepte Demonstrate

1. **IPC Mechanisms**: Shared Memory, Semaphores, Sockets
2. **Data Structures**: Heap (Priority Queue), Stack, Circular Buffer
3. **Synchronization**: Mutex, Condition Variables, Atomic operations
4. **Process Management**: fork(), Process pools, Worker lifecycle
5. **Thread Management**: Thread pools, Lock-free queues
6. **Performance**: Zero-copy, Cache locality, Lock contention minimization

---

**Autor**: Proiect de Licență - REST API Framework
**Data**: 2025
**Tehnologii**: C++17, POSIX, Linux IPC
