#include "request.h"
#include "console_parser.h"

using namespace std;


RequestHolder Request::Create(Request::Type type) {
    switch (type) {
        case Request::Type::STOP:
            return make_unique<UpdateStopRequest>();
        case Request::Type::BUS:
            return make_unique<UpdateBusRequest>();
        case Request::Type::READ_BUS:
            return make_unique<ReadBusRequest>();
        case Request::Type::READ_STOP:
            return make_unique<ReadStopRequest>();
        case Request::Type::ROUTE:
            return make_unique<RouteRequest>();
        default:
            return nullptr;
    }
}

ResponseHolder ReadBusRequest::Process(const TransportManager& manager) const {
    try {
        auto result = make_shared<BusResponse>();
        result->number = number;
        auto& bus = manager.ReadBus(number);
        result->unique_stops = bus->unique_stops.size();
        result->real_length = manager.GetRealDistance(number);
        result->route_length = manager.GetRouteDistance(number);
        result->stops_on_route = bus->stops.size();
        return result;
    } catch (exception& ex) {
        return make_shared<Response>(ex.what());
    }
}

void ReadBusRequest::ParseFrom(istream& in) {
    number = ReadToken(in,'\n');
}

void ReadBusRequest::ParseFrom(const Json::Node& node) {
    number = node.AsMap().at("name").AsString();
    id = node.AsMap().at("id").AsInt();
}


ResponseHolder ReadStopRequest::Process(const TransportManager& manager) const {
    try {
        auto result = make_shared<StopResponse>(stop_name, manager.ReadStop(stop_name)->buses);
        return result;
    } catch (exception& ex) {
        return make_shared<Response>(ex.what());
    }
}

void ReadStopRequest::ParseFrom(istream& in) {
    stop_name = ReadToken(in,'\n');
}

void ReadStopRequest::ParseFrom(const Json::Node& node) {
    stop_name = node.AsMap().at("name").AsString();
    id = node.AsMap().at("id").AsInt();
}


ResponseHolder RouteRequest::Process(const TransportManager& manager) const {
//    cerr << "Hello boys" << endl;
    auto route = manager.FindBestRoute(from, to);

    if (!route) {
        return make_shared<Response>("not found");
    }
    using Holder = RouteResponse::ElementHolder;
    vector<Holder> result;
    auto& graph = manager.GetGraph();
    auto& router = manager.GetRouter();
    for (size_t edge_idx = 0; edge_idx < route->edge_count; ++edge_idx) {
        Graph::EdgeId edgeId = router.GetRouteEdge(route->id, edge_idx);
        auto edge_info = manager.GetInfoByEdgeId(edgeId);
        auto edge = graph.GetEdge(edgeId);
        if (edge_info.is_stop_not_bus) {
            auto holder = make_shared<RouteResponse::Wait>();
            holder->wait_time = edge.weight;
            holder->stop_name = string(edge_info.name);
            result.push_back(holder);
        } else {
            auto holder = make_shared<RouteResponse::Bus>();
            holder->span_count = edge_info.span_count;
            holder->bus_name = string(edge_info.name);
            holder->time = edge.weight;
            result.push_back(holder);
        }
    }
    return make_shared<RouteResponse>(id, route->weight, result);
}

void RouteRequest::ParseFrom(const Json::Node& node) {
    id = node.AsMap().at("id").AsInt();
    from = node.AsMap().at("from").AsString();
    to = node.AsMap().at("to").AsString();
}


void UpdateBusRequest::Process(TransportManager& manager) const {
    manager.AddBus(content_);
    for (auto stop : content_->unique_stops) {
        manager.GetStop(stop)->AddBus(content_->number);
    }
}

void UpdateBusRequest::ParseFrom(istream& in) {
    string name = ReadToken(in, ':');
    in.ignore();
    string content;
    getline(in, content);
    auto bus = make_shared<Bus>();
    bus->number = move(name);
    auto [stops, route_type] = ReadContent(content);
    if (route_type == " - ") {
        bus->type = Bus::Type::LINE;
        bus->stops.reserve(stops.size()*2-1);
        bus->stops = move(stops);
        for (auto it = next(bus->stops.rbegin()); it < bus->stops.rend(); ++it) {
            bus->stops.push_back(*it);
        }
    } else if (route_type == " > ") {
        bus->type = Bus::Type::CYCLE;
        bus->stops.reserve(stops.size());
        bus->stops = move(stops);
    }
    bus->unique_stops = unordered_set<string_view>(bus->stops.begin(), bus->stops.end());
    content_ = bus;
}

void UpdateBusRequest::ParseFrom(const Json::Node& node) {
    auto bus = make_shared<Bus>();
    const auto& node_map = node.AsMap();
    auto stops = ReadStops(node_map.at("stops"));

    bus->type = node_map.at("is_roundtrip").AsBool()
                ? Bus::Type::CYCLE
                : Bus::Type::LINE;

    if (bus->type == Bus::Type::LINE) {
        bus->stops = move(stops);
        bus->stops.reserve(bus->stops.size()*2-1);
        for (auto it = next(bus->stops.rbegin()); it < bus->stops.rend(); ++it) {
            bus->stops.push_back(*it);
        }
    } else if (bus->type == Bus::Type::CYCLE) {
        bus->stops.reserve(stops.size());
        bus->stops = move(stops);
    }
    bus->unique_stops = unordered_set<string_view>(bus->stops.begin(), bus->stops.end());
    bus->number = node_map.at("name").AsString();
    content_ = bus;
}


pair<vector<string>, string_view> UpdateBusRequest::ReadContent(string_view input) {
    vector<string> result;
    string_view route_type = "";
    if (input.find('>') != string_view::npos) {
        route_type = " > ";
    } else if (input.find('-') != string_view::npos) {
        route_type = " - ";
    }
    while(!input.empty()) {
        result.emplace_back(ReadToken(input, route_type));
    }
    return {result, route_type};
}

vector<string> UpdateBusRequest::ReadStops(const Json::Node& node) {
    vector<string> result = {};
    const auto& stops_vector = node.AsArray();
    result.reserve(stops_vector.size());

    for (const Json::Node& stop : stops_vector) {
        result.push_back(stop.AsString());
    }
    return result;
}


void UpdateStopRequest::Process(TransportManager& manager) const {
    manager.AddStop(content_);
}

void UpdateStopRequest::ParseFrom(istream& in) {
    string name = ReadToken(in, ':');
    double lat, longit;
    string dists_in_line;
    in >> lat;
    in.ignore();
    in>>longit;
    if (in.peek() == ','){
        in.ignore();
        in.ignore();
        getline(in, dists_in_line, '\n');
    }
    content_ = make_shared<Stop>(Stop{name,
                                      Point{.latitude =  lat, .longitude = longit}, {}, {}});
    content_->dist_to_stops = ParseDistances(dists_in_line);
}

void UpdateStopRequest::ParseFrom(const Json::Node& node) {
    const auto& node_map = node.AsMap();
    double lat = node_map.at("latitude").AsDouble();
    double longit = node_map.at("longitude").AsDouble();

    content_ = make_shared<Stop>(Stop{node_map.at("name").AsString(),
                                      Point{.latitude =  lat, .longitude = longit}, {}, {}});
    content_->dist_to_stops = ParseDistMap(node_map.at("road_distances"));
}


unordered_map<string, int> UpdateStopRequest::ParseDistances(string_view input) {
    if (input.empty()) {
        return {};
    }
    unordered_map<string, int> result;
    string_view delimiter = ", ";
    while(!input.empty()) {
        auto dist_to_stop = ReadToken(input, delimiter);
        int dist = stoi(string(ReadToken(dist_to_stop, "m to ")));
        result[string(dist_to_stop)] = dist;
    }
    return result;
}

unordered_map<string, int> UpdateStopRequest::ParseDistMap(const Json::Node& node) {
    unordered_map<string, int> result = {};
    for (const auto& [stop, dist] : node.AsMap()) {
        result[stop] = dist.AsInt();
    }
    return result;
}


void SettingRequest::Process(TransportManager& manager) const {
    manager.SetWaitTime(wait_time);
    manager.SetVelocity(bus_velocity);
}

void SettingRequest::ParseFrom(const Json::Node& node) {
    wait_time = node.AsMap().at("bus_wait_time").AsInt();
    bus_velocity = node.AsMap().at("bus_velocity").AsDouble();
}


optional<Request::Type> ConvertRequestTypeFromString(string_view type_str, UPD_OR_READ uor) {
    if (uor == UPD_OR_READ::READ) {
        if (const auto it = READ_TYPES_FROM_STR.find(type_str);
                it != READ_TYPES_FROM_STR.end()) {
            return it->second;
        }
    } else {
        if (const auto it = UPDATE_TYPES_FROM_STR.find(type_str);
                it != UPDATE_TYPES_FROM_STR.end()) {
            return it->second;
        }
    }
    return nullopt;
}


RequestHolder ParseRequest(UPD_OR_READ uor, istream& in) {
    string type;
    in>>type;
    const auto request_type = ConvertRequestTypeFromString(type, uor);
    if (!request_type) {
        return nullptr;
    }
    RequestHolder request = Request::Create(*request_type);
    if (request) {
        request->ParseFrom(in);
    }
    return request;
}


deque<RequestHolder> ReadRequests(UPD_OR_READ uor, istream& in) {
    size_t request_count;
    in >> request_count;

    deque<RequestHolder> requests;
    //requests.reserve(request_count);

    for (size_t i = 0; i < request_count; ++i) {
        if (auto request = ParseRequest(uor, in)) {
            requests.push_back(move(request));
        }
    }
    return requests;
}


vector<ResponseHolder> ProcessRequests(TransportManager& manager,
                                       const deque<RequestHolder>& requests) {
    vector<ResponseHolder> responses;
    for (const auto& request_holder : requests) {
        if (request_holder->type == Request::Type::READ_BUS ||
            request_holder->type == Request::Type::READ_STOP ||
            request_holder->type == Request::Type::ROUTE) {
            const auto& request = dynamic_cast<const ReadRequest&>(*request_holder);
            responses.emplace_back(request.Process(manager));
            responses.back()->id = request.id;
        } else {
            const auto& request = dynamic_cast<const ModifyRequest&>(*request_holder);
            request.Process(manager);
        }
    }
    return responses;
}


namespace Json {

    RequestHolder ParseRequest(UPD_OR_READ uor, const Node& request_node) {
        const auto request_type = ConvertRequestTypeFromString(request_node.AsMap().at("type").AsString(), uor);
        if (!request_type) {
            return nullptr;
        }
        RequestHolder request = Request::Create(*request_type);
        if (request) {
            request->ParseFrom(request_node);
        }
        return request;
    }

    deque<RequestHolder> ReadRequests(const Document& doc) {
        deque<RequestHolder> requests;
        const auto& root_map = doc.GetRoot().AsMap();
        if (root_map.find("routing_settings") != root_map.end()) {
            RequestHolder set_request = make_unique<SettingRequest>();
            set_request->ParseFrom(root_map.at("routing_settings"));
            requests.push_back(move(set_request));
        }
        const auto& base_requests = root_map.at("base_requests").AsArray();
        const auto& stat_requests = root_map.at("stat_requests").AsArray();

        for (const auto& base_req : base_requests) {
            requests.push_back(Json::ParseRequest(UPD_OR_READ::UPD, base_req));
        }
        for (const auto& base_req : stat_requests) {
            requests.push_back(Json::ParseRequest(UPD_OR_READ::READ, base_req));
        }
        return requests;
    }
}
