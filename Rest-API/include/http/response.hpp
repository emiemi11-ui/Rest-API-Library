#pragma once
#include <string>

class HttpResponse {
public:
    // Construieste un raspuns HTTP/1.1 cu Content-Type: application/json
    // Returneaza raspunsul ca std::string (gata de trimis pe socket).
    static std::string json(int status, const std::string& body);
};
