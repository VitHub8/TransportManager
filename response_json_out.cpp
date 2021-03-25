#include "response.h"

#include <cmath>

using namespace std;

Json::Node Response::BuildNode() const {
    map<string, Json::Node> result = {
            {"request_id", Json::Node(id)},
            {"error_message", Json::Node(error_message)}
    };
    return result;
}

Json::Node BusResponse::BuildNode() const {
    map<string, Json::Node> result = {
            {"route_length", Json::Node(real_length)},
            {"request_id", Json::Node(id)},
            {"stop_count", Json::Node(static_cast<int>(stops_on_route))},
            {"unique_stop_count", Json::Node(static_cast<int>(unique_stops))},
    };
    if (route_length > 0) {
        result.emplace("curvature", Json::Node(double(real_length*1.0/route_length)));
    } else {
        result.emplace("curvature", Json::Node(-1.0));
    }
    return result;
}

Json::Node StopResponse::BuildNode() const {
    vector<Json::Node> buses_temp = {};
    buses_temp.reserve(buses.size());
    for (auto& bus : buses) {
        buses_temp.emplace_back(string(bus));
    }
    map<string, Json::Node> result = {
            {"buses", Json::Node(move(buses_temp))},
            {"request_id", Json::Node(id)}
    };
    return result;
}

Json::Node RouteResponse::BuildNode() const {
    vector<Json::Node> elements_;
    elements_.reserve(elements.size());
    for (auto& elem : elements) {
        elements_.push_back(elem->BuildNode());
    }
    map<string, Json::Node> result = {
            {"items", Json::Node(move(elements_))},
            {"request_id", Json::Node(id)},
            {"total_time", total_time}
    };
    return Json::Node(move(result));
}

Json::Document ConvertToJson(const std::vector<ResponseHolder>& responses) {
    vector<Json::Node> nodes;
    nodes.reserve(responses.size());
    for (auto& holder : responses) {
        nodes.push_back(holder->BuildNode());
    }
    return Json::Document(move(nodes));
}
