#include <cstring>

#include "argparser.h"
#include "validation.h"



namespace nargparse{

namespace{

bool IsDigit(char a){
	return a<='9' && a>='0'; 
}

} // namespace


bool ValidateArgumentSize(ArgumentParser& parser, const char* arg){
	if(strlen(arg)>=parser.max_arg_length){
		return false;
	}
	return true;
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



} // namespace nargparse
