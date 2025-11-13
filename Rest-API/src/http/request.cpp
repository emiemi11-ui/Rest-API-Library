// Exemplu generic – adaptează la numele funcției/fisierului tău:
#include "http/request.hpp"
#include <sstream>

static void fill_target_and_path(HttpRequest& req, const std::string& uri) {
    req.target = uri; // EX: "/api/users/add?name=Ana"
    auto qpos = uri.find('?');
    if (qpos == std::string::npos) req.path = uri;
    else req.path = uri.substr(0, qpos);
}

/*
   Unde ai parsarea primei linii:
   prima linie e ceva de genul: "GET /api/users/add?name=Ana HTTP/1.1"
*/
bool parse_request_line(const std::string& line, HttpRequest& req) {
    // Atenție: dacă ai DEJA o funcție existentă care sparge linia,
    // nu o șterge; doar adaugă fill_target_and_path(req, uri);
    std::istringstream iss(line);
    std::string method, uri, version;
    if (!(iss >> method >> uri >> version)) return false;
    req.method = method;
    fill_target_and_path(req, uri);
    return true;
}
