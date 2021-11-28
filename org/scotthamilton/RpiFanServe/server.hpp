#pragma once
#include <limits>
#include <map>
#include <optional>
#include <sdbusplus/sdbus.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/utility/dedup_variant.hpp>
#include <string>
#include <systemd/sd-bus.h>
#include <tuple>



#ifndef SDBUSPP_NEW_CAMELCASE
#define SDBUSPP_NEW_CAMELCASE 1
#endif

namespace sdbusplus
{
namespace org
{
namespace scotthamilton
{
namespace server
{

class RpiFanServe
{
    public:
        /* Define all of the basic class operations:
         *     Not allowed:
         *         - Default constructor to avoid nullptrs.
         *         - Copy operations due to internal unique_ptr.
         *         - Move operations due to 'this' being registered as the
         *           'context' with sdbus.
         *     Allowed:
         *         - Destructor.
         */
        RpiFanServe() = delete;
        RpiFanServe(const RpiFanServe&) = delete;
        RpiFanServe& operator=(const RpiFanServe&) = delete;
        RpiFanServe(RpiFanServe&&) = delete;
        RpiFanServe& operator=(RpiFanServe&&) = delete;
        virtual ~RpiFanServe() = default;

        /** @brief Constructor to put object onto bus at a dbus path.
         *  @param[in] bus - Bus to attach to.
         *  @param[in] path - Path to attach at.
         */
        RpiFanServe(bus_t& bus, const char* path);


        using PropertiesVariant = sdbusplus::utility::dedup_variant_t<
                int64_t>;

        /** @brief Constructor to initialize the object from a map of
         *         properties.
         *
         *  @param[in] bus - Bus to attach to.
         *  @param[in] path - Path to attach at.
         *  @param[in] vals - Map of property name to value for initialization.
         */
        RpiFanServe(bus_t& bus, const char* path,
                     const std::map<std::string, PropertiesVariant>& vals,
                     bool skipSignal = false);



        /** @brief Send signal 'CacheUpdated'
         *
         *  Signal indicating the cache was updated.
         */
        void cacheUpdated(
            );

        /** Get value of CacheLifeExpectancy */
        virtual int64_t cacheLifeExpectancy() const;
        /** Set value of CacheLifeExpectancy with option to skip sending signal */
        virtual int64_t cacheLifeExpectancy(int64_t value,
               bool skipSignal);
        /** Set value of CacheLifeExpectancy */
        virtual int64_t cacheLifeExpectancy(int64_t value);

        /** @brief Sets a property by name.
         *  @param[in] _name - A string representation of the property name.
         *  @param[in] val - A variant containing the value to set.
         */
        void setPropertyByName(const std::string& _name,
                               const PropertiesVariant& val,
                               bool skipSignal = false);

        /** @brief Gets a property by name.
         *  @param[in] _name - A string representation of the property name.
         *  @return - A variant containing the value of the property.
         */
        PropertiesVariant getPropertyByName(const std::string& _name);


        /** @brief Emit interface added */
        void emit_added()
        {
            _org_scotthamilton_RpiFanServe_interface.emit_added();
        }

        /** @brief Emit interface removed */
        void emit_removed()
        {
            _org_scotthamilton_RpiFanServe_interface.emit_removed();
        }

        static constexpr auto interface = "org.scotthamilton.RpiFanServe";

    private:

        /** @brief sd-bus callback for get-property 'CacheLifeExpectancy' */
        static int _callback_get_CacheLifeExpectancy(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'CacheLifeExpectancy' */
        static int _callback_set_CacheLifeExpectancy(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);


        static const vtable_t _vtable[];
        sdbusplus::server::interface_t
                _org_scotthamilton_RpiFanServe_interface;
        sdbusplus::SdBusInterface *_intf;

        int64_t _cacheLifeExpectancy = 0;

};


} // namespace server
} // namespace scotthamilton
} // namespace org

namespace message::details
{
} // namespace message::details
} // namespace sdbusplus

