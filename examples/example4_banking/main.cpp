#include "../../framework/include/restapi.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <ctime>

using namespace RestAPI;

// ===== DATA STRUCTURES =====
struct Account {
    std::string account_id;
    std::string owner;
    double balance;
    std::string type; // "checking", "savings"
    std::string currency;
};

struct Transaction {
    std::string transaction_id;
    std::string from_account;
    std::string to_account;
    double amount;
    long timestamp;
    std::string status; // "completed", "pending", "failed"
};

// In-memory storage
std::map<std::string, Account> accounts;
std::vector<Transaction> transactions;
int next_transaction_id = 1;

// ===== HELPER FUNCTIONS =====
std::string generateTransactionId() {
    return "TXN" + std::to_string(next_transaction_id++);
}

int main() {
    RestApiFramework app(8083, 2);
    app.enable_cors(true);

    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║       EXAMPLE 4: BANKING API                   ║\n";
    std::cout << "║       Account Management & Transfers           ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    std::cout << "\n";

    // Initialize sample accounts
    accounts["ACC001"] = {"ACC001", "Ion Popescu", 5000.00, "checking", "RON"};
    accounts["ACC002"] = {"ACC002", "Maria Ionescu", 10000.00, "savings", "RON"};
    accounts["ACC003"] = {"ACC003", "Vasile Georgescu", 7500.50, "checking", "RON"};
    accounts["ACC004"] = {"ACC004", "Elena Marinescu", 15000.00, "savings", "EUR"};

    // ===== ENDPOINT 1: Get account balance =====
    app.get("/api/accounts/:id/balance", [](const Request& req) {
        std::string id = req.getParam("id");

        auto it = accounts.find(id);
        if (it == accounts.end()) {
            return Response::json(404, R"({"error": "Account not found"})");
        }

        const Account& acc = it->second;
        std::ostringstream oss;
        oss << R"({)"
            << R"("account_id": ")" << acc.account_id << R"(",)"
            << R"("owner": ")" << acc.owner << R"(",)"
            << R"("balance": )" << acc.balance << ","
            << R"("currency": ")" << acc.currency << R"(",)"
            << R"("type": ")" << acc.type << R"(")"
            << R"(})";

        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 2: Get account details =====
    app.get("/api/accounts/:id", [](const Request& req) {
        std::string id = req.getParam("id");

        auto it = accounts.find(id);
        if (it == accounts.end()) {
            return Response::json(404, R"({"error": "Account not found"})");
        }

        const Account& acc = it->second;
        std::ostringstream oss;
        oss << R"({)"
            << R"("account_id": ")" << acc.account_id << R"(",)"
            << R"("owner": ")" << acc.owner << R"(",)"
            << R"("balance": )" << acc.balance << ","
            << R"("type": ")" << acc.type << R"(",)"
            << R"("currency": ")" << acc.currency << R"(")"
            << R"(})";

        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 3: List all accounts =====
    app.get("/api/accounts", [](const Request& req) {
        (void)req;

        std::ostringstream oss;
        oss << R"({"accounts": [)";

        bool first = true;
        for (const auto& [id, acc] : accounts) {
            if (!first) oss << ",";
            oss << R"({"account_id": ")" << acc.account_id << R"(",)"
                << R"("owner": ")" << acc.owner << R"(",)"
                << R"("balance": )" << acc.balance << ","
                << R"("type": ")" << acc.type << R"("})";
            first = false;
        }

        oss << R"(], "count": )" << accounts.size() << "}";
        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 4: Deposit money =====
    app.post("/api/accounts/:id/deposit", [](const Request& req) {
        std::string id = req.getParam("id");

        auto it = accounts.find(id);
        if (it == accounts.end()) {
            return Response::json(404, R"({"error": "Account not found"})");
        }

        // In real app, parse amount from req.body
        double amount = 100.00; // Demo amount

        it->second.balance += amount;

        std::ostringstream oss;
        oss << R"({)"
            << R"("status": "success",)"
            << R"("message": "Deposit completed",)"
            << R"("account_id": ")" << id << R"(",)"
            << R"("amount": )" << amount << ","
            << R"("new_balance": )" << it->second.balance
            << R"(})";

        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 5: Withdraw money =====
    app.post("/api/accounts/:id/withdraw", [](const Request& req) {
        std::string id = req.getParam("id");

        auto it = accounts.find(id);
        if (it == accounts.end()) {
            return Response::json(404, R"({"error": "Account not found"})");
        }

        // In real app, parse amount from req.body
        double amount = 50.00; // Demo amount

        if (it->second.balance < amount) {
            return Response::json(400, R"({"error": "Insufficient funds"})");
        }

        it->second.balance -= amount;

        std::ostringstream oss;
        oss << R"({)"
            << R"("status": "success",)"
            << R"("message": "Withdrawal completed",)"
            << R"("account_id": ")" << id << R"(",)"
            << R"("amount": )" << amount << ","
            << R"("new_balance": )" << it->second.balance
            << R"(})";

        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 6: Transfer money =====
    app.post("/api/transfer", [](const Request& req) {
        // In real app, parse from/to/amount from req.body
        std::string from = "ACC001";
        std::string to = "ACC002";
        double amount = 200.00;

        auto from_it = accounts.find(from);
        auto to_it = accounts.find(to);

        if (from_it == accounts.end() || to_it == accounts.end()) {
            return Response::json(404, R"({"error": "Account not found"})");
        }

        if (from_it->second.balance < amount) {
            return Response::json(400, R"({"error": "Insufficient funds"})");
        }

        // Perform transfer
        from_it->second.balance -= amount;
        to_it->second.balance += amount;

        // Record transaction
        Transaction txn;
        txn.transaction_id = generateTransactionId();
        txn.from_account = from;
        txn.to_account = to;
        txn.amount = amount;
        txn.timestamp = std::time(nullptr);
        txn.status = "completed";
        transactions.push_back(txn);

        std::ostringstream oss;
        oss << R"({)"
            << R"("status": "success",)"
            << R"("message": "Transfer completed",)"
            << R"("transaction_id": ")" << txn.transaction_id << R"(",)"
            << R"("from_account": ")" << from << R"(",)"
            << R"("to_account": ")" << to << R"(",)"
            << R"("amount": )" << amount << ","
            << R"("timestamp": )" << txn.timestamp
            << R"(})";

        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 7: Get transaction history =====
    app.get("/api/transactions", [](const Request& req) {
        (void)req;

        std::ostringstream oss;
        oss << R"({"transactions": [)";

        for (size_t i = 0; i < transactions.size(); ++i) {
            if (i > 0) oss << ",";
            const auto& txn = transactions[i];
            oss << R"({"transaction_id": ")" << txn.transaction_id << R"(",)"
                << R"("from": ")" << txn.from_account << R"(",)"
                << R"("to": ")" << txn.to_account << R"(",)"
                << R"("amount": )" << txn.amount << ","
                << R"("status": ")" << txn.status << R"(",)"
                << R"("timestamp": )" << txn.timestamp << "}";
        }

        oss << R"(], "count": )" << transactions.size() << "}";
        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 8: Get account transactions =====
    app.get("/api/accounts/:id/transactions", [](const Request& req) {
        std::string id = req.getParam("id");

        std::ostringstream oss;
        oss << R"({"account_id": ")" << id << R"(", "transactions": [)";

        bool first = true;
        for (const auto& txn : transactions) {
            if (txn.from_account == id || txn.to_account == id) {
                if (!first) oss << ",";
                oss << R"({"transaction_id": ")" << txn.transaction_id << R"(",)"
                    << R"("type": ")" << (txn.from_account == id ? "debit" : "credit") << R"(",)"
                    << R"("amount": )" << txn.amount << ","
                    << R"("timestamp": )" << txn.timestamp << "}";
                first = false;
            }
        }

        oss << "]}";
        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 9: Health check =====
    app.get("/health", [](const Request& req) {
        (void)req;
        return Response::json(200, R"({
            "status": "healthy",
            "domain": "Banking",
            "accounts_count": )" + std::to_string(accounts.size()) + R"(,
            "transactions_count": )" + std::to_string(transactions.size()) + R"(
        })");
    });

    // Print available endpoints
    std::cout << "\n📍 Available Endpoints:\n";
    std::cout << "  GET  /api/accounts                    - List all accounts\n";
    std::cout << "  GET  /api/accounts/:id                - Get account details\n";
    std::cout << "  GET  /api/accounts/:id/balance        - Get account balance\n";
    std::cout << "  POST /api/accounts/:id/deposit        - Deposit money\n";
    std::cout << "  POST /api/accounts/:id/withdraw       - Withdraw money\n";
    std::cout << "  POST /api/transfer                    - Transfer money\n";
    std::cout << "  GET  /api/transactions                - All transactions\n";
    std::cout << "  GET  /api/accounts/:id/transactions   - Account transactions\n";
    std::cout << "  GET  /health                          - Health check\n";
    std::cout << "\n";
    std::cout << "💡 Examples:\n";
    std::cout << "  curl http://localhost:8083/api/accounts\n";
    std::cout << "  curl http://localhost:8083/api/accounts/ACC001/balance\n";
    std::cout << "  curl -X POST http://localhost:8083/api/transfer\n";
    std::cout << "  curl http://localhost:8083/api/transactions\n";
    std::cout << "\n";

    app.start();
    return 0;
}
