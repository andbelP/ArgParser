#include "argparser.h"

namespace nargparse{

const char* ExtractNamedArgWithEqualsValue(const char* arg);


NamedArgument* FindNamedArgumentByName(ArgumentParser& parser, const char* name_of_arg);

PosArgument* FindPosArgumentByName(ArgumentParser& parser, const char* name_of_arg);

Flag* FindFlag(ArgumentParser& parser, const char* arg);

NamedArgument* FindSplittedNamedArgument(ArgumentParser& parser, const char* arg);

NamedArgument* FindNamedArgumentWithEquals(ArgumentParser& parser, const char* arg);

PosArgument* FindPosArg(ArgumentParser& parser, const char* arg);

ParseInfo ParseFlag(ArgumentParser& parser, const char* const* argv, int i);

ParseInfo ParseSplittedNamedArgument(ArgumentParser& parser, int argc, const char* const* argv, int& i);

ParseInfo ParseNamedArgumentWithEquals(ArgumentParser& parser, const char* argv);

ParseInfo ParsePosArgument(ArgumentParser& parser, const char* argv);

}