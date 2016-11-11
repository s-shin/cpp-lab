#include <iostream>
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include "cpplab_generated.h"

const char* kCpplabFbsPath = "src/flatbuffers-parse-json/cpplab.fbs";
const char* kFooUserJsonPath = "src/flatbuffers-parse-json/foo_user.json";
const char* kFooGroupJsonPath = "src/flatbuffers-parse-json/foo_group.json";

int main(int argc, char** argv) {
  {
    flatbuffers::Parser parser;
    std::cout << "Load fbs file: " << kCpplabFbsPath << std::endl;
    std::string fbs;
    if (!flatbuffers::LoadFile(kCpplabFbsPath, false, &fbs)) {
      std::cout << "  => Failed..." << std::endl;
      return 1;
    }
    std::cout << "  => OK" << std::endl;
    std::cout << "Parse fbs data." << std::endl;
    if (!parser.Parse(fbs.c_str())) {
      std::cout << "  => Failed..." << std::endl
                << "Fbs data:" << std::endl
                << "===" << std::endl
                << fbs << std::endl
                << "===" << std::endl;
      return 1;
    }
    std::cout << "  => OK" << std::endl;

    std::cout << "Load json file: " << kFooUserJsonPath << std::endl;
    std::string user_data;
    if (!flatbuffers::LoadFile(kFooUserJsonPath, false, &user_data)) {
      std::cout << "  => Failed..." << std::endl;
      return 1;
    }
    std::cout << "  => OK" << std::endl;
    std::cout << "Parse json data" << std::endl;
    parser.SetRootType("User");
    if (!parser.Parse(user_data.c_str())) {
      std::cout << "  => Failed..." << std::endl
                << "JSON data:" << std::endl
                << "===" << std::endl
                << user_data << std::endl
                << "===" << std::endl;
    }
    std::cout << "  => OK" << std::endl;
    auto user =
        flatbuffers::GetRoot<cpplab::User>(parser.builder_.GetBufferPointer());
    std::cout << "# User name: " << user->name()->str() << std::endl
              << "# Location: " << user->location()->str() << std::endl;
  }

  {
    flatbuffers::Parser parser;
    std::cout << "Load fbs file: " << kCpplabFbsPath << std::endl;
    std::string fbs;
    if (!flatbuffers::LoadFile(kCpplabFbsPath, false, &fbs)) {
      std::cout << "  => Failed..." << std::endl;
      return 1;
    }
    std::cout << "  => OK" << std::endl;
    std::cout << "Parse fbs file data." << std::endl;
    if (!parser.Parse(fbs.c_str())) {
      std::cout << "  => Failed..." << std::endl
                << "Fbs data:" << std::endl
                << "===" << std::endl
                << fbs << std::endl
                << "===" << std::endl;
      return 1;
    }
    std::cout << "  => OK" << std::endl;

    std::cout << "Load json file: " << kFooGroupJsonPath << std::endl;
    std::string group_data;
    if (!flatbuffers::LoadFile(kFooGroupJsonPath, false, &group_data)) {
      std::cout << "  => Failed..." << std::endl;
      return 1;
    }
    std::cout << "  => OK" << std::endl;
    std::cout << "Parse json data" << std::endl;
    parser.SetRootType("UserGroup");
    if (!parser.Parse(group_data.c_str())) {
      std::cout << "  => Failed..." << std::endl
                << "JSON data:" << std::endl
                << "===" << std::endl
                << group_data << std::endl
                << "===" << std::endl;
    }
    std::cout << "  => OK" << std::endl;
    auto group = flatbuffers::GetRoot<cpplab::UserGroup>(
        parser.builder_.GetBufferPointer());
    std::cout << "# Group name: " << group->name()->str() << std::endl
              << "# Users:" << std::endl;
    for (const auto u : *(group->users())) {
      std::cout << "# - " << u->name()->str() << std::endl;
    }
  }

  return 0;
}
