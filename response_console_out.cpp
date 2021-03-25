#include "response.h"

#include <iomanip>

using namespace std;


void Response::Print(ostream& os) const {
    os << error_message;
}

std::ostream& operator<<(std::ostream& os, Response* response) {
    response->Print(os);
    return os;
}

void BusResponse::Print (ostream& os) const {
    os << "Bus " << number << ": "
       << stops_on_route << " stops on route, "
       << unique_stops << " unique stops, "
       << real_length << " route length, "
       << setprecision(6)
       << real_length*1.0/route_length << " curvature";
       //<< route_length << " geo_length";
}

void StopResponse::Print (ostream& os) const {
    os << "Stop " << stop_name << ": ";
    if (!buses.empty()) {
        os << "buses";
        for (auto bus : buses) {
            os << " " << bus;
        }
    } else {
        os << "no buses";
    }
}

void PrintResponses(const vector<ResponseHolder>& responses, ostream& out) {
    for (auto& response : responses) {
        out << setprecision(6) << response << endl;
    }
}