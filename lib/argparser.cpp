#include "argparser.h"

namespace nargparse{
  namespace{
    void AllocateMoreFlagsMemory(ArgumentParser& parser){
      Flag* new_flags = new Flag[parser.size_of_flags_array*2];
      for(int i = 0; i < parser.count_of_flags; i++){
        new_flags[i]=parser.flags[i];
      }
      delete[] parser.flags;
      parser.flags=new_flags;
      parser.size_of_flags_array*=2;
    }
    void AllocateMoreFlagsMemory(ArgumentParser& parser){
      Argument* new_args = new Argument[parser.size_of_args_array*2];
      for(int i = 0; i < parser.count_of_args; i++){
        new_args[i]=parser.args[i];
      }
      delete[] parser.args;
      parser.args=new_args;
      parser.size_of_args_array*=2;
    }
  }
  ArgumentParser CreateParser(const char* name_of_parser, const size_t max_arg_length){
    ArgumentParser parser;
    parser.name_of_parser=name_of_parser;
    parser.max_arg_length = max_arg_length;

    parser.count_of_args=0;
    parser.count_of_flags=0;
    parser.size_of_args_array=0;
    parser.size_of_flags_array=0;

    return parser;
  }

  void FreeParser(ArgumentParser parser){

  }

  bool Parse(ArgumentParser& parser, int argc, const char** argv){
    return false;
  }

  void AddFlag(ArgumentParser& parser, const char* short_flag, const char* long_flag, bool* result, const char* comment, bool default_value){
    *result=default_value;
    if(parser.count_of_flags==parser.size_of_flags_array){
      AllocateMoreFlagsMemory(parser);
    }
    parser.flags[parser.count_of_flags].short_flag=short_flag;
    parser.flags[parser.count_of_flags].long_flag=long_flag;
    parser.flags[parser.count_of_flags].result=result;
    parser.flags[parser.count_of_flags].comment=comment;
    parser.flags[parser.count_of_flags].default_value=default_value;

    parser.count_of_flags++;
  }

  void AddArgument(ArgumentParser& parser, const char* short_arg, const char* long_arg, int* result_value, const char* name_of_arg, ParsingType type, bool (*valid_func)(const int& value), const char* valid_comment){
    
  }

  void AddArgument(ArgumentParser& parser, const char* short_arg, const char* long_arg, float* result_value, const char* name_of_arg, ParsingType type, bool (*valid_func)(const float& value), const char* valid_comment){

  }

  void AddArgument(ArgumentParser& parser, const char* short_arg, const char* long_arg, void* result_value, const char* name_of_arg, ParsingType type, bool (*valid_func)(const char* const& value), const char* valid_comment){

  }

  void AddArgument(ArgumentParser& parser, void* result_value, const char* name_of_arg, ParsingType type, bool (*valid_func)(const char* const &value)){

  }

  void AddArgument(ArgumentParser& parser, int* result_value, const char* name_of_arg, ParsingType type, bool (*valid_func)(const int& value), const char* valid_comment){

  }

  int GetRepeatedCount(ArgumentParser& parser, const char* comment){
    return 0;
  }



  bool GetRepeated(ArgumentParser& parser, const char* name_of_arg, int pos_of_arg, void* result){
    return true;
  }
  bool GetRepeated(ArgumentParser& parser, const char* name_of_arg, int pos_of_arg, int* result){
    return true;
  }
  bool GetRepeated(ArgumentParser& parser, const char* name_of_arg, int pos_of_arg, float* result){
    return true;
  }

  void AddHelp(ArgumentParser& parser){

  }

}