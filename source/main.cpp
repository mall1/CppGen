#include <chrono>
#include <filesystem>
#include <iostream>

#include "cppgen/cppgen.h"

int main(int argc, char* argv[]) {
  std::filesystem::path work_dir;
  std::filesystem::path profile_full_path;
  if (argc >= 2 && std::filesystem::exists(argv[1])
      && std::filesystem::path(argv[1]).is_absolute()) {
    profile_full_path = argv[1];
    std::cout << "profile_full_path: " << profile_full_path << std::endl;
  } else {
    return -1;
  }

  auto start_time = std::chrono::system_clock::now();
  cppgen::CppGen cppgen(profile_full_path.string().c_str());
  cppgen.run();

  auto duration_time = std::chrono::system_clock::now() - start_time;
  std::cout << "Completed in "
            << std::chrono::duration_cast<std::chrono::milliseconds>(duration_time).count() << "ms"
            << std::endl;
}