#include <iostream>
#include <cstring>

#include "validation.h"
#include "memory_management.h"
#include "parsing.h"

#include "argparser.h"

namespace nargparse {

ArgumentParser CreateParser(const char* name_of_parser, const size_t max_arg_length) {
	ArgumentParser parser;
	parser.help_added=false;

	parser.name_of_parser = name_of_parser;
	parser.max_arg_length = max_arg_length;

	parser.count_of_pos_args = 0;
	parser.count_of_flags = 0;
	parser.count_of_named_args = 0;

	parser.size_of_pos_args_array = 1;
	parser.size_of_flags_array = 1;
	parser.size_of_named_args_array = 1;

	parser.flags = new Flag[1];
	parser.pos_args = new PosArgument[1];
	parser.named_args = new NamedArgument[1];

	return parser;
}

void FreeParser(ArgumentParser& parser) {
	for(int i = 0; i < parser.count_of_named_args; i++){
		for(int j  = 0; j < parser.named_args[i].results.count_of_all_results; j++){
			switch(parser.named_args[i].arg_type){
				case(ArgType::kChar): {
					delete[] static_cast<char*>(parser.named_args[i].results.all_results[j]);
					break;
				}
				case(ArgType::kFloat): {
					delete static_cast<float*>(parser.named_args[i].results.all_results[j]);
					break;
				}
				case(ArgType::kInteger): {
					delete static_cast<int*>(parser.named_args[i].results.all_results[j]);
					break;
				}
			}
		}
		delete[] parser.named_args[i].results.all_results;
	}

	for(int i = 0; i < parser.count_of_pos_args; i++){
		for(int j  = 0; j < parser.pos_args[i].results.count_of_all_results; j++){
			switch(parser.pos_args[i].arg_type){
				case(ArgType::kChar): {
					delete[] static_cast<char*>(parser.pos_args[i].results.all_results[j]);
					break;
				}
				case(ArgType::kFloat): {
					delete static_cast<float*>(parser.pos_args[i].results.all_results[j]);
					break;
				}
				case(ArgType::kInteger): {
					delete static_cast<int*>(parser.pos_args[i].results.all_results[j]);
					break;
				}
			}
		}
		delete[] parser.pos_args[i].results.all_results;
	}



  	delete[] parser.flags;
  	delete[] parser.named_args;
  	delete[] parser.pos_args;
}

bool Parse(ArgumentParser& parser, int argc, const char* const* argv) {
	for (int i = 1; i < argc; ++i) {
		if(!ValidateArgumentSize(parser, argv[i])){
			return false;
		}
		ParseInfo is_flag_arg = ParseFlag(parser, argv, i);
		if (is_flag_arg.IsParsed) {
			continue;
		}

		ParseInfo is_splitted_arg = ParseSplittedNamedArgument(parser, argc, argv, i);
		if (is_splitted_arg.IsParsed) {
			if (!is_splitted_arg.ValidationFuncResult) {
				return false;
			}
			continue;
		}

		ParseInfo is_named_with_equals = ParseNamedArgumentWithEquals(parser, argv[i]);
		if (is_named_with_equals.IsParsed) {
			if (!is_named_with_equals.ValidationFuncResult) {
				return false;
			}
			continue;
		}

		ParseInfo is_pos_arg = ParsePosArgument(parser, argv[i]);
		if (is_pos_arg.IsParsed) {
			if (!is_pos_arg.ValidationFuncResult) {
				return false;
			}
			continue;
		}
	}
	
	return ValidateArgsAfterParsing(parser);
}

void AddFlag(ArgumentParser& parser, const char* short_flag, const char* long_flag, bool* result, const char* comment, bool default_value) {
  *result = default_value;
  if (parser.count_of_flags == parser.size_of_flags_array) {
    AllocateMoreFlagsMemory(parser);
  }
  parser.flags[parser.count_of_flags].short_flag = short_flag;
  parser.flags[parser.count_of_flags].long_flag = long_flag;
  parser.flags[parser.count_of_flags].result = result;
  parser.flags[parser.count_of_flags].comment = comment;
  parser.flags[parser.count_of_flags].default_value = default_value;

  parser.count_of_flags++;
}



#define IMPLEMENT_ADD_POS_ARGUMENT(TYPE, ARG_TYPE, FUNC_TYPE)                \
void AddArgument(ArgumentParser& parser, TYPE* result_value,               \
                   const char* name_of_arg, ParsingType type,                \
                   bool (*valid_func)(const FUNC_TYPE& value),               \
                   const char* valid_comment) {                              \
    if (parser.count_of_pos_args == parser.size_of_pos_args_array) {         \
      AllocateMorePosArgsMemory(parser);                                     \
    }                                                                        \
    parser.pos_args[parser.count_of_pos_args].name_of_arg = name_of_arg;     \
    parser.pos_args[parser.count_of_pos_args].results.first_result = result_value;\
    parser.pos_args[parser.count_of_pos_args].results.count_of_all_results=0;\
    parser.pos_args[parser.count_of_pos_args].results.size_of_all_results=0;\
    parser.pos_args[parser.count_of_pos_args].results.all_results=nullptr;\
    parser.pos_args[parser.count_of_pos_args].valid_func.TYPE##_valid_func = valid_func;\
    parser.pos_args[parser.count_of_pos_args].valid_comment = valid_comment; \
    parser.pos_args[parser.count_of_pos_args].type = type;                   \
    parser.pos_args[parser.count_of_pos_args].arg_type = ArgType::ARG_TYPE;  \
    parser.count_of_pos_args++;                                              \
  }

#define IMPLEMENT_ADD_NAMED_ARGUMENT(TYPE, ARG_TYPE, FUNC_TYPE)                \
  void AddArgument(                                                            \
      ArgumentParser& parser, const char* short_arg, const char* long_arg,     \
      TYPE* result_value, const char* name_of_arg, ParsingType type,           \
      bool (*valid_func)(const FUNC_TYPE& value), const char* valid_comment) { \
    if (parser.count_of_named_args == parser.size_of_named_args_array) {       \
      AllocateMoreNamedArgsMemory(parser);                                     \
    }                                                                          \
    parser.named_args[parser.count_of_named_args].short_arg = short_arg;       \
    parser.named_args[parser.count_of_named_args].long_arg = long_arg;         \
    parser.named_args[parser.count_of_named_args].name_of_arg = name_of_arg;   \
    parser.named_args[parser.count_of_named_args].results.first_result = result_value;\
    parser.named_args[parser.count_of_named_args].results.count_of_all_results=0;\
    parser.named_args[parser.count_of_named_args].results.size_of_all_results=0;\
	parser.named_args[parser.count_of_named_args].results.all_results=nullptr;\
    parser.named_args[parser.count_of_named_args].valid_func.TYPE##_valid_func = valid_func;\
    parser.named_args[parser.count_of_named_args].valid_comment = valid_comment;\
    parser.named_args[parser.count_of_named_args].type = type;                 \
    parser.named_args[parser.count_of_named_args].arg_type = ArgType::ARG_TYPE;\
    parser.count_of_named_args++;                                              \
  }

IMPLEMENT_ADD_NAMED_ARGUMENT(int, kInteger, int)

IMPLEMENT_ADD_NAMED_ARGUMENT(float, kFloat, float)

IMPLEMENT_ADD_NAMED_ARGUMENT(void, kChar, char* const)

IMPLEMENT_ADD_POS_ARGUMENT(float, kFloat, float)

IMPLEMENT_ADD_POS_ARGUMENT(void, kChar, char* const)

IMPLEMENT_ADD_POS_ARGUMENT(int, kInteger, int)

int GetRepeatedCount(ArgumentParser& parser, const char* name_of_arg) {
    NamedArgument* potential_named_arg = FindNamedArgumentByName(parser, name_of_arg);\
    if(potential_named_arg!=nullptr){\
		return (*potential_named_arg).results.count_of_all_results;
    }\
    PosArgument* potential_pos_arg = FindPosArgumentByName(parser, name_of_arg);\
    if(potential_pos_arg!=nullptr){\
		return (*potential_pos_arg).results.count_of_all_results;
	}\
	return -1;\
}

bool GetRepeated(ArgumentParser& parser, const char* name_of_arg, int pos_of_arg,const char** result) {\
    NamedArgument* potential_named_arg = FindNamedArgumentByName(parser, name_of_arg);\
    if(potential_named_arg!=nullptr){\
        if((*potential_named_arg).results.count_of_all_results <= pos_of_arg){\
            return false;\
        }\
        char* arg_val = static_cast<char*>((*potential_named_arg).results.all_results[pos_of_arg]);\
        *result = arg_val;
        return true;\
    }\
    PosArgument* potential_pos_arg = FindPosArgumentByName(parser, name_of_arg);\
    if(potential_pos_arg!=nullptr){\
        if((*potential_pos_arg).results.count_of_all_results <= pos_of_arg){\
            return false;\
        }\
        char* arg_val = static_cast<char*>((*potential_pos_arg).results.all_results[pos_of_arg]);\
        *result = arg_val;\
		return true;\
    }\
    return false;\
}

#define IMPLEMENT_GET_REPEATED(TYPE)  \
bool GetRepeated(ArgumentParser& parser, const char* name_of_arg, int pos_of_arg, TYPE* result) {\
    NamedArgument* potential_named_arg = FindNamedArgumentByName(parser, name_of_arg);\
    if(potential_named_arg!=nullptr){\
        if((*potential_named_arg).results.count_of_all_results <= pos_of_arg){\
            return false;\
        }\
        *result = *(static_cast<TYPE*>((*potential_named_arg).results.all_results[pos_of_arg]));\
        return true;\
    }\
    PosArgument* potential_pos_arg = FindPosArgumentByName(parser, name_of_arg);\
    if(potential_pos_arg!=nullptr){\
        if((*potential_pos_arg).results.count_of_all_results <= pos_of_arg){\
            return false;\
        }\
        *result = *(static_cast<TYPE*>((*potential_pos_arg).results.all_results[pos_of_arg]));\
        return true;\
    }\
    return false;\
}

IMPLEMENT_GET_REPEATED(int)

IMPLEMENT_GET_REPEATED(float)

void AddHelp(ArgumentParser& parser) {
	parser.help_added=true;
}

void PrintHelp(ArgumentParser& parser) {
	if(!parser.help_added){
		return;
	}
	std::cout << "Named arguments:\n" << "Count of named arguments = " << parser.count_of_named_args<<"\n\n";
	for(int i = 0; i < parser.count_of_named_args; i++){
		std::cout << parser.named_args[i].short_arg << ' ' << parser.named_args[i].long_arg << ' ' << parser.named_args[i].name_of_arg<<"\n";
	}
	std::cout<<"\n====================================\n\n\n\n\n";
	std::cout << "Positional arguments:\n" << "Count of positional arguments = " << parser.count_of_named_args<<"\n";
	for(int i = 0; i < parser.count_of_pos_args; i++){
		std::cout << parser.pos_args[i].name_of_arg<<"\n";
	}
}

}  // namespace nargparse