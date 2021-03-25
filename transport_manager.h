#pragma once

#include <iostream>

#include "locator.h"
#include "router.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <deque>
#include <utility>

struct Stop {
    std::string name;
    Point coords;
    std::set<std::string_view> buses;
    std::unordered_map<std::string, int> dist_to_stops;

    void AddBus(std::string_view bus) {
        buses.insert(bus);
    }
};
using StopHolder = std::shared_ptr<Stop>;


struct Bus {
    enum class Type {
        LINE,
        CYCLE
    };

    std::string number;
    Type type;
    std::vector<std::string> stops = {};
    std::unordered_set<std::string_view> unique_stops = {};
};
using BusHolder = std::shared_ptr<Bus>;


class TransportManager {
public:
    struct EdgeInfo {
        bool is_stop_not_bus = true;
        std::string_view name;
        int span_count = 0;
    };

public:
    void AddBus(const BusHolder& bus);
    void AddStop(const StopHolder& stop);
    StopHolder& GetStop(std::string_view stop);
    const BusHolder& ReadBus(std::string_view bus) const;
    const StopHolder& ReadStop(std::string_view stop) const;
    double GetRouteDistance(std::string_view bus) const;
    int GetRealDistance(std::string_view bus) const;

    void SetWaitTime(int time) { wait_time = time; }
    void SetVelocity(double velocity) { bus_velocity = velocity; }

    int GetWaitTime() const { return wait_time; }
    double GetBusVelocity() const { return bus_velocity; }

    auto& GetInfoByEdgeId(Graph::EdgeId edge_id) const {
        return edgeId_to_info[edge_id];
    }

    std::string_view GetStopByVertexId(Graph::VertexId vertex_id) const {
        return vertexId_to_stop[vertex_id];
    }

    auto& GetGraph() const { return *graph_; }
    auto& GetRouter() const { return *router; }

    auto FindBestRoute(std::string_view start, std::string_view end) const {
        BuildGraph();
        if (last_route) {
            router->ReleaseRoute(*last_route);
        }
        auto result = router->BuildRoute(stops_to_vertex.at(start).first, stops_to_vertex.at(end).first);
        last_route = result->id;
        return result;
    }

private:
    int GetRealDistTwoStops(std::string_view lhs, std::string_view rhs) const;

    std::vector<std::pair<int, Graph::Edge<double>>> MakeBusEdges(const BusHolder& bh) const;
    void BuildGraph() const;

    int wait_time = 0;
    double bus_velocity = 0;

    std::unordered_map<std::string_view, BusHolder> all_buses;
    std::unordered_map<std::string_view, StopHolder> all_stops;

    std::unordered_map<std::string_view, std::pair<Graph::VertexId, Graph::VertexId>> stops_to_vertex;
    std::deque<std::string_view> vertexId_to_stop;
    mutable std::unordered_map<Graph::EdgeId, EdgeInfo> edgeId_to_info;
    mutable std::unordered_map<std::string_view, std::vector<std::pair<int, Graph::Edge<double>>>> edges;
    mutable std::unordered_map<std::string_view, Graph::Edge<double>> wait_edges;

    mutable std::optional<Graph::DirectedWeightedGraph<double>> graph_;
    mutable std::shared_ptr<Graph::Router<double>> router;

    mutable std::optional<Graph::Router<double>::RouteId> last_route;
};
