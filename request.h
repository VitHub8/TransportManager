#pragma once

#include "transport_manager.h"
#include "response.h"
#include "json.h"

#include <memory>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <optional>
#include <variant>

struct Request;
using RequestHolder = std::unique_ptr<Request>;

struct Request {
    enum class Type {
        READ_BUS,
        STOP,
        BUS,
        READ_STOP,
        ROUTE,
        SETTING
    };

    explicit Request(Type type) : type(type) {}
    static RequestHolder Create(Type type);
    virtual ~Request() = default;
    virtual void ParseFrom(std::istream& in) = 0;
    virtual void ParseFrom(const Json::Node&) = 0;
    const Type type;
};


struct ReadRequest : public Request {
    using Request::Request;
    virtual ResponseHolder Process(const TransportManager& manager) const = 0;

    int id;
};


struct ReadBusRequest : ReadRequest {
    ReadBusRequest() : ReadRequest(Request::Type::READ_BUS) {}

    ResponseHolder Process(const TransportManager& manager) const override;
    void ParseFrom(std::istream& in) override;
    void ParseFrom(const Json::Node&) override;

    std::string number;
};


struct ReadStopRequest : ReadRequest {
    ReadStopRequest() : ReadRequest(Request::Type::READ_STOP) {}

    ResponseHolder Process(const TransportManager& manager) const override;
    void ParseFrom(std::istream& in) override;
    void ParseFrom(const Json::Node&) override;
    std::string stop_name;

};


struct RouteRequest : ReadRequest {
    RouteRequest() : ReadRequest(Request::Type::ROUTE) {}

    ResponseHolder Process(const TransportManager& manager) const override;
    void ParseFrom(std::istream& in) override {}
    void ParseFrom(const Json::Node& node) override;
    std::string from;
    std::string to;
};


struct ModifyRequest : public Request {
    using Request::Request;
    virtual void Process(TransportManager& manager) const = 0;
};


struct UpdateBusRequest : public ModifyRequest {
    UpdateBusRequest() : ModifyRequest(Request::Type::BUS) {}

    void Process(TransportManager& manager) const override;
    void ParseFrom(std::istream& in) override;
    void ParseFrom(const Json::Node&) override;

private:
    static std::pair<std::vector<std::string>, std::string_view> ReadContent(std::string_view input);
    static std::vector<std::string> ReadStops(const Json::Node& node);

    BusHolder content_;
};


struct UpdateStopRequest : public ModifyRequest {
    UpdateStopRequest() : ModifyRequest(Request::Type::STOP) {}

    void Process(TransportManager& manager) const override;
    void ParseFrom(std::istream& in) override;
    void ParseFrom(const Json::Node&) override;

private:
    static std::unordered_map<std::string, int> ParseDistances(std::string_view input);
    static std::unordered_map<std::string, int> ParseDistMap(const Json::Node& node);

    StopHolder content_;
};


struct SettingRequest : public ModifyRequest {
    SettingRequest() : ModifyRequest(Request::Type::SETTING) {}

    void Process(TransportManager& manager) const override;
    void ParseFrom(std::istream& in) override {}
    void ParseFrom(const Json::Node& node) override;

private:
    int wait_time = 0;
    double bus_velocity = 0;
};



const std::unordered_map<std::string_view, Request::Type> UPDATE_TYPES_FROM_STR = {
        {"Stop", Request::Type::STOP},
        {"Bus", Request::Type::BUS},
        {"routing_settings", Request::Type::SETTING}
};

const std::unordered_map<std::string_view, Request::Type> READ_TYPES_FROM_STR = {
        {"Bus", Request::Type::READ_BUS},
        {"Stop", Request::Type::READ_STOP},
        {"Route", Request::Type::ROUTE}
};

enum class UPD_OR_READ {
    READ,
    UPD
};

std::optional<Request::Type> ConvertRequestTypeFromString(std::string_view type_str, UPD_OR_READ uor);

RequestHolder ParseRequest(UPD_OR_READ uor, std::istream& in);

std::deque<RequestHolder> ReadRequests(UPD_OR_READ uor, std::istream& in);

std::vector<ResponseHolder> ProcessRequests(TransportManager& manager,
                                                        const std::deque<RequestHolder>& requests);

namespace Json {
    RequestHolder ParseRequest(UPD_OR_READ uor, const Node& request_node);
    std::deque<RequestHolder> ReadRequests(const Document& doc);

}