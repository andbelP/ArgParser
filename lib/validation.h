#pragma once

#include "argparser.h"

namespace nargparse {

bool ValidateArgumentSize(ArgumentParser &parser, const char *arg);

bool ValidateArgsAfterParsing(ArgumentParser &parser);

bool IsValidInteger(const char *arg);

bool IsValidFloat(const char *arg);

} // namespace nargparse
