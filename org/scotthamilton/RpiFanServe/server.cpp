#include <algorithm>
#include <map>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/sdbus.hpp>
#include <sdbusplus/sdbuspp_support/server.hpp>
#include <sdbusplus/server.hpp>
#include <string>
#include <tuple>

#include <org/scotthamilton/RpiFanServe/server.hpp>

#include <org/scotthamilton/RpiFanServe/error.hpp>


namespace sdbusplus
{
namespace org
{
namespace scotthamilton
{
namespace server
{

RpiFanServe::RpiFanServe(bus_t& bus, const char* path)
        : _org_scotthamilton_RpiFanServe_interface(
                bus, path, interface, _vtable, this), _intf(bus.getInterface())
{
}

RpiFanServe::RpiFanServe(bus_t& bus, const char* path,
                           const std::map<std::string, PropertiesVariant>& vals,
                           bool skipSignal)
        : RpiFanServe(bus, path)
{
    for (const auto& v : vals)
    {
        setPropertyByName(v.first, v.second, skipSignal);
    }
}



void RpiFanServe::cacheUpdated(
            )
{
    auto& i = _org_scotthamilton_RpiFanServe_interface;
    auto m = i.new_signal("CacheUpdated");

    m.append();
    m.signal_send();
}

namespace details
{
namespace RpiFanServe
{
static const auto _signal_CacheUpdated =
        utility::tuple_to_array(std::make_tuple('\0'));
}
}


auto RpiFanServe::cacheLifeExpectancy() const ->
        int64_t
{
    return _cacheLifeExpectancy;
}

int RpiFanServe::_callback_get_CacheLifeExpectancy(
        sd_bus* /*bus*/, const char* /*path*/, const char* /*interface*/,
        const char* /*property*/, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    auto o = static_cast<RpiFanServe*>(context);

    try
    {
        return sdbusplus::sdbuspp::property_callback(
                reply, o->_intf, error,
                std::function(
                    [=]()
                    {
                        return o->cacheLifeExpectancy();
                    }
                ));
    }
    catch(const sdbusplus::org::scotthamilton::RpiFanServe::Error::InvalidCacheLifeExpectancy& e)
    {
        return o->_intf->sd_bus_error_set(error, e.name(), e.description());
    }
}

auto RpiFanServe::cacheLifeExpectancy(int64_t value,
                                         bool skipSignal) ->
        int64_t
{
    if (_cacheLifeExpectancy != value)
    {
        _cacheLifeExpectancy = value;
        if (!skipSignal)
        {
            _org_scotthamilton_RpiFanServe_interface.property_changed("CacheLifeExpectancy");
        }
    }

    return _cacheLifeExpectancy;
}

auto RpiFanServe::cacheLifeExpectancy(int64_t val) ->
        int64_t
{
    return cacheLifeExpectancy(val, false);
}

int RpiFanServe::_callback_set_CacheLifeExpectancy(
        sd_bus* /*bus*/, const char* /*path*/, const char* /*interface*/,
        const char* /*property*/, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    auto o = static_cast<RpiFanServe*>(context);

    try
    {
        return sdbusplus::sdbuspp::property_callback(
                value, o->_intf, error,
                std::function(
                    [=](int64_t&& arg)
                    {
                        o->cacheLifeExpectancy(std::move(arg));
                    }
                ));
    }
    catch(const sdbusplus::org::scotthamilton::RpiFanServe::Error::InvalidCacheLifeExpectancy& e)
    {
        return o->_intf->sd_bus_error_set(error, e.name(), e.description());
    }

    return true;
}

namespace details
{
namespace RpiFanServe
{
static const auto _property_CacheLifeExpectancy =
    utility::tuple_to_array(message::types::type_id<
            int64_t>());
}
}

void RpiFanServe::setPropertyByName(const std::string& _name,
                                     const PropertiesVariant& val,
                                     bool skipSignal)
{
    if (_name == "CacheLifeExpectancy")
    {
        auto& v = std::get<int64_t>(val);
        cacheLifeExpectancy(v, skipSignal);
        return;
    }
}

auto RpiFanServe::getPropertyByName(const std::string& _name) ->
        PropertiesVariant
{
    if (_name == "CacheLifeExpectancy")
    {
        return cacheLifeExpectancy();
    }

    return PropertiesVariant();
}


const vtable_t RpiFanServe::_vtable[] = {
    vtable::start(),

    vtable::signal("CacheUpdated",
                   details::RpiFanServe::_signal_CacheUpdated
                        .data()),
    vtable::property("CacheLifeExpectancy",
                     details::RpiFanServe::_property_CacheLifeExpectancy
                        .data(),
                     _callback_get_CacheLifeExpectancy,
                     _callback_set_CacheLifeExpectancy,
                     vtable::property_::emits_change),
    vtable::end()
};

} // namespace server
} // namespace scotthamilton
} // namespace org
} // namespace sdbusplus

