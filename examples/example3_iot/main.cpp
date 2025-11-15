#include "../../framework/include/restapi.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <algorithm>

using namespace RestAPI;

// ===== DATA STRUCTURES =====
struct SensorReading {
    std::string sensor_id;
    double temperature;
    double humidity;
    long timestamp;
    std::string location;
};

// In-memory storage
std::vector<SensorReading> readings;

// ===== HELPER FUNCTIONS =====
std::string getCurrentTimestamp() {
    return std::to_string(std::time(nullptr));
}

double calculateAverage(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    double sum = 0.0;
    for (double v : values) sum += v;
    return sum / values.size();
}

int main() {
    RestApiFramework app(8082, 2);
    app.enable_cors(true);

    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║      EXAMPLE 3: IOT SENSORS API                ║\n";
    std::cout << "║      Temperature & Humidity Monitoring         ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    std::cout << "\n";

    // Add some sample data
    readings.push_back({"SENS001", 22.5, 55.2, std::time(nullptr) - 3600, "Living Room"});
    readings.push_back({"SENS002", 24.1, 60.5, std::time(nullptr) - 3600, "Bedroom"});
    readings.push_back({"SENS003", 21.8, 52.8, std::time(nullptr) - 3600, "Kitchen"});

    // ===== ENDPOINT 1: Submit sensor data =====
    app.post("/api/sensors/data", [](const Request& req) {
        // In a real app, parse JSON from req.body
        // For demo, we'll create a sample reading
        SensorReading reading;
        reading.sensor_id = "SENS00" + std::to_string(readings.size() + 1);
        reading.temperature = 23.0 + (rand() % 50) / 10.0;
        reading.humidity = 50.0 + (rand() % 200) / 10.0;
        reading.timestamp = std::time(nullptr);
        reading.location = "Demo Location";

        readings.push_back(reading);

        std::ostringstream oss;
        oss << R"({)"
            << R"("status": "success",)"
            << R"("message": "Sensor data received",)"
            << R"("sensor_id": ")" << reading.sensor_id << R"(",)"
            << R"("reading_id": )" << readings.size()
            << R"(})";

        return Response::json(201, oss.str());
    });

    // ===== ENDPOINT 2: Get latest reading for a sensor =====
    app.get("/api/sensors/:id/latest", [](const Request& req) {
        std::string sensor_id = req.getParam("id");

        // Find latest reading for this sensor
        for (auto it = readings.rbegin(); it != readings.rend(); ++it) {
            if (it->sensor_id == sensor_id) {
                std::ostringstream oss;
                oss << R"({)"
                    << R"("sensor_id": ")" << it->sensor_id << R"(",)"
                    << R"("temperature": )" << it->temperature << ","
                    << R"("humidity": )" << it->humidity << ","
                    << R"("timestamp": )" << it->timestamp << ","
                    << R"("location": ")" << it->location << R"(")"
                    << R"(})";
                return Response::json(200, oss.str());
            }
        }

        return Response::json(404, R"({"error": "No data for this sensor"})");
    });

    // ===== ENDPOINT 3: Get all readings for a sensor =====
    app.get("/api/sensors/:id/history", [](const Request& req) {
        std::string sensor_id = req.getParam("id");
        std::ostringstream oss;
        oss << R"({"sensor_id": ")" << sensor_id << R"(", "readings": [)";

        bool first = true;
        for (const auto& reading : readings) {
            if (reading.sensor_id == sensor_id) {
                if (!first) oss << ",";
                oss << R"({"temperature": )" << reading.temperature
                    << R"(, "humidity": )" << reading.humidity
                    << R"(, "timestamp": )" << reading.timestamp << "}";
                first = false;
            }
        }

        oss << "]}";
        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 4: Get statistics for all sensors =====
    app.get("/api/sensors/stats", [](const Request& req) {
        (void)req;

        if (readings.empty()) {
            return Response::json(200, R"({
                "total_readings": 0,
                "average_temperature": 0,
                "average_humidity": 0
            })");
        }

        std::vector<double> temps;
        std::vector<double> humidities;
        for (const auto& r : readings) {
            temps.push_back(r.temperature);
            humidities.push_back(r.humidity);
        }

        double avg_temp = calculateAverage(temps);
        double avg_humidity = calculateAverage(humidities);

        std::ostringstream oss;
        oss << R"({)"
            << R"("total_readings": )" << readings.size() << ","
            << R"("average_temperature": )" << avg_temp << ","
            << R"("average_humidity": )" << avg_humidity << ","
            << R"("min_temperature": )" << *std::min_element(temps.begin(), temps.end()) << ","
            << R"("max_temperature": )" << *std::max_element(temps.begin(), temps.end())
            << R"(})";

        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 5: List all sensors =====
    app.get("/api/sensors", [](const Request& req) {
        (void)req;

        // Get unique sensor IDs
        std::vector<std::string> sensor_ids;
        for (const auto& r : readings) {
            if (std::find(sensor_ids.begin(), sensor_ids.end(), r.sensor_id) == sensor_ids.end()) {
                sensor_ids.push_back(r.sensor_id);
            }
        }

        std::ostringstream oss;
        oss << R"({"sensors": [)";
        for (size_t i = 0; i < sensor_ids.size(); ++i) {
            if (i > 0) oss << ",";
            oss << R"(")" << sensor_ids[i] << R"(")";
        }
        oss << R"(], "count": )" << sensor_ids.size() << "}";

        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 6: Get alerts (high temperature) =====
    app.get("/api/sensors/alerts", [](const Request& req) {
        (void)req;

        const double TEMP_THRESHOLD = 25.0;

        std::ostringstream oss;
        oss << R"({"alerts": [)";

        bool first = true;
        for (const auto& reading : readings) {
            if (reading.temperature > TEMP_THRESHOLD) {
                if (!first) oss << ",";
                oss << R"({"sensor_id": ")" << reading.sensor_id << R"(",)"
                    << R"("temperature": )" << reading.temperature << ","
                    << R"("location": ")" << reading.location << R"(",)"
                    << R"("message": "Temperature above threshold"})";
                first = false;
            }
        }

        oss << "]}";
        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 7: Health check =====
    app.get("/health", [](const Request& req) {
        (void)req;
        return Response::json(200, R"({
            "status": "healthy",
            "domain": "IoT Sensors",
            "sensors_count": )" + std::to_string(readings.size()) + R"(
        })");
    });

    // Print available endpoints
    std::cout << "\n📍 Available Endpoints:\n";
    std::cout << "  POST /api/sensors/data           - Submit sensor reading\n";
    std::cout << "  GET  /api/sensors                - List all sensors\n";
    std::cout << "  GET  /api/sensors/:id/latest     - Latest reading for sensor\n";
    std::cout << "  GET  /api/sensors/:id/history    - All readings for sensor\n";
    std::cout << "  GET  /api/sensors/stats          - Statistics (all sensors)\n";
    std::cout << "  GET  /api/sensors/alerts         - High temperature alerts\n";
    std::cout << "  GET  /health                     - Health check\n";
    std::cout << "\n";
    std::cout << "💡 Examples:\n";
    std::cout << "  curl http://localhost:8082/api/sensors\n";
    std::cout << "  curl http://localhost:8082/api/sensors/SENS001/latest\n";
    std::cout << "  curl http://localhost:8082/api/sensors/stats\n";
    std::cout << "  curl -X POST http://localhost:8082/api/sensors/data\n";
    std::cout << "\n";

    app.start();
    return 0;
}
