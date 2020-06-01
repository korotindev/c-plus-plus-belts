//
// Created by Дмитрий Коротин on 01.06.2020.
//

#ifndef C_PLUS_PLUS_BELTS_REQUEST_H
#define C_PLUS_PLUS_BELTS_REQUEST_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "Coordinate.h"

struct Request;
using RequestHolder = std::unique_ptr<Request>;

struct Request {
    enum class Type {
        EntertainStop,
        EntertainBus,
        ReadBus,
    };

    Request(Type type) : type(type) {}

    static RequestHolder Create(Type type);
    virtual void ParseFrom(std::string_view input) = 0;
    virtual ~Request() = default;

    const Type type;
};


using TypeConverter = std::unordered_map<std::string_view, Request::Type>;
const TypeConverter MODIFY_TYPES_CONVERTER = {
        {"Bus",  Request::Type::EntertainBus},
        {"Stop", Request::Type::EntertainStop},
};
const TypeConverter READ_TYPES_CONVERTER = {
        {"Bus", Request::Type::ReadBus},
};

std::optional<Request::Type> ConvertRequestTypeFromString(const TypeConverter &converter, std::string_view type_str);

RequestHolder ParseRequest(const TypeConverter &converter, std::string_view request_str);


struct ReadRequest : Request {
    using Request::Request;
    virtual void Process() const = 0;
};

struct ModifyRequest : Request {
    using Request::Request;
    virtual void Process() const = 0;
};

struct EntertainStopRequest : ModifyRequest {
    EntertainStopRequest() : ModifyRequest(Type::EntertainStop) {}

    void ParseFrom(std::string_view input) override;
    void Process() const override;

    std::string StopName;
    double Latitude;
    double Longitude;
};

struct EntertainBusRequest : ModifyRequest {
    EntertainBusRequest() : ModifyRequest(Type::EntertainBus) {}

    void ParseFrom(std::string_view input) override;
    void Process() const override;

    std::string BusName;
    std::vector<std::string> StopsNames;
};

struct ReadBusRequest : ReadRequest {
    ReadBusRequest() : ReadRequest(Type::ReadBus) {}

    void ParseFrom(std::string_view input) override;
    void Process() const override;

    std::string BusName;
};


#endif //C_PLUS_PLUS_BELTS_REQUEST_H
