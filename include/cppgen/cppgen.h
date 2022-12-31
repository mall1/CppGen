#pragma once
namespace cppgen {
  class CppGenImpl;
  class CppGen {
    CppGenImpl* m_impl;

  public:
    CppGen(const char* profile_full_path);
    ~CppGen();
    void run();
  };
};  // namespace cppgen
