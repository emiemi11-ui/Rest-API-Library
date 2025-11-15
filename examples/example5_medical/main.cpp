#include "../../framework/include/restapi.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <ctime>

using namespace RestAPI;

// ===== DATA STRUCTURES =====
struct Patient {
    std::string patient_id;
    std::string name;
    int age;
    std::string blood_type;
    std::string phone;
    std::vector<std::string> medical_history;
};

struct Appointment {
    std::string appointment_id;
    std::string patient_id;
    std::string doctor;
    std::string date;
    std::string time;
    std::string status; // "scheduled", "completed", "cancelled"
};

struct MedicalRecord {
    std::string record_id;
    std::string patient_id;
    std::string date;
    std::string diagnosis;
    std::string treatment;
    std::string doctor;
};

// In-memory storage
std::map<std::string, Patient> patients;
std::vector<Appointment> appointments;
std::vector<MedicalRecord> records;
int next_appointment_id = 1;
int next_record_id = 1;

// ===== HELPER FUNCTIONS =====
std::string generateAppointmentId() {
    return "APT" + std::to_string(next_appointment_id++);
}

std::string generateRecordId() {
    return "REC" + std::to_string(next_record_id++);
}

int main() {
    RestApiFramework app(8084, 2);
    app.enable_cors(true);

    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║     EXAMPLE 5: MEDICAL RECORDS API             ║\n";
    std::cout << "║     Patient Management & Appointments          ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    std::cout << "\n";

    // Initialize sample patients
    patients["P001"] = {"P001", "Ion Popescu", 45, "A+", "0721234567",
                        {"Diabet tip 2", "Hipertensiune"}};
    patients["P002"] = {"P002", "Maria Ionescu", 32, "O-", "0732345678",
                        {"Alergii sezoniere"}};
    patients["P003"] = {"P003", "Vasile Georgescu", 58, "B+", "0743456789",
                        {"Hipertensiune", "Colesterol"}};
    patients["P004"] = {"P004", "Elena Marinescu", 28, "AB+", "0754567890",
                        {}};

    // ===== ENDPOINT 1: Get patient details =====
    app.get("/api/patients/:id", [](const Request& req) {
        std::string id = req.getParam("id");

        auto it = patients.find(id);
        if (it == patients.end()) {
            return Response::json(404, R"({"error": "Patient not found"})");
        }

        const Patient& p = it->second;
        std::ostringstream oss;
        oss << R"({)"
            << R"("patient_id": ")" << p.patient_id << R"(",)"
            << R"("name": ")" << p.name << R"(",)"
            << R"("age": )" << p.age << ","
            << R"("blood_type": ")" << p.blood_type << R"(",)"
            << R"("phone": ")" << p.phone << R"(")"
            << R"(})";

        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 2: List all patients =====
    app.get("/api/patients", [](const Request& req) {
        (void)req;

        std::ostringstream oss;
        oss << R"({"patients": [)";

        bool first = true;
        for (const auto& [id, p] : patients) {
            if (!first) oss << ",";
            oss << R"({"patient_id": ")" << p.patient_id << R"(",)"
                << R"("name": ")" << p.name << R"(",)"
                << R"("age": )" << p.age << ","
                << R"("blood_type": ")" << p.blood_type << R"("})";
            first = false;
        }

        oss << R"(], "count": )" << patients.size() << "}";
        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 3: Get patient medical history =====
    app.get("/api/patients/:id/history", [](const Request& req) {
        std::string id = req.getParam("id");

        auto it = patients.find(id);
        if (it == patients.end()) {
            return Response::json(404, R"({"error": "Patient not found"})");
        }

        const Patient& p = it->second;
        std::ostringstream oss;
        oss << R"({"patient_id": ")" << id << R"(", "medical_history": [)";

        for (size_t i = 0; i < p.medical_history.size(); ++i) {
            if (i > 0) oss << ",";
            oss << R"(")" << p.medical_history[i] << R"(")";
        }

        oss << R"(], "count": )" << p.medical_history.size() << "}";
        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 4: Create appointment =====
    app.post("/api/appointments", [](const Request& req) {
        // In real app, parse patient_id, doctor, date, time from req.body
        Appointment apt;
        apt.appointment_id = generateAppointmentId();
        apt.patient_id = "P001";
        apt.doctor = "Dr. Popescu";
        apt.date = "2024-11-20";
        apt.time = "10:00";
        apt.status = "scheduled";

        appointments.push_back(apt);

        std::ostringstream oss;
        oss << R"({)"
            << R"("status": "success",)"
            << R"("message": "Appointment created",)"
            << R"("appointment_id": ")" << apt.appointment_id << R"(",)"
            << R"("patient_id": ")" << apt.patient_id << R"(",)"
            << R"("doctor": ")" << apt.doctor << R"(",)"
            << R"("date": ")" << apt.date << R"(",)"
            << R"("time": ")" << apt.time << R"(")"
            << R"(})";

        return Response::json(201, oss.str());
    });

    // ===== ENDPOINT 5: Get all appointments =====
    app.get("/api/appointments", [](const Request& req) {
        (void)req;

        std::ostringstream oss;
        oss << R"({"appointments": [)";

        for (size_t i = 0; i < appointments.size(); ++i) {
            if (i > 0) oss << ",";
            const auto& apt = appointments[i];
            oss << R"({"appointment_id": ")" << apt.appointment_id << R"(",)"
                << R"("patient_id": ")" << apt.patient_id << R"(",)"
                << R"("doctor": ")" << apt.doctor << R"(",)"
                << R"("date": ")" << apt.date << R"(",)"
                << R"("status": ")" << apt.status << R"("})";
        }

        oss << R"(], "count": )" << appointments.size() << "}";
        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 6: Get appointments for a patient =====
    app.get("/api/patients/:id/appointments", [](const Request& req) {
        std::string id = req.getParam("id");

        std::ostringstream oss;
        oss << R"({"patient_id": ")" << id << R"(", "appointments": [)";

        bool first = true;
        for (const auto& apt : appointments) {
            if (apt.patient_id == id) {
                if (!first) oss << ",";
                oss << R"({"appointment_id": ")" << apt.appointment_id << R"(",)"
                    << R"("doctor": ")" << apt.doctor << R"(",)"
                    << R"("date": ")" << apt.date << R"(",)"
                    << R"("time": ")" << apt.time << R"(",)"
                    << R"("status": ")" << apt.status << R"("})";
                first = false;
            }
        }

        oss << "]}";
        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 7: Add medical record =====
    app.post("/api/records", [](const Request& req) {
        // In real app, parse from req.body
        MedicalRecord rec;
        rec.record_id = generateRecordId();
        rec.patient_id = "P001";
        rec.date = "2024-11-15";
        rec.diagnosis = "Grip sezonieră";
        rec.treatment = "Paracetamol 500mg";
        rec.doctor = "Dr. Ionescu";

        records.push_back(rec);

        std::ostringstream oss;
        oss << R"({)"
            << R"("status": "success",)"
            << R"("message": "Medical record added",)"
            << R"("record_id": ")" << rec.record_id << R"(",)"
            << R"("patient_id": ")" << rec.patient_id << R"(")"
            << R"(})";

        return Response::json(201, oss.str());
    });

    // ===== ENDPOINT 8: Get medical records for patient =====
    app.get("/api/patients/:id/records", [](const Request& req) {
        std::string id = req.getParam("id");

        std::ostringstream oss;
        oss << R"({"patient_id": ")" << id << R"(", "records": [)";

        bool first = true;
        for (const auto& rec : records) {
            if (rec.patient_id == id) {
                if (!first) oss << ",";
                oss << R"({"record_id": ")" << rec.record_id << R"(",)"
                    << R"("date": ")" << rec.date << R"(",)"
                    << R"("diagnosis": ")" << rec.diagnosis << R"(",)"
                    << R"("doctor": ")" << rec.doctor << R"("})";
                first = false;
            }
        }

        oss << "]}";
        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 9: Search patients by name =====
    app.get("/api/patients/search/:name", [](const Request& req) {
        std::string name = req.getParam("name");

        std::ostringstream oss;
        oss << R"({"search_term": ")" << name << R"(", "results": [)";

        bool first = true;
        for (const auto& [id, p] : patients) {
            if (p.name.find(name) != std::string::npos) {
                if (!first) oss << ",";
                oss << R"({"patient_id": ")" << p.patient_id << R"(",)"
                    << R"("name": ")" << p.name << R"(",)"
                    << R"("age": )" << p.age << "}";
                first = false;
            }
        }

        oss << "]}";
        return Response::json(200, oss.str());
    });

    // ===== ENDPOINT 10: Health check =====
    app.get("/health", [](const Request& req) {
        (void)req;
        return Response::json(200, R"({
            "status": "healthy",
            "domain": "Medical Records",
            "patients_count": )" + std::to_string(patients.size()) + R"(,
            "appointments_count": )" + std::to_string(appointments.size()) + R"(
        })");
    });

    // Print available endpoints
    std::cout << "\n📍 Available Endpoints:\n";
    std::cout << "  GET  /api/patients                   - List all patients\n";
    std::cout << "  GET  /api/patients/:id               - Get patient details\n";
    std::cout << "  GET  /api/patients/:id/history       - Patient medical history\n";
    std::cout << "  GET  /api/patients/:id/appointments  - Patient appointments\n";
    std::cout << "  GET  /api/patients/:id/records       - Patient medical records\n";
    std::cout << "  GET  /api/patients/search/:name      - Search patients\n";
    std::cout << "  POST /api/appointments               - Create appointment\n";
    std::cout << "  GET  /api/appointments               - List appointments\n";
    std::cout << "  POST /api/records                    - Add medical record\n";
    std::cout << "  GET  /health                         - Health check\n";
    std::cout << "\n";
    std::cout << "💡 Examples:\n";
    std::cout << "  curl http://localhost:8084/api/patients\n";
    std::cout << "  curl http://localhost:8084/api/patients/P001\n";
    std::cout << "  curl http://localhost:8084/api/patients/P001/history\n";
    std::cout << "  curl -X POST http://localhost:8084/api/appointments\n";
    std::cout << "\n";

    app.start();
    return 0;
}
