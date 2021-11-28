#pragma once

#include <cerrno>
#include <sdbusplus/exception.hpp>

namespace sdbusplus
{
namespace org
{
namespace scotthamilton
{
namespace RpiFanServe
{
namespace Error
{

struct InvalidCacheLifeExpectancy final :
        public sdbusplus::exception::generated_exception
{
    static constexpr auto errName = "org.scotthamilton.RpiFanServe.Error.InvalidCacheLifeExpectancy";
    static constexpr auto errDesc =
            "An invalid cache life expectancy argument was provided.";
    static constexpr auto errWhat =
            "org.scotthamilton.RpiFanServe.Error.InvalidCacheLifeExpectancy: An invalid cache life expectancy argument was provided.";

    const char* name() const noexcept override;
    const char* description() const noexcept override;
    const char* what() const noexcept override;
};

} // namespace Error
} // namespace RpiFanServe
} // namespace scotthamilton
} // namespace org
} // namespace sdbusplus

