#include <cstddef>
namespace nargparse {
enum ParsingType {
  kNargsZeroOrMore,
  kNargsRequired,
  kNargsOptional,
  kNargsOneOrMore
};

struct Flag {
  const char *short_flag;
  const char *long_flag;
  bool *result;
  const char *comment;
  bool default_value;
};

struct Argument {
  ArgType arg_type;

  const char *short_arg;
  const char *long_arg;
  void *result;
  const char *name_of_arg;
  ParsingType type;

  union {
    bool (*default_valid_func)();
    bool (*int_valid_func)(const int &value);
    bool (*float_valid_func)(const float &value);
    bool (*char_valid_func)(const char *const &value);
  } valid_func;

  const char *valid_comment;
};

enum class ArgType { kInteger, kFloat, kChar };

struct ArgumentParser {
  const char *name_of_parser;
  size_t max_arg_length;

  size_t count_of_flags;
  size_t count_of_args;
  size_t size_of_flags_array;
  size_t size_of_args_array;

  Flag *flags;
  Argument *args;
};

ArgumentParser CreateParser(const char *name_of_parser,
                            const size_t max_arg_length);
void FreeParser(ArgumentParser parser);
bool Parse(ArgumentParser &parser, int argc, const char **argv);
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
                 ParsingType type, bool (*valid_func)(const float &value),
                 const char *valid_comment = nullptr);

void AddArgument(ArgumentParser &parser, const char *short_arg,
                 const char *long_arg, void *result, const char *name_of_arg,
                 ParsingType type = kNargsRequired,
                 bool (*valid_func)(const char *const &value) = nullptr,
                 const char *valid_comment = nullptr);

void AddArgument(ArgumentParser &parser, void *result, const char *name_of_arg,
                 ParsingType type = kNargsRequired,
                 bool (*valid_func)(const char *const &value) = nullptr);

void AddArgument(ArgumentParser &parser, int *result, const char *name_of_arg,
                 ParsingType type = kNargsRequired,
                 bool (*valid_func)(const int &value) = nullptr,
                 const char *valid_comment = nullptr);

int GetRepeatedCount(ArgumentParser &parser, const char *name_of_arg);

bool GetRepeated(ArgumentParser &parser, const char *name_of_arg,
                 int pos_of_arg, void *result);
bool GetRepeated(ArgumentParser &parser, const char *name_of_arg,
                 int pos_of_arg, int *result);
bool GetRepeated(ArgumentParser &parser, const char *name_of_arg,
                 int pos_of_arg, float *result);

void AddHelp(ArgumentParser &parser);
} // namespace nargparse