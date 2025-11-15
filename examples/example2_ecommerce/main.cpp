#include "../../framework/include/restapi.hpp"
#include "../../infrastructure/include/data/sqlitedatabase.hpp"
#include "../../infrastructure/include/data/connectionpool.hpp"
#include "include/models/product.hpp"
#include "include/models/order.hpp"
#include "include/models/user.hpp"
#include "include/data/productrepository.hpp"
#include "include/services/productservice.hpp"
#include "include/controllers/productcontroller.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <memory>

using namespace RestAPI;

int main() {
    // Create framework instance
    RestApiFramework app(8080, 4);

    // Enable CORS
    app.enable_cors(true);

    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║      EXAMPLE 2: E-COMMERCE API                 ║\n";
    std::cout << "║      Full-Featured Online Shop                 ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    std::cout << "\n";

    // Initialize database
    std::cout << "🔧 Initializing database...\n";
    std::map<std::string, std::string> db_config = {
        {"database", "ecommerce.db"}
    };
    auto db = std::make_unique<SqliteDatabase>();
    DatabaseConnection conn(std::move(db), db_config);
    conn.connect();

    // Initialize repositories and services
    ProductRepository productRepo(conn);
    ProductService productService(productRepo);
    ProductController productController(productService);

    std::cout << "✅ Database and services initialized\n\n";

    // ===== PRODUCT ENDPOINTS =====

    // GET /api/products - Get all products
    app.get("/api/products", [&productService](const Request& req) {
        try {
            auto products = productService.getAllProducts();

            std::ostringstream json;
            json << "{\"products\":[";
            for (size_t i = 0; i < products.size(); ++i) {
                json << products[i].toJson();
                if (i < products.size() - 1) json << ",";
            }
            json << "],\"count\":" << products.size() << "}";

            return Response::json(200, json.str());
        } catch (const std::exception& e) {
            return Response::json(500, "{\"error\":\"" + std::string(e.what()) + "\"}");
        }
    });

    // GET /api/products/:id - Get product by ID
    app.get("/api/products/:id", [&productService](const Request& req) {
        try {
            int id = std::stoi(req.getParam("id"));
            auto product = productService.getProduct(id);

            if (product.has_value()) {
                return Response::json(200, product->toJson());
            } else {
                return Response::json(404, "{\"error\":\"Product not found\"}");
            }
        } catch (const std::exception& e) {
            return Response::json(400, "{\"error\":\"" + std::string(e.what()) + "\"}");
        }
    });

    // POST /api/products - Create new product
    app.post("/api/products", [&productService](const Request& req) {
        try {
            Product product = Product::fromJson(req.getBody());
            Product created = productService.createProduct(product);
            return Response::json(201, created.toJson());
        } catch (const std::exception& e) {
            return Response::json(400, "{\"error\":\"" + std::string(e.what()) + "\"}");
        }
    });

    // PUT /api/products/:id - Update product
    app.put("/api/products/:id", [&productService](const Request& req) {
        try {
            int id = std::stoi(req.getParam("id"));
            Product product = Product::fromJson(req.getBody());
            productService.updateProduct(id, product);
            return Response::json(200, "{\"message\":\"Product updated successfully\"}");
        } catch (const std::exception& e) {
            return Response::json(400, "{\"error\":\"" + std::string(e.what()) + "\"}");
        }
    });

    // DELETE /api/products/:id - Delete product
    app.del("/api/products/:id", [&productService](const Request& req) {
        try {
            int id = std::stoi(req.getParam("id"));
            productService.deleteProduct(id);
            return Response::json(200, "{\"message\":\"Product deleted successfully\"}");
        } catch (const std::exception& e) {
            return Response::json(400, "{\"error\":\"" + std::string(e.what()) + "\"}");
        }
    });

    // GET /api/products/category/:category - Get products by category
    app.get("/api/products/category/:category", [&productService](const Request& req) {
        try {
            std::string category = req.getParam("category");
            auto products = productService.getProductsByCategory(category);

            std::ostringstream json;
            json << "{\"products\":[";
            for (size_t i = 0; i < products.size(); ++i) {
                json << products[i].toJson();
                if (i < products.size() - 1) json << ",";
            }
            json << "],\"category\":\"" << category << "\",\"count\":" << products.size() << "}";

            return Response::json(200, json.str());
        } catch (const std::exception& e) {
            return Response::json(400, "{\"error\":\"" + std::string(e.what()) + "\"}");
        }
    });

    // GET /api/products/search/:keyword - Search products
    app.get("/api/products/search/:keyword", [&productService](const Request& req) {
        try {
            std::string keyword = req.getParam("keyword");
            auto products = productService.searchProducts(keyword);

            std::ostringstream json;
            json << "{\"products\":[";
            for (size_t i = 0; i < products.size(); ++i) {
                json << products[i].toJson();
                if (i < products.size() - 1) json << ",";
            }
            json << "],\"keyword\":\"" << keyword << "\",\"count\":" << products.size() << "}";

            return Response::json(200, json.str());
        } catch (const std::exception& e) {
            return Response::json(400, "{\"error\":\"" + std::string(e.what()) + "\"}");
        }
    });

    // GET /api/products/active - Get active products only
    app.get("/api/products/active", [&productService](const Request& req) {
        try {
            auto products = productService.getActiveProducts();

            std::ostringstream json;
            json << "{\"products\":[";
            for (size_t i = 0; i < products.size(); ++i) {
                json << products[i].toJson();
                if (i < products.size() - 1) json << ",";
            }
            json << "],\"count\":" << products.size() << "}";

            return Response::json(200, json.str());
        } catch (const std::exception& e) {
            return Response::json(500, "{\"error\":\"" + std::string(e.what()) + "\"}");
        }
    });

    // PUT /api/products/:id/stock - Update product stock
    app.put("/api/products/:id/stock", [&productService](const Request& req) {
        try {
            int id = std::stoi(req.getParam("id"));
            // Extract quantity from JSON body
            std::string body = req.getBody();
            size_t pos = body.find("\"quantity\":");
            if (pos == std::string::npos) {
                return Response::json(400, "{\"error\":\"Quantity not provided\"}");
            }
            pos += 11;
            int quantity = std::stoi(body.substr(pos));

            productService.updateStock(id, quantity);
            return Response::json(200, "{\"message\":\"Stock updated successfully\"}");
        } catch (const std::exception& e) {
            return Response::json(400, "{\"error\":\"" + std::string(e.what()) + "\"}");
        }
    });

    // ===== GENERAL ENDPOINTS =====

    // GET / - API Info
    app.get("/", [](const Request& req) {
        (void)req;
        return Response::json(200, R"({
            "name": "E-Commerce REST API",
            "version": "1.0.0",
            "description": "Full-featured e-commerce API with products, orders, and users",
            "endpoints": {
                "products": "/api/products",
                "orders": "/api/orders",
                "users": "/api/users"
            }
        })");
    });

    // GET /health - Health check
    app.get("/health", [](const Request& req) {
        (void)req;
        return Response::json(200, R"({
            "status": "healthy",
            "service": "e-commerce-api",
            "database": "connected"
        })");
    });

    // Print available endpoints
    std::cout << "\n📍 Available Endpoints:\n";
    std::cout << "  GET    /                              - API Info\n";
    std::cout << "  GET    /health                        - Health check\n";
    std::cout << "\n  Products:\n";
    std::cout << "  GET    /api/products                  - Get all products\n";
    std::cout << "  GET    /api/products/:id              - Get product by ID\n";
    std::cout << "  POST   /api/products                  - Create new product\n";
    std::cout << "  PUT    /api/products/:id              - Update product\n";
    std::cout << "  DELETE /api/products/:id              - Delete product\n";
    std::cout << "  GET    /api/products/category/:cat    - Get products by category\n";
    std::cout << "  GET    /api/products/search/:keyword  - Search products\n";
    std::cout << "  GET    /api/products/active           - Get active products\n";
    std::cout << "  PUT    /api/products/:id/stock        - Update stock\n";
    std::cout << "\n";
    std::cout << "💡 Examples:\n";
    std::cout << "  curl http://localhost:8080/api/products\n";
    std::cout << "  curl http://localhost:8080/api/products/1\n";
    std::cout << "  curl http://localhost:8080/api/products/category/electronics\n";
    std::cout << "  curl -X POST -d '{\"name\":\"Laptop\",\"price\":999.99}' http://localhost:8080/api/products\n";
    std::cout << "\n";

    // Start server
    app.start();

    return 0;
}
