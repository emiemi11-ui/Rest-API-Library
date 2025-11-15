# CAPITOLUL 1: INTRODUCERE

## 1.1. Context și Motivație

În era modernă a dezvoltării software, API-urile REST (Representational State Transfer) au devenit standard de facto pentru comunicarea între sisteme distribuite. De la aplicații web și mobile până la sisteme IoT (Internet of Things) și servicii cloud, API-urile REST facilitează integrarea și interoperabilitatea între componente heterogene.

### 1.1.1. Importanța Framework-urilor REST

Framework-urile pentru dezvoltarea API-urilor REST oferă dezvoltatorilor:
- **Abstractizare**: Eliminarea codului repetitiv (boilerplate)
- **Reutilizabilitate**: Componente generice aplicabile în multiple contexte
- **Performanță**: Optimizări la nivel de infrastructură
- **Productivitate**: Reducerea timpului de development

### 1.1.2. Performanța în C++

C++ rămâne limbajul de alegere pentru sisteme cu cerințe stricte de performanță datorită:
- **Control la nivel scăzut**: Acces direct la resurse hardware
- **Zero-overhead abstraction**: Abstractizări fără penalizare de performanță
- **Determinism**: Comportament previzibil în alocarea memoriei
- **Ecosistem POSIX**: Acces nativ la API-uri de sistem pentru multi-processing și IPC

### 1.1.3. Nevoia de Reutilizabilitate

Problema majoră în dezvoltarea API-urilor REST în C++ este lipsa de **genericitate și reutilizabilitate**:

**Abordarea tradițională (GREȘITĂ):**
```cpp
// Implementare hardcodata pentru un singur domeniu (E-Commerce)
class ECommerceServer {
    void handleProductRequest() { /* logic specific */ }
    void handleOrderRequest() { /* logic specific */ }
    // ❌ NU poate fi reutilizat pentru Banking, IoT, etc.
};
```

**Abordarea propusă (CORECTĂ):**
```cpp
// Framework generic reutilizabil pentru ORICE domeniu
#include <restapi.hpp>

RestApiFramework app(8080, 4);
app.get("/products", handleProducts);  // E-Commerce
app.get("/sensors", handleSensors);     // IoT
app.get("/accounts", handleAccounts);   // Banking
// ✅ Același framework, domenii diferite
```

### 1.1.4. Motivația Proiectului

Această lucrare de licență răspunde la următoarele nevoi identificate:

1. **Lipsa framework-urilor C++ moderne și ușor de utilizat**
   - Framework-urile existente (Crow, Pistache) sunt complexe și greu de integrat
   - API-urile sunt verbale și necesită cod extins pentru task-uri simple

2. **Necesitatea arhitecturilor enterprise-grade**
   - Multe framework-uri C++ sunt mono-thread sau folosesc doar thread pool
   - Lipsesc implementări practice ale pattern-ului Master/Worker cu IPC

3. **Cerințele academice și practice**
   - Demonstrarea conceptelor de Sisteme de Operare (PSO): multi-processing, IPC, sincronizare
   - Aplicabilitate practică în industrie pentru sisteme high-performance

4. **Generalizare și extensibilitate**
   - Aplicații din domenii diverse (E-Commerce, Banking, IoT, Medical) au structuri API similare
   - Un framework generic poate reduce duplicarea codului și accelera dezvoltarea

## 1.2. Problema Identificată

### 1.2.1. Framework-uri Monolitice

Framework-urile REST actuale sunt adesea **legate strâns de un domeniu specific**:

**Exemplu problematic:**
```cpp
// ❌ BAD: Serverul IoT nu poate fi reutilizat pentru Banking
class IoTServer {
    SQLiteDB sensors_db;  // Hardcodat pentru senzori
    void processSensorData() { /* hardcoded logic */ }
};

// Dacă vrei Banking, trebuie să rescrii TOTUL:
class BankingServer {
    SQLiteDB accounts_db; // Rescris de la zero
    void processTransaction() { /* rescris de la zero */ }
};
```

**Consecințe:**
- Duplicare masivă de cod (HTTP parsing, routing, worker management, etc.)
- Timp lung de development pentru fiecare nou proiect
- Erori introduse prin re-implementare
- Dificultate în mentenanță

### 1.2.2. Lipsa Separării Responsabilităților

În abordarea tradițională, **business logic** și **infrastructură** sunt amestecate:

```cpp
// ❌ BAD: Logică de business + infrastructură în același loc
void handle_request(int client_fd) {
    // Infrastructură (ar trebui să fie în framework)
    char buffer[4096];
    read(client_fd, buffer, sizeof(buffer));
    parse_http(buffer);  // HTTP parsing

    // Business logic (ar trebui să fie în aplicație)
    Product p = db.getProduct(id);
    string json = serialize(p);

    // Infrastructură (iar în framework)
    send_http_response(client_fd, json);
}
```

### 1.2.3. Performanță Sub-optimă

Multe framework-uri C++ nu exploatează complet capabilitățile sistemelor multi-core moderne:

**Probleme identificate:**
1. **Doar multi-threading**: Nu folosesc multi-processing pentru izolare reală
2. **IPC limitat**: Nu folosesc shared memory și semafoare pentru comunicare eficientă
3. **I/O blocking**: Nu folosesc epoll/kqueue pentru non-blocking I/O
4. **Single point of failure**: Crash într-un thread afectează tot serverul

### 1.2.4. Dificultate în Scalare

Scaling vertical (mai multe cores) și scaling horizontal (mai multe servere) sunt dificile:
- Lipsă de izolare între workers (un crash => crash total)
- Dificultate în distribuirea load-ului
- Lipsă de metrici și monitoring

## 1.3. Obiectivele Lucrării

### 1.3.1. Obiectiv General

**Proiectarea și implementarea unui framework REST API în C++ care este:**
- **Generic și reutilizabil** pentru multiple domenii (E-Commerce, Banking, IoT, Medical, etc.)
- **Performant și scalabil** prin arhitectură multi-processing cu IPC
- **Simplu și intuitiv** cu API inspirat din Express.js
- **Production-ready** cu features enterprise (graceful shutdown, health checks, logging)

### 1.3.2. Obiective Specifice

#### 1. Arhitectură Multi-Processing

**Obiectiv:** Implementare reală a pattern-ului Master/Worker folosind `fork()` pentru:
- Izolare între procese (crash într-un worker nu afectează restul)
- Utilizare optimă a CPU-urilor multi-core
- Fault tolerance și auto-restart pentru workers

**Tehnologii:** `fork()`, `waitpid()`, `kill()`, signal handling (`sigaction`)

#### 2. Inter-Process Communication (IPC)

**Obiectiv:** Comunicare eficientă între Master și Workers prin:
- **Shared Memory**: Partajarea statisticilor și a cozii de task-uri
- **Semafoare**: Sincronizare thread-safe a accesului la resurse partajate
- **Producer-Consumer pattern**: Master produce conexiuni, Workers le consumă

**Tehnologii:** `shm_open()`, `mmap()`, `sem_open()`, `sem_wait()`, `sem_post()`

#### 3. Multi-Threading în Workers

**Obiectiv:** Fiecare worker să aibă un ThreadPool pentru:
- Procesare paralelă a request-urilor
- Utilizare eficientă a resurselor
- Load balancing intern

**Tehnologii:** `pthread_create()`, `pthread_mutex_lock()`, condition variables

#### 4. I/O Non-Blocking

**Obiectiv:** Acceptare eficientă a conexiunilor prin:
- **epoll()**: Edge-triggered monitoring pentru scalabilitate
- **Non-blocking sockets**: Pentru evitarea blocării Master-ului

**Tehnologii:** `epoll_create()`, `epoll_ctl()`, `epoll_wait()`, `fcntl(O_NONBLOCK)`

#### 5. API Simplu și Intuitiv

**Obiectiv:** API la nivel de framework care să permită:
```cpp
RestApiFramework app(port, workers);

app.get("/path", handler);
app.post("/path", handler);

app.start();  // Pornește serverul
```

**Inspirație:** Express.js (JavaScript), Flask (Python), Gin (Go)

#### 6. Validare prin Exemple Practice

**Obiectiv:** Demonstrarea versatilității prin implementarea a 5 domenii diferite:
1. **Simple API**: Calculator și utility endpoints (fără database)
2. **E-Commerce**: Products, Users, Orders (CRUD complet)
3. **IoT Sensors**: Colectare și agregare date senzori
4. **Banking**: Conturi, tranzacții, transferuri
5. **Medical Records**: Pacienți, consultatii, istoric medical

**Criterii de succes:**
- Același framework (librestapi.a) pentru toate
- ZERO modificări în codul framework-ului
- Doar business logic diferă între domenii

#### 7. Performanță Competitivă

**Obiectiv:** Atingerea unor performanțe comparabile sau superioare framework-urilor existente:
- **Target throughput**: > 40,000 requests/sec
- **Target latency**: < 10ms (P95)
- **Memory usage**: < 300MB pentru 4 workers

**Metodologie:** Benchmark-uri cu Apache Bench și wrk

## 1.4. Contribuții

### 1.4.1. Contribuții Științifice

1. **Demonstrație practică a arhitecturii Master/Worker în C++**
   - Implementare completă cu IPC POSIX
   - Pattern production-ready pentru alte sisteme

2. **Studiu comparativ al framework-urilor REST**
   - Analiza C++ (Crow, Pistache) vs alte limbaje (Node.js, Python)
   - Identificarea trade-off-urilor (performanță vs developer experience)

3. **Evaluare empirică a performanței multi-processing vs multi-threading**
   - Benchmark-uri concrete
   - Măsurători de throughput, latency, scalabilitate

### 1.4.2. Contribuții Tehnice

1. **Framework open-source complet funcțional**
   - Licență MIT pentru reutilizare liberă
   - Cod bine documentat și structurat
   - Exemple practice pentru 5 domenii

2. **Implementare educațională a conceptelor PSO**
   - fork() și managementul proceselor
   - IPC cu shared memory și semafoare
   - epoll() pentru I/O asincron
   - Thread Pool și sincronizare

3. **Pattern-uri de design reutilizabile**
   - Separarea responsabilităților (framework vs aplicație)
   - Router cu pattern matching pentru paths dinamice
   - Middleware pattern pentru cross-cutting concerns
   - Connection pooling pentru database

### 1.4.3. Contribuții Aplicative

1. **Accelerarea dezvoltării de API-uri în C++**
   - Reducerea timpului de development cu 80%
   - Eliminarea codului boilerplate

2. **Baseline pentru proiecte viitoare**
   - Poate fi extins cu WebSocket, HTTP/2, gRPC
   - Poate fi integrat cu sisteme de monitoring (Prometheus)
   - Poate fi containerizat (Docker) și orchestrat (Kubernetes)

## 1.5. Organizarea Lucrării

Această lucrare este structurată în 6 capitole:

**Capitolul 1 - Introducere**
- Context și motivație
- Problema identificată
- Obiective și contribuții

**Capitolul 2 - Related Work (Studiul Literaturii)**
- Framework-uri REST în alte limbaje
- Framework-uri REST în C++
- Concepte teoretice (multi-processing, IPC, threading)
- Gap analysis

**Capitolul 3 - Arhitectură și Design**
- Cerințe funcționale și non-funcționale
- Arhitectura de nivel înalt (3-layer architecture)
- Arhitectura multi-processing (Master/Worker)
- Mecanismele IPC (shared memory, semaphores)
- Designul API-ului public

**Capitolul 4 - Implementare**
- Tehnologii utilizate
- Structura proiectului
- Detalii implementare Master Process
- Detalii implementare Worker Process
- Detalii implementare IPC
- Detalii implementare HTTP parsing și routing

**Capitolul 5 - Testare și Validare**
- Metodologie de testare
- Teste unitare
- Teste de integrare
- Benchmark-uri de performanță
- Comparație cu alte framework-uri
- Validare prin use-case-uri practice

**Capitolul 6 - Concluzii și Perspective**
- Rezultate obținute
- Contribuții
- Limitări
- Direcții viitoare

**Bibliografie**
- Minim 20 de referințe (cărți, articole, documentație tehnică)

**Anexe (opțional)**
- Fragmente de cod relevante
- Configurații de testare
- Grafice suplimentare

---

## Referințe Folosite în Capitolul 1

[1] R. Fielding, "Architectural Styles and the Design of Network-based Software Architectures",
    PhD thesis, University of California, Irvine, 2000.

[2] B. Stroustrup, "The C++ Programming Language", 4th Edition, Addison-Wesley, 2013.

[3] W. R. Stevens, S. A. Rago, "Advanced Programming in the UNIX Environment",
    3rd Edition, Addison-Wesley, 2013.

[4] M. Kerrisk, "The Linux Programming Interface", No Starch Press, 2010.

---

**Total pagini estimate pentru Capitolul 1: 3-4 pagini**
