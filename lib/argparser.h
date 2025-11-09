#pragma once

#include <cstddef>

namespace nargparse {

constexpr size_t kDefaultMaxArgLength=1024;

enum ParsingType {
  kNargsRequired,
  kNargsZeroOrMore,
  kNargsOptional,
  kNargsOneOrMore
};
struct ParsingResults{
  void* first_result;
  void** all_results;
  int count_of_all_results;
  int size_of_all_results;
};
struct Flag {
  const char *short_flag;
  const char *long_flag;
  bool *result;
  const char *comment;
  bool default_value;
};

struct ParseInfo {
  bool IsParsed;
  bool ValidationFuncResult;
};

enum class ArgType { kInteger, kFloat, kChar };

struct NamedArgument {
  ArgType arg_type;

  const char *short_arg;
  const char *long_arg;
  ParsingResults results;
  const char *name_of_arg;
  ParsingType type;

  union {
    bool (*int_valid_func)(const int &value);
    bool (*float_valid_func)(const float &value);
    bool (*void_valid_func)(const char *const &value);
  } valid_func;

  const char *valid_comment;
};

struct PosArgument {
  ArgType arg_type;

  ParsingResults results;
  const char *name_of_arg;
  ParsingType type;

  union {
    bool (*int_valid_func)(const int &value);
    bool (*float_valid_func)(const float &value);
    bool (*void_valid_func)(const char *const &value);
  } valid_func;

  const char *valid_comment;
};

struct ArgumentParser {
  const char *name_of_parser;
  bool help_added;

  size_t max_arg_length;

  size_t count_of_flags;
  size_t count_of_pos_args;
  size_t count_of_named_args;

  size_t size_of_flags_array;
  size_t size_of_pos_args_array;
  size_t size_of_named_args_array;

  Flag *flags;
  PosArgument *pos_args;
  NamedArgument *named_args;
};

ArgumentParser CreateParser(const char *name_of_parser,
                            const size_t max_arg_length=kDefaultMaxArgLength);
void FreeParser(ArgumentParser& parser);
bool Parse(ArgumentParser &parser, int argc, const char* const* argv);
void AddFlag(ArgumentParser &parser, const char *short_flag,
             const char *long_flag, bool *result, const char *name_of_flag,
             bool default_value = false);

void AddArgument(ArgumentParser &parser, const char *short_arg,
                 const char *long_arg, int *result, const char *name_of_arg,
                 ParsingType type = kNargsRequired,
                 bool (*valid_func)(const int &value) = nullptr,
                 const char *valid_comment = nullptr);

void AddArgument(ArgumentParser &parser, const char *short_arg,
                 const char *long_arg, float *result, const char *name_of_arg,
                 ParsingType type, bool (*valid_func)(const float &value)=nullptr,
                 const char *valid_comment = nullptr);

void AddArgument(ArgumentParser &parser, const char *short_arg,
                 const char *long_arg, void *result, const char *name_of_arg,
                 ParsingType type = kNargsRequired,
                 bool (*valid_func)(const char *const &value) = nullptr,
                 const char *valid_comment = nullptr);

void AddArgument(ArgumentParser &parser, void *result, const char *name_of_arg,
                 ParsingType type = kNargsRequired,
                 bool (*valid_func)(const char *const &value) = nullptr,
                 const char *valid_comment = nullptr);

void AddArgument(ArgumentParser &parser, float *result, const char *name_of_arg,
                 ParsingType type = kNargsRequired,
                 bool (*valid_func)(const float &value) = nullptr,
                 const char *valid_comment = nullptr);

void AddArgument(ArgumentParser &parser, int *result, const char *name_of_arg,
                 ParsingType type = kNargsRequired,
                 bool (*valid_func)(const int &value) = nullptr,
                 const char *valid_comment = nullptr);

int GetRepeatedCount(ArgumentParser &parser, const char *name_of_arg);

bool GetRepeated(ArgumentParser &parser, const char *name_of_arg,
                 int pos_of_arg, const char** result);
bool GetRepeated(ArgumentParser &parser, const char *name_of_arg,
                 int pos_of_arg, int *result);
bool GetRepeated(ArgumentParser &parser, const char *name_of_arg,
                 int pos_of_arg, float *result);

void AddHelp(ArgumentParser &parser);
void PrintHelp(ArgumentParser &parser);

} // namespace nargparse