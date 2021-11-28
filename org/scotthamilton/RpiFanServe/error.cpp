#include <org/scotthamilton/RpiFanServe/error.hpp>

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
const char* InvalidCacheLifeExpectancy::name() const noexcept
{
    return errName;
}
const char* InvalidCacheLifeExpectancy::description() const noexcept
{
    return errDesc;
}
const char* InvalidCacheLifeExpectancy::what() const noexcept
{
    return errWhat;
}

} // namespace Error
} // namespace RpiFanServe
} // namespace scotthamilton
} // namespace org
} // namespace sdbusplus

