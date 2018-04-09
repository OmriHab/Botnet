#ifndef BOTNET_DEFINES_H
#define BOTNET_DEFINES_H

#include <string>

namespace botnet_defines {

static const std::string BOT_FILE_UPLOAD_DIR = "Botloads";

// Both auth must be the same length
static const std::string SERVER_AUTH = "666";
static const std::string BOT_AUTH    = "999";

static const int AUTH_LEN = 3;


}

#endif
