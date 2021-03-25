#include "transport_manager.h"

using namespace std;


void TransportManager::AddBus(const BusHolder& bus) {
    all_buses[bus->number] = bus;
}

void TransportManager::AddStop(const StopHolder& stop) {
    auto stop_ = GetStop(stop->name);
    stop_->name = stop->name;
    stop_->coords = stop->coords;
    stop_->dist_to_stops = move(stop->dist_to_stops);
    vertexId_to_stop.push_back(stop_->name);
    vertexId_to_stop.push_back(stop_->name);
    stops_to_vertex[stop_->name] = pair<Graph::VertexId, Graph::VertexId>(vertexId_to_stop.size()-2,
                                                                          vertexId_to_stop.size()-1);
    wait_edges[stop_->name] = Graph::Edge<double>{vertexId_to_stop.size()-2,
                                                    vertexId_to_stop.size()-1,
                                                    double(wait_time)};

}

StopHolder& TransportManager::GetStop(string_view stop) {
    if (all_stops.find(stop) == all_stops.end()) {
        all_stops[stop] = make_shared<Stop>();
    }
    return all_stops[stop];
}

const BusHolder& TransportManager::ReadBus(string_view bus) const {
    if (all_buses.find(bus) == all_buses.end()) {
        throw invalid_argument("not found");
    }
    return all_buses.at(bus);
}

const StopHolder& TransportManager::ReadStop(string_view stop) const {
    if (all_stops.find(stop) == all_stops.end()) {
        throw invalid_argument("not found");
    }
    return all_stops.at(stop);
}

double TransportManager::GetRouteDistance(string_view bus) const {
    const auto& bus_ = ReadBus(bus);
    if (bus_->stops.empty()) {
        return 0;
    }
    Distance dist{ReadStop(bus_->stops.front())->coords, 0.0};
    for (auto& stop : bus_->stops) {
        dist+=ReadStop(stop)->coords;
    }
    return dist.distance;
}

int TransportManager::GetRealDistance(string_view bus) const {
    const auto& bus_ = ReadBus(bus);
    if (bus_->stops.empty()) {
        return 0;
    }
    if (bus_->stops.size() == 1) {
        return GetRealDistTwoStops(bus_->stops.front(), bus_->stops.front());
    }
    int result = 0;
    for (auto it = bus_->stops.begin(); it < prev(bus_->stops.end()); ++it) {
        result += GetRealDistTwoStops(*it, *next(it));
    }
    return result;
}

int TransportManager::GetRealDistTwoStops(string_view lhs, string_view rhs) const {
    auto& l_stop = ReadStop(lhs);
    auto& r_stop = ReadStop(rhs);
    if (l_stop->dist_to_stops.find(string(rhs)) != l_stop->dist_to_stops.end()) {
        return l_stop->dist_to_stops.at(string(rhs));
    }
    if (r_stop->dist_to_stops.find(string(lhs)) != r_stop->dist_to_stops.end()) {
        return r_stop->dist_to_stops.at(string(lhs));
    }
    //return FindMinDistanceTwoPoints(l_stop->coords, r_stop->coords);
}

vector<pair<int, Graph::Edge<double>>> TransportManager::MakeBusEdges(const BusHolder& bh) const {
    vector<pair<int, Graph::Edge<double>>> result;
    result.reserve(bh->stops.size()*(bh->stops.size()-1)/2);
    for (size_t i = 0; i < bh->stops.size()-1; ++i) {
        double edge_weight = 0;
        int span_count = 0;
        for (size_t j = i+1; j < bh->stops.size(); ++j) {
            edge_weight += GetRealDistTwoStops(bh->stops[j-1], bh->stops[j])/1000.0/bus_velocity*60.0;

            ++span_count;
            result.emplace_back(span_count, Graph::Edge<double>{stops_to_vertex.at(bh->stops[i]).second,
                                                                stops_to_vertex.at(bh->stops[j]).first,
                                                                edge_weight}
            );
        }
    }
    return result;
}


void TransportManager::BuildGraph() const {
    if (!graph_) {
        graph_ = Graph::DirectedWeightedGraph<double>(vertexId_to_stop.size());
        for (auto& [bus, bh] : all_buses) {
            edges[bus] = MakeBusEdges(bh);
        }
        for (auto& [name, pair_span_edges] :  edges) {
            for (auto &[span_count, edge] : pair_span_edges) {
                auto edge_id = graph_->AddEdge(edge);
                edgeId_to_info[edge_id] = EdgeInfo{false, name, span_count};
            }
        }

        for (auto& [name, edge] : wait_edges) {
            auto edge_id = graph_->AddEdge(edge);
            edgeId_to_info[edge_id] = EdgeInfo{true, name, 0};
        }
        router = make_shared<Graph::Router<double>>(*graph_);
    }
}