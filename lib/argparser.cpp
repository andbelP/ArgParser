#include <iostream>
#include <cstring>

#include "argparser.h"

namespace nargparse {

namespace {

struct ParseInfo {
  bool IsParsed;
  bool ValidationFuncResult;
};

}  // namespace

namespace {
float StringToFloat(const char* str) {
  float result = 0.;
  while (*str != '\0' && *str != '.') {
    result = result * 10.;
    result += (*str - '0');
    str++;
  }
  float div = 10.0;
  if (*str == '\0') {
    return result;
  }
  str++;
  while (*str != '\0') {
    result += (*str - '0') / div;
    div *= 10;
    str++;
  }
  return result;
}
int StringToInt(const char* str) {
  int result = 0;
  while (*str != '\0') {
    result *= 10;
    result += (*str - '0');
    str++;
  }
  return result;
}
bool StartsWith(const char* str, const char* start) {
  if (strlen(start) > strlen(str)) {
    return false;
  }
  for (int i = 0; i < strlen(start); i++) {
    if (start[i] != str[i]) {
      return false;
    }
  }
  return true;
}
bool IsDigit(char a){
	return a<='9' && a>='0'; 
}
bool IsValidInteger(const char* arg){
	while(*arg!='\0'){
		if(!IsDigit(*arg)){
			return false;
		}
		arg++;
	}
	return true;
}
bool IsValidFloat(const char* arg){
	while(*arg!='\0' && *arg!='.'){
		if(!IsDigit(*arg)){
			return false;
		}
		arg++;
	}
	if(*arg=='\0'){
		return true;
	}
	arg++;
	while(*arg!='\0'){
		if(!IsDigit(*arg)){
			return false;
		}
		arg++;
	}
	return true;
}
}  // namespace

namespace {
#define IMPLEMENT_REALLOC_RESULTS_MEMORY(NAMED_OR_POS)                  \
  void ReallocResultsMemory(NAMED_OR_POS& argument) {                   \
    size_t new_size = (argument.results.size_of_all_results == 0) ? 1 : (argument.results.size_of_all_results * 2); \
    void** new_arr = new void*[new_size];                               \
    for (int i = 0; i < argument.results.count_of_all_results; i++) {   \
      new_arr[i] = argument.results.all_results[i];                     \
    }                                                                   \
    if(argument.results.all_results!=nullptr){\
      delete[] argument.results.all_results;                              \
    }\
    argument.results.all_results = new_arr;       \
	argument.results.size_of_all_results=new_size;\
  }

IMPLEMENT_REALLOC_RESULTS_MEMORY(NamedArgument);

IMPLEMENT_REALLOC_RESULTS_MEMORY(PosArgument);

#define IMPLEMENT_ADD_NEW_RESULT(NAMED_OR_POS)                       \
  void AddNewResult(NAMED_OR_POS& argument, void* value) {           \
    ReallocResultsMemory(argument);                                  \
    size_t count_of_results = argument.results.count_of_all_results; \
    argument.results.all_results[count_of_results] = value;          \
    argument.results.count_of_all_results++;                         \
  }

IMPLEMENT_ADD_NEW_RESULT(NamedArgument)

IMPLEMENT_ADD_NEW_RESULT(PosArgument)
bool ValidateArgumentSize(ArgumentParser& parser, const char* arg){
	if(strlen(arg)>=parser.max_arg_length){
		return false;
	}
	return true;
}

void AllocateMoreFlagsMemory(ArgumentParser& parser) {
  Flag* new_flags = new Flag[parser.size_of_flags_array * 2];
  for (int i = 0; i < parser.count_of_flags; i++) {
    new_flags[i] = parser.flags[i];
  }
  delete[] parser.flags;
  parser.flags = new_flags;
  parser.size_of_flags_array *= 2;
}

void AllocateMorePosArgsMemory(ArgumentParser& parser) {
  PosArgument* new_args = new PosArgument[parser.size_of_pos_args_array * 2];
  for (int i = 0; i < parser.count_of_pos_args; i++) {
    new_args[i] = parser.pos_args[i];
  }
  delete[] parser.pos_args;
  parser.pos_args = new_args;
  parser.size_of_pos_args_array *= 2;
}

void AllocateMoreNamedArgsMemory(ArgumentParser& parser) {
  NamedArgument* new_args = new NamedArgument[parser.size_of_named_args_array * 2];
  for (int i = 0; i < parser.count_of_named_args; i++) {
    new_args[i] = parser.named_args[i];
  }
  delete[] parser.named_args;
  parser.named_args = new_args;
  parser.size_of_named_args_array *= 2;
}

NamedArgument* FindNamedArgumentByName(ArgumentParser& parser, const char* name_of_arg){
    for(int i = 0; i < parser.count_of_named_args; i++){
        if(strcmp(parser.named_args[i].name_of_arg, name_of_arg)==0){
            return &parser.named_args[i];
        }
    }
    return nullptr;
}
PosArgument* FindPosArgumentByName(ArgumentParser& parser, const char* name_of_arg){
    for(int i = 0; i < parser.count_of_pos_args; i++){
        if(strcmp(parser.pos_args[i].name_of_arg, name_of_arg)==0){
            return &parser.pos_args[i];
        }
    }
    return nullptr;
}

Flag* FindFlag(ArgumentParser& parser, const char* arg) {
  for (int i = 0; i < parser.count_of_flags; i++) {
    if (parser.flags[i].short_flag != nullptr && strcmp(parser.flags[i].short_flag, arg) == 0 ||
                                            parser.flags[i].long_flag != nullptr && strcmp(parser.flags[i].long_flag, arg) == 0) {
      return &parser.flags[i];
    }
  }
  return nullptr;
}

NamedArgument* FindSplittedNamedArgument(ArgumentParser& parser, const char* arg) {
  for (int i = 0; i < parser.count_of_named_args; i++) {
    if (parser.named_args[i].short_arg != nullptr && strcmp(parser.named_args[i].short_arg, arg) == 0 ||
        parser.named_args[i].long_arg != nullptr && strcmp(parser.named_args[i].long_arg, arg) == 0) {
      return &parser.named_args[i];
    }
  }
  return nullptr;
}

NamedArgument* FindNamedArgumentWithEquals(ArgumentParser& parser, const char* arg) {
  for (int i = 0; i < parser.count_of_named_args; i++) {
    if (parser.named_args[i].long_arg != nullptr && StartsWith(arg, parser.named_args[i].long_arg) 
            && strlen(arg) > strlen(parser.named_args[i].long_arg) + 1 && arg[strlen(parser.named_args[i].long_arg)] == '=') {
      return &parser.named_args[i];
    }
  }
  return nullptr;
}

PosArgument* FindPosArg(ArgumentParser& parser, const char* arg) {
  for (int i = 0; i < parser.count_of_pos_args; i++) {
    if (parser.pos_args[i].results.count_of_all_results == 0 || parser.pos_args[i].type==ParsingType::kNargsOneOrMore 
		|| parser.pos_args[i].type==ParsingType::kNargsZeroOrMore) {
      return &parser.pos_args[i];
    }
  }
  return nullptr;
}

bool IsNamedArgWithEquals(const char* arg) {
  if (strlen(arg) < 5) {
    return false;
  }
  if (*arg != '-' || *(arg + 1) != '-') {
    return false;
  }
  arg += 2;
  while (*arg != '=' && *arg != '\0') {
    arg++;
  }
  if (*arg == '\0') {
    return false;
  }
  arg++;
  if (*arg == '\0') {
    return false;
  }
  return true;
}

const char* ExtractNamedArgWithEqualsValue(const char* arg) {
  while (*arg != '=') {
    arg++;
  }
  arg++;
  return arg;
}

ParseInfo ParseFlag(ArgumentParser& parser, const char* const* argv, int i) {
  Flag* potential_flag = FindFlag(parser, argv[i]);
  if (potential_flag == nullptr) {
    return {false, false};
  }

  *(*potential_flag).result = true;
  return ParseInfo{true, true};
}

ParseInfo ParseSplittedNamedArgument(ArgumentParser& parser, int argc, const char* const* argv, int& i) {
	NamedArgument* potential_named_arg = FindSplittedNamedArgument(parser, argv[i]);
  	if (potential_named_arg == nullptr) {
    	return {false, false};
  	}

  	if (i >= (argc - 1) || !ValidateArgumentSize(parser, argv[i+1])) {
    	return {true, false};
  	}	

  	if ((*potential_named_arg).arg_type == ArgType::kChar) {
    	const char* arg_val = argv[i+1];
    	char* new_str = new char[strlen(arg_val)+1];
    	strcpy(new_str, arg_val);
    	if ((*potential_named_arg).results.count_of_all_results == 0) {
      		char* result = static_cast<char*>((*potential_named_arg).results.first_result);
      		strcpy(result, arg_val); 
    	}
    	if ((*potential_named_arg).valid_func.void_valid_func != nullptr && !(*potential_named_arg).valid_func.void_valid_func(new_str)) {
        	return {true, false};
    	}
    	AddNewResult(*potential_named_arg, new_str);
  }
	else if ((*potential_named_arg).arg_type == ArgType::kFloat) {
		float* arg_val = new float;
		if(!IsValidFloat(argv[i+1])){
			return {true,false};
		}
		*arg_val = StringToFloat(argv[i+1]);
		if((*potential_named_arg).results.count_of_all_results==0){
			float* result = static_cast<float*>((*potential_named_arg).results.first_result);
			*result = *arg_val;
		}
		if ((*potential_named_arg).valid_func.float_valid_func !=nullptr && !(*potential_named_arg).valid_func.float_valid_func(*arg_val)) {
			return {true, false};
		}
		AddNewResult(*potential_named_arg, arg_val);  
  }
  else if ((*potential_named_arg).arg_type == ArgType::kInteger) {
		int* arg_val = new int;
		if(!IsValidInteger(argv[i+1])){
			return {true,false};
		}
		*arg_val=StringToInt(argv[i+1]);
		if((*potential_named_arg).results.count_of_all_results==0){
			int* result = static_cast<int*>((*potential_named_arg).results.first_result);
			*result = *arg_val;
		}
		if ((*potential_named_arg).valid_func.int_valid_func != nullptr && !(*potential_named_arg).valid_func.int_valid_func(*arg_val)) {
			return {true, false};
		}
		AddNewResult(*potential_named_arg, arg_val);
  }

  i++;
  return {true, true};
}

ParseInfo ParseNamedArgumentWithEquals(ArgumentParser& parser, const char* argv) {
	NamedArgument* potential_named_arg_with_equals = FindNamedArgumentWithEquals(parser, argv);
	if (potential_named_arg_with_equals == nullptr) {
		return {false, false};
  	}

	if ((*potential_named_arg_with_equals).arg_type == ArgType::kChar) {

		const char* arg_val = ExtractNamedArgWithEqualsValue(argv);
		char* new_str = new char[strlen(arg_val)+1];
		strcpy(new_str, arg_val);
		if ((*potential_named_arg_with_equals).results.count_of_all_results == 0) {
			char* result = static_cast<char*>((*potential_named_arg_with_equals).results.first_result);
			strcpy(result, arg_val); 
		}
		if ((*potential_named_arg_with_equals).valid_func.void_valid_func != nullptr && !(*potential_named_arg_with_equals).valid_func.void_valid_func(new_str)) {
			return {true, false};
		}
		AddNewResult(*potential_named_arg_with_equals, new_str);

  	} 
	else if ((*potential_named_arg_with_equals).arg_type == ArgType::kFloat) {

    	float* arg_val = new float;
		if(!IsValidFloat(ExtractNamedArgWithEqualsValue(argv))){
			return {true,false};
		}
    	*arg_val = StringToFloat(ExtractNamedArgWithEqualsValue(argv));
    	if((*potential_named_arg_with_equals).results.count_of_all_results==0){
        	float* result = static_cast<float*>((*potential_named_arg_with_equals).results.first_result);
        	*result = *arg_val;
    	}
    	if ((*potential_named_arg_with_equals).valid_func.float_valid_func !=nullptr && !(*potential_named_arg_with_equals).valid_func.float_valid_func(*arg_val)) {
        	return {true, false};
    	}
    	AddNewResult(*potential_named_arg_with_equals, arg_val);  

  	} 
	else if ((*potential_named_arg_with_equals).arg_type == ArgType::kInteger) {

    	int* arg_val = new int;
    	*arg_val=StringToInt(ExtractNamedArgWithEqualsValue(argv));
		if(!IsValidInteger(ExtractNamedArgWithEqualsValue(argv))){
			return {true,false};
		}
    	if((*potential_named_arg_with_equals).results.count_of_all_results==0){
      		int* result = static_cast<int*>((*potential_named_arg_with_equals).results.first_result);
        	*result = StringToInt(ExtractNamedArgWithEqualsValue(argv));
    	}

    	if ((*potential_named_arg_with_equals).valid_func.int_valid_func != nullptr && !(*potential_named_arg_with_equals).valid_func.int_valid_func(*arg_val)) {
      		return {true, false};
    	}
    	AddNewResult(*potential_named_arg_with_equals, arg_val);
  	}
  	return {true, true};
}

ParseInfo ParsePosArgument(ArgumentParser& parser, const char* argv) {
  	PosArgument* potential_pos_arg = FindPosArg(parser, argv);
  	if (potential_pos_arg == nullptr) {
    	return {false, false};
  	}

  	if ((*potential_pos_arg).arg_type == ArgType::kChar) {

		if((*potential_pos_arg).results.count_of_all_results==0){
			char* result = static_cast<char*>((*potential_pos_arg).results.first_result);
			strcpy(result, argv);
		}
		char* new_str = new char[strlen(argv)+1];
		strcpy(new_str, argv);
		if ((*potential_pos_arg).valid_func.void_valid_func != nullptr && !(*potential_pos_arg).valid_func.void_valid_func(new_str)) {
			return {true, false};
		}
		AddNewResult(*potential_pos_arg, new_str);

  	} 
	else if ((*potential_pos_arg).arg_type == ArgType::kFloat) {
		if(!IsValidFloat(argv)){
			return {true,false};
		}
		float* arg_val = new float;
		*arg_val = StringToFloat(argv);
		if((*potential_pos_arg).results.count_of_all_results==0){
			float* result = static_cast<float*>((*potential_pos_arg).results.first_result);
			*result = *arg_val;
		}
		if ((*potential_pos_arg).valid_func.float_valid_func != nullptr && !(*potential_pos_arg).valid_func.float_valid_func(*arg_val)) {
			return {true, false};
		}
		AddNewResult(*potential_pos_arg, arg_val);
	} 
	else if ((*potential_pos_arg).arg_type == ArgType::kInteger) {
		if(!IsValidInteger(argv)){
			return {true,false};
		}
		int* arg_val = new int;
		*arg_val=StringToInt(argv);
		if((*potential_pos_arg).results.count_of_all_results==0){
			int* result = static_cast<int*>((*potential_pos_arg).results.first_result);
			*result = *arg_val;
		}
		if ((*potential_pos_arg).valid_func.int_valid_func != nullptr && !(*potential_pos_arg).valid_func.int_valid_func(*arg_val)) {
			return {true, false};
		}
		AddNewResult(*potential_pos_arg, arg_val);

  }
  return {true, true};
}

bool ValidateArgsAfterParsing(ArgumentParser& parser){
	int count_of_results;
	for (int i = 0; i < parser.count_of_named_args; i++){
		count_of_results = parser.named_args[i].results.count_of_all_results;
		if ((parser.named_args[i].type == ParsingType::kNargsRequired || parser.named_args[i].type == ParsingType::kNargsOneOrMore) && count_of_results == 0){
			return false;
		}
		if(parser.named_args[i].type == ParsingType::kNargsRequired && count_of_results>1){
			return false;
		}
	}
	for (int i = 0; i < parser.count_of_pos_args; i++){
		count_of_results = parser.pos_args[i].results.count_of_all_results;
		if ((parser.pos_args[i].type == ParsingType::kNargsRequired || parser.pos_args[i].type == ParsingType::kNargsOneOrMore) && count_of_results == 0){
			return false;
		}
		if(parser.pos_args[i].type == ParsingType::kNargsRequired && count_of_results>1){
			return false;
		}
	}
	return true;
}

}  // namespace

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
					delete static_cast<char*>(parser.named_args[i].results.all_results[j]);
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
			switch(parser.named_args[i].arg_type){
				case(ArgType::kChar): {
					delete static_cast<char*>(parser.pos_args[i].results.all_results[j]);
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
		ParseInfo IsFlagArg = ParseFlag(parser, argv, i);
		if (IsFlagArg.IsParsed) {
			continue;
		}

		ParseInfo IsSplittedNamedArg = ParseSplittedNamedArgument(parser, argc, argv, i);
		if (IsSplittedNamedArg.IsParsed) {
			if (!IsSplittedNamedArg.ValidationFuncResult) {
				return false;
			}
			continue;
		}

		ParseInfo IsNamedWithEquals = ParseNamedArgumentWithEquals(parser, argv[i]);
		if (IsNamedWithEquals.IsParsed) {
			if (!IsNamedWithEquals.ValidationFuncResult) {
				return false;
			}
			continue;
		}

		ParseInfo IsPosArg = ParsePosArgument(parser, argv[i]);
		if (IsPosArg.IsParsed) {
			if (!IsPosArg.ValidationFuncResult) {
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