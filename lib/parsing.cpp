#include <cstring>

#include "parsing.h"
#include "validation.h"
#include "argparser.h"

namespace nargparse{


    
#define IMPLEMENT_REALLOC_RESULTS_MEMORY(NAMED_OR_POS)                  \
  void ReallocResultsMemory(NAMED_OR_POS& argument) {                   \
    size_t new_size = (argument.results.size_of_all_results == 0) ? 1 : (argument.results.size_of_all_results * 2); \
    void** new_arr = new void*[new_size];                               \
    for (int i = 0; i < argument.results.count_of_all_results; i++) {   \
      new_arr[i] = argument.results.all_results[i];                     \
    }                                                                   \
    delete[] argument.results.all_results;                              \
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
        	delete[] new_str;
			return {true, false};
    	}
    	AddNewResult(*potential_named_arg, new_str);
  	}
	else if ((*potential_named_arg).arg_type == ArgType::kFloat) {
		float* arg_val = new float;
		if(!IsValidFloat(argv[i+1])){
			delete arg_val;
			return {true,false};
		}
		*arg_val = StringToFloat(argv[i+1]);
		if((*potential_named_arg).results.count_of_all_results==0){
			float* result = static_cast<float*>((*potential_named_arg).results.first_result);
			*result = *arg_val;
		}
		if ((*potential_named_arg).valid_func.float_valid_func !=nullptr && !(*potential_named_arg).valid_func.float_valid_func(*arg_val)) {
			delete arg_val;
			return {true, false};
		}
		AddNewResult(*potential_named_arg, arg_val);  
  	}
  	else if ((*potential_named_arg).arg_type == ArgType::kInteger) {
		int* arg_val = new int;
		if(!IsValidInteger(argv[i+1])){
			delete arg_val;
			return {true,false};
		}
		*arg_val=StringToInt(argv[i+1]);
		if((*potential_named_arg).results.count_of_all_results==0){
			int* result = static_cast<int*>((*potential_named_arg).results.first_result);
			*result = *arg_val;
		}
		if ((*potential_named_arg).valid_func.int_valid_func != nullptr && !(*potential_named_arg).valid_func.int_valid_func(*arg_val)) {
			delete arg_val;
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
			delete[] new_str;
			return {true, false};
		}
		AddNewResult(*potential_named_arg_with_equals, new_str);

  	} 
	else if ((*potential_named_arg_with_equals).arg_type == ArgType::kFloat) {

    	float* arg_val = new float;
		if(!IsValidFloat(ExtractNamedArgWithEqualsValue(argv))){
			delete arg_val;
			return {true,false};
		}
    	*arg_val = StringToFloat(ExtractNamedArgWithEqualsValue(argv));
    	if((*potential_named_arg_with_equals).results.count_of_all_results==0){
        	float* result = static_cast<float*>((*potential_named_arg_with_equals).results.first_result);
        	*result = *arg_val;
    	}
    	if ((*potential_named_arg_with_equals).valid_func.float_valid_func !=nullptr && !(*potential_named_arg_with_equals).valid_func.float_valid_func(*arg_val)) {
        	delete arg_val;
			return {true, false};
    	}
    	AddNewResult(*potential_named_arg_with_equals, arg_val);  

  	} 
	else if ((*potential_named_arg_with_equals).arg_type == ArgType::kInteger) {

    	int* arg_val = new int;
    	*arg_val=StringToInt(ExtractNamedArgWithEqualsValue(argv));
		if(!IsValidInteger(ExtractNamedArgWithEqualsValue(argv))){
			delete arg_val;
			return {true,false};
		}
    	if((*potential_named_arg_with_equals).results.count_of_all_results==0){
      		int* result = static_cast<int*>((*potential_named_arg_with_equals).results.first_result);
        	*result = StringToInt(ExtractNamedArgWithEqualsValue(argv));
    	}

    	if ((*potential_named_arg_with_equals).valid_func.int_valid_func != nullptr && !(*potential_named_arg_with_equals).valid_func.int_valid_func(*arg_val)) {
      		delete arg_val;
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
			delete[] new_str;
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
			delete arg_val;
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
			delete arg_val;
			return {true, false};
		}
		AddNewResult(*potential_pos_arg, arg_val);

  	}
  	return {true, true};
}



}