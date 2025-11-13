# REST API Server în C++ 🚀

Server REST API complet funcțional implementat în C++ cu arhitectură în straturi (Layered Architecture), multi-threading și IPC.

## 📋 Caracteristici

✅ **Arhitectură în 3 straturi:**
- **Controller Layer** - Gestionare endpoint-uri HTTP
- **Service Layer** - Logică de business și validări
- **Data Layer** - Repository Pattern pentru persistență

✅ **Concurență:**
- ThreadPool pentru procesare paralelă
- Multi-threading safe
- IPC (Inter-Process Communication)
- Sincronizare cu Mutex și Semaphore

✅ **HTTP:**
- Parsare cereri HTTP/1.1
- Routing dinamic cu parametri (/api/users/:id)
- Suport pentru GET, POST, PUT, DELETE
- Răspunsuri JSON

✅ **CRUD Complet:**
- GET /api/users - Obține toți utilizatorii
- GET /api/users/:id - Obține un utilizator specific
- POST /api/users - Creează un utilizator
- PUT /api/users/:id - Actualizează un utilizator
- DELETE /api/users/:id - Șterge un utilizator

## 🛠️ Compilare

```bash
cd Rest-API
mkdir -p build && cd build
cmake ..
make
```

## 🚀 Pornire Server

```bash
# Pornește pe portul 8080 cu 4 workers
./rest_api 8080 4
```

## 📡 API Endpoints - Testare cu curl

### 1. Health Check
```bash
curl http://localhost:8080/health
```

### 2. Obține toți utilizatorii
```bash
curl http://localhost:8080/api/users
```

### 3. Obține utilizator după ID
```bash
curl http://localhost:8080/api/users/1
```

### 4. Creează un utilizator nou
```bash
curl -X POST http://localhost:8080/api/users \
  -H "Content-Type: application/json" \
  -d '{"name":"Mihai Ionescu","email":"mihai@example.com"}'
```

### 5. Actualizează un utilizator
```bash
curl -X PUT http://localhost:8080/api/users/1 \
  -H "Content-Type: application/json" \
  -d '{"name":"Ion Updated","email":"ion.new@test.com"}'
```

### 6. Șterge un utilizator
```bash
curl -X DELETE http://localhost:8080/api/users/2
```

## 🧪 Script de Testare Automată

Rulează `tests/test_api.sh` pentru testare completă!

## 📊 Arhitectură

```
Client Request → Server → ThreadPool → Worker → Router → Controller → Service → Repository → Database
```

**Made with ❤️ în C++17**
