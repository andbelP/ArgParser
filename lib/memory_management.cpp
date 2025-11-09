#include "memory_management.h"

namespace nargparse{

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



}