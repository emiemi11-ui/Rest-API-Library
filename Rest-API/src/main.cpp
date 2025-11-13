#include "core/server.hpp"
#include "http/router.hpp"
#include "http/request.hpp"
#include "http/response.hpp"

#include "data/sqlitedatabase.hpp"
#include "data/databaseconnection.hpp"
#include "data/userrepository.hpp"
#include "services/userservice.hpp"
#include "controllers/usercontroller.hpp"

#include <iostream>
#include <memory>
#include <cstdlib>
#include <map>
#include <string>

// --- helper minim pentru parsing de query din target (/path?key=val&...)
static std::map<std::string,std::string> parseQuery(const std::string& target) {
    std::map<std::string,std::string> out;
    auto qpos = target.find('?');
    if (qpos == std::string::npos) return out;
    auto q = target.substr(qpos + 1);
    size_t i = 0;
    while (i < q.size()) {
        size_t amp = q.find('&', i);
        std::string pair = (amp == std::string::npos) ? q.substr(i) : q.substr(i, amp - i);
        size_t eq = pair.find('=');
        std::string k = (eq == std::string::npos) ? pair : pair.substr(0, eq);
        std::string v = (eq == std::string::npos) ? ""   : pair.substr(eq + 1);
        out[k] = v; // NB: fără URL decode pentru simplitate
        if (amp == std::string::npos) break;
        i = amp + 1;
    }
    return out;
}

int main(int argc, char** argv) {
    int port = 8080;
    int workers = 4;
    if (argc > 1) port = std::atoi(argv[1]);
    if (argc > 2) workers = std::atoi(argv[2]);

    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║     REST API Server cu Autentificare          ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";

    // 1) DB: SQLite prin interfața generică
    std::cout << "[INIT] Conectare la SQLite (app.db)...\n";
    auto sqlite = std::make_unique<SqliteDatabase>();
    DatabaseConnection db(std::move(sqlite), {{"file","app.db"}});
    if (!db.connect()) {
        std::cerr << "[EROARE] Nu m-am putut conecta la SQLite (app.db)\n";
        return 1;
    }
    std::cout << "[OK] Conectat la baza de date\n";

    // 2) Initializează Repository + Service + Controller (Layered Architecture)
    std::cout << "[INIT] Initializare arhitectură: Repository → Service → Controller\n";
    UserRepository userRepo(db);
    userRepo.init();
    std::cout << "[OK] Repository initializat\n";

    UserService userService(userRepo);
    std::cout << "[OK] Service initializat\n";

    UserController userController(userService);
    std::cout << "[OK] Controller initializat\n";

    // 3) Router: configurare endpoint-uri
    std::cout << "\n[INIT] Configurare endpoint-uri...\n";
    Router router;

    // Health check
    router.get("/health", [](const HttpRequest& req, const std::map<std::string,std::string>& params) {
        (void)req; (void)params;
        return HttpResponse::json(200, "{\"status\":\"OK\",\"message\":\"Server is running\"}");
    });
    std::cout << "  ✓ GET  /health\n";

    // ========== AUTENTIFICARE ==========
    // POST /api/auth/register - Înregistrare
    router.post("/api/auth/register", [&userController](const HttpRequest& req,
                                                        const std::map<std::string,std::string>& params) {
        userController.setRawRequest(req.raw);
        return userController.registerUser(req, params);
    });
    std::cout << "  ✓ POST /api/auth/register\n";

    // POST /api/auth/login - Login
    router.post("/api/auth/login", [&userController](const HttpRequest& req,
                                                      const std::map<std::string,std::string>& params) {
        userController.setRawRequest(req.raw);
        return userController.loginUser(req, params);
    });
    std::cout << "  ✓ POST /api/auth/login\n";

    // ========== CRUD USERS ==========
    // GET /api/users - Lista utilizatori
    router.get("/api/users", [&userController](const HttpRequest& req,
                                               const std::map<std::string,std::string>& params) {
        return userController.getAll(req, params);
    });
    std::cout << "  ✓ GET  /api/users\n";

    // GET /api/users/:id - User specific
    router.get("/api/users/:id", [&userController](const HttpRequest& req,
                                                    const std::map<std::string,std::string>& params) {
        return userController.getById(req, params);
    });
    std::cout << "  ✓ GET  /api/users/:id\n";

    // POST /api/users - Creează user
    router.post("/api/users", [&userController](const HttpRequest& req,
                                                const std::map<std::string,std::string>& params) {
        userController.setRawRequest(req.raw);
        return userController.create(req, params);
    });
    std::cout << "  ✓ POST /api/users\n";

    // PUT /api/users/:id - Actualizează user
    router.put("/api/users/:id", [&userController](const HttpRequest& req,
                                                    const std::map<std::string,std::string>& params) {
        userController.setRawRequest(req.raw);
        return userController.update(req, params);
    });
    std::cout << "  ✓ PUT  /api/users/:id\n";

    // DELETE /api/users/:id - Șterge user
    router.del("/api/users/:id", [&userController](const HttpRequest& req,
                                                    const std::map<std::string,std::string>& params) {
        return userController.remove(req, params);
    });
    std::cout << "  ✓ DEL  /api/users/:id\n";

    // ========== LEGACY ENDPOINTS (pentru compatibilitate) ==========
    // POST /api/users/add?name=Ana
    router.post("/api/users/add", [&userRepo](const HttpRequest& req,
                                              const std::map<std::string,std::string>& params) {
        (void)params;
        auto q = parseQuery(req.target);
        auto it = q.find("name");
        if (it == q.end() || it->second.empty()) {
            return HttpResponse::json(400, "{\"error\":\"missing name\"}");
        }
        bool ok = userRepo.add(it->second);
        return HttpResponse::json(ok ? 201 : 500, ok ? "{\"ok\":true}" : "{\"ok\":false}");
    });
    std::cout << "  ✓ POST /api/users/add (legacy)\n";

    // 4) Server HTTP
    std::cout << "\n[INIT] Pornire server HTTP...\n";
    Server server(port, workers);
    server.setRouter(router);

    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║  🚀 Server pornit cu succes!                  ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  Port:     " << port << "                                ║\n";
    std::cout << "║  Workers:  " << workers << "                                  ║\n";
    std::cout << "║  Database: SQLite (app.db)                     ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  📡 Endpoints disponibile:                     ║\n";
    std::cout << "║    • POST /api/auth/register                   ║\n";
    std::cout << "║    • POST /api/auth/login                      ║\n";
    std::cout << "║    • GET  /api/users                           ║\n";
    std::cout << "║    • POST /api/users                           ║\n";
    std::cout << "║    • GET  /api/users/:id                       ║\n";
    std::cout << "║    • PUT  /api/users/:id                       ║\n";
    std::cout << "║    • DEL  /api/users/:id                       ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    std::cout << "Server listening on http://localhost:" << port << "\n\n";

    server.start();
    return 0;
}
