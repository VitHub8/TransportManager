#pragma once

#include "json.h"

#include <memory>
#include <iostream>
#include <string>
#include <set>
#include <string_view>


struct Response {
    explicit Response(int id) : id(id) {}
    explicit Response(std::string error) : id(0), error_message(std::move(error)) {}

    virtual void Print(std::ostream& os) const;
    virtual Json::Node BuildNode() const;

    int id;
    std::string error_message = "not found";
};

using ResponseHolder = std::shared_ptr<Response>;


std::ostream& operator<<(std::ostream& os, Response* response);


struct BusResponse : public Response {

    BusResponse() : Response(0) {}
    explicit BusResponse(int id) : Response(id) {}

    Json::Node BuildNode() const override;
    void Print (std::ostream& os) const override;

    std::string_view number;
    int stops_on_route = 0;
    int unique_stops = 0;
    double route_length = 0;
    int real_length = 0;

};


struct StopResponse : public Response {

    StopResponse(std::string_view stop_name, const std::set<std::string_view>& buses)
            : Response(0),
              stop_name(stop_name),
              buses(buses) {}

    StopResponse(int id, std::string_view stop_name, const std::set<std::string_view>& buses)
            : Response(id),
              stop_name(stop_name),
              buses(buses) {}

    Json::Node BuildNode() const override;
    void Print (std::ostream& os) const override;

    std::string_view stop_name;
    const std::set<std::string_view>& buses;
};



struct RouteResponse : public Response {

    struct Element {
        enum class Type {
            WAIT,
            BUS
        };

        virtual Json::Node BuildNode() const =0;
        Element(Type type_) : type(type_) {}

        std::string GetType() const {
            return (type == Type::BUS) ? "Bus" : "Wait";
        }

        Type type;
    };

    using ElementHolder = std::shared_ptr<Element>;


    struct Wait : public Element {
        Wait() : Element(Type::WAIT) {}
        Json::Node BuildNode() const override {
            std::map<std::string, Json::Node> result = {
                    {"type", Json::Node(GetType())},
                    {"stop_name", Json::Node(stop_name)},
                    {"time", Json::Node(wait_time)}
            };
            return Json::Node(move(result));
        }

        int wait_time = 0;
        std::string stop_name;
    };

    struct Bus : public Element {
        Bus() : Element(Type::BUS) {}
        Json::Node BuildNode() const override {
            std::map<std::string, Json::Node> result = {
                    {"type", Json::Node(GetType())},
                    {"bus", Json::Node(bus_name)},
                    {"time", Json::Node(time)},
                    {"span_count", Json::Node(span_count)}
            };
            return Json::Node(move(result));
        }

        std::string bus_name;
        int span_count = 0;
        double time = 0;
    };

    RouteResponse(int id, double total_time_, std::vector<ElementHolder> elems)
            : Response(id),
              total_time(total_time_),
              elements(elems) {}


    void Print(std::ostream& os) const override {}
    Json::Node BuildNode() const override;

    double total_time = 0;
    std::vector<ElementHolder> elements;


};

void PrintResponses(const std::vector<ResponseHolder>& responses,
                    std::ostream& out);

Json::Document ConvertToJson(const std::vector<ResponseHolder>& responses);
