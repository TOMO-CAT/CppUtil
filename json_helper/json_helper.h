#pragma once

#include "json/json.h"
#include "json_helper/unmarshal.h"

namespace json_helper {

#define JSON_HELPER(...) JSON_HELPER_UNMARSHAL_MEMBER_FUNCTION(__VA_ARGS__)

}  // namespace json_helper