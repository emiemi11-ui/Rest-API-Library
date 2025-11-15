#include "../../framework/include/restapi.hpp"
#include <iostream>
#include <string>
#include <sstream>

using namespace RestAPI;

int main() {
    // Create framework instance
    RestApiFramework app(8080, 2);

    // Enable CORS
    app.enable_cors(true);

    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║         EXAMPLE 1: SIMPLE API                  ║\n";
    std::cout << "║         Demonstrație REST API Framework        ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    std::cout << "\n";

    // ===== ENDPOINT 1: Hello World =====
    app.get("/", [](const Request& req) {
        (void)req; // unused
        return Response::json(200, R"({
            "message": "Hello, World!",
            "framework": "REST API Framework C++",
            "version": "1.0.0"
        })");
    });

    // ===== ENDPOINT 2: Echo =====
    app.post("/echo", [](const Request& req) {
        std::string echoed = req.getBody();
        std::string response = R"({"echoed": ")" + echoed + R"("})";
        return Response::json(200, response);
    });

    // ===== ENDPOINT 3: Calculator - Addition =====
    app.get("/add/:a/:b", [](const Request& req) {
        try {
            int a = std::stoi(req.getParam("a"));
            int b = std::stoi(req.getParam("b"));
            int result = a + b;

            std::ostringstream oss;
            oss << R"({)"
                << R"("operation": "addition",)"
                << R"("a": )" << a << ","
                << R"("b": )" << b << ","
                << R"("result": )" << result
                << R"(})";

            return Response::json(200, oss.str());
        } catch (...) {
            return Response::json(400, R"({"error": "Invalid numbers"})");
        }
    });

    // ===== ENDPOINT 4: Calculator - Subtraction =====
    app.get("/sub/:a/:b", [](const Request& req) {
        try {
            int a = std::stoi(req.getParam("a"));
            int b = std::stoi(req.getParam("b"));
            int result = a - b;

            std::ostringstream oss;
            oss << R"({)"
                << R"("operation": "subtraction",)"
                << R"("a": )" << a << ","
                << R"("b": )" << b << ","
                << R"("result": )" << result
                << R"(})";

            return Response::json(200, oss.str());
        } catch (...) {
            return Response::json(400, R"({"error": "Invalid numbers"})");
        }
    });

    // ===== ENDPOINT 5: Calculator - Multiplication =====
    app.get("/mul/:a/:b", [](const Request& req) {
        try {
            int a = std::stoi(req.getParam("a"));
            int b = std::stoi(req.getParam("b"));
            int result = a * b;

            std::ostringstream oss;
            oss << R"({)"
                << R"("operation": "multiplication",)"
                << R"("a": )" << a << ","
                << R"("b": )" << b << ","
                << R"("result": )" << result
                << R"(})";

            return Response::json(200, oss.str());
        } catch (...) {
            return Response::json(400, R"({"error": "Invalid numbers"})");
        }
    });

    // ===== ENDPOINT 6: Calculator - Division =====
    app.get("/div/:a/:b", [](const Request& req) {
        try {
            int a = std::stoi(req.getParam("a"));
            int b = std::stoi(req.getParam("b"));

            if (b == 0) {
                return Response::json(400, R"({"error": "Division by zero"})");
            }

            double result = static_cast<double>(a) / static_cast<double>(b);

            std::ostringstream oss;
            oss << R"({)"
                << R"("operation": "division",)"
                << R"("a": )" << a << ","
                << R"("b": )" << b << ","
                << R"("result": )" << result
                << R"(})";

            return Response::json(200, oss.str());
        } catch (...) {
            return Response::json(400, R"({"error": "Invalid numbers"})");
        }
    });

    // ===== ENDPOINT 7: Greet user =====
    app.get("/greet/:name", [](const Request& req) {
        std::string name = req.getParam("name");
        std::string response = R"({"message": "Hello, )" + name + R"(!", "timestamp": "2024-11-15"})";
        return Response::json(200, response);
    });

    // ===== ENDPOINT 8: Health check =====
    app.get("/health", [](const Request& req) {
        (void)req;
        return Response::json(200, R"({
            "status": "healthy",
            "uptime": "running",
            "framework": "REST API Framework"
        })");
    });

    // Print available endpoints
    std::cout << "\n📍 Available Endpoints:\n";
    std::cout << "  GET  /                   - Hello World\n";
    std::cout << "  POST /echo               - Echo request body\n";
    std::cout << "  GET  /add/:a/:b          - Add two numbers\n";
    std::cout << "  GET  /sub/:a/:b          - Subtract two numbers\n";
    std::cout << "  GET  /mul/:a/:b          - Multiply two numbers\n";
    std::cout << "  GET  /div/:a/:b          - Divide two numbers\n";
    std::cout << "  GET  /greet/:name        - Greet a user\n";
    std::cout << "  GET  /health             - Health check\n";
    std::cout << "\n";
    std::cout << "💡 Examples:\n";
    std::cout << "  curl http://localhost:8080/\n";
    std::cout << "  curl http://localhost:8080/add/5/3\n";
    std::cout << "  curl http://localhost:8080/greet/John\n";
    std::cout << "  curl -X POST -d 'test' http://localhost:8080/echo\n";
    std::cout << "\n";

    // Start server
    app.start();

    return 0;
}
