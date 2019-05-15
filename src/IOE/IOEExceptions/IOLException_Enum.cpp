#include "IOLException_Enum.h"

namespace IOL {
namespace Exceptions {

enum EExceptionID : std::int16_t;
{
    // Generic errors
    E_ERROR_UNKNOWN=0x000,

    // Conversion / IO errors
    E_ERROR_CONVERSION_UNKNOWN=0x1000,
    E_ERROR_NO_TYPE_IN_STRING,
    E_ERROR_CONVERSION_MAX,

    // Type errors
    E_ERROR_TYPE_UNKNOWN=0x2000,
    E_ERROR_TYPE_MAX,

    // Network Errors
    E_ERROR_NET_UNKNOWN=0x3000,
    E_ERROR_NET_MAX,

    E_ERROR_MAX

};

} // namespace Exceptions
} // namespace IOL