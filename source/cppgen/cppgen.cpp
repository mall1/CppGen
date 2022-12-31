#include "cppgen/cppgen.h"

#include <chrono>
#include <filesystem>
#include <iostream>

#include "cppgen/generator/codeGenerator.h"
#include "cppgen/generator/dynamicData.h"
#include "cppgen/parser/parser.h"
namespace cppgen {

  static std::string toCamel(std::string former, bool firstUp = false) {
    bool turn = firstUp;
    for (auto& c : former) {
      if (turn) {
        c = std::toupper(c);
      }
      if (c == '_') {
        turn = true;
      } else {
        turn = false;
      }
    }
    return std::regex_replace(former, std::regex("_"), "");
  }
  static std::string lowerall(std::string former) {
    return std::regex_replace(former, std::regex("_"), "");
  }

  CXCursor getcursor(std::shared_ptr<ListIter>& layer, int inner) {
    std::shared_ptr<DynamicIter> top_iter = std::static_pointer_cast<DynamicIter>(layer);
    if (top_iter->m_contexts.size()) {
      return top_iter->m_contexts[top_iter->get_cur_index() + inner]->m_handle;
    }
    return CXCursor();
  }

  class CppGenImpl {
    std::shared_ptr<MetaParser> m_parser;
    std::shared_ptr<SimpleGen> m_generator;

  public:
    CppGenImpl(std::filesystem::path profile_full_path)
        : m_parser(std::make_shared<MetaParser>(profile_full_path)),
          m_generator(std::make_shared<SimpleGen>()) {
      m_generator->m_converters = {
          {"toCamelUp", [](std::string former, std::shared_ptr<ListIter>&,
                           size_t) { return toCamel(former, true); }},
          {"toCamel",
           [](std::string former, std::shared_ptr<ListIter>&, size_t) { return toCamel(former); }},
          {"underline",
           [](std::string former, std::shared_ptr<ListIter>&, size_t) { return former; }},
          {"lowerall",
           [](std::string former, std::shared_ptr<ListIter>&, size_t) { return lowerall(former); }},
          {"typename",
           [=](std::string former, std::shared_ptr<ListIter>& listiter, size_t inner) {
             Cursor c(getcursor(listiter, inner));
             return c.getType().GetDisplayName();
             ;
           }},
          {"fieldname", [=](std::string former, std::shared_ptr<ListIter>& listiter, size_t inner) {
             Cursor c(getcursor(listiter, inner));
             return c.getDisplayName();
             ;
           }}};

      m_generator->m_lists = std::make_shared<DynamicData>(m_parser);
    }

    void run() {
      if (m_parser->parse() == 0) {
        for (auto file : m_parser->m_config->m_template_files) {
          m_generator->parsefile(file, m_parser->m_config->m_output_root);
        }
      }
    }
  };

  CppGen::CppGen(const char* profile_full_path) { m_impl = new CppGenImpl(profile_full_path); }
  CppGen::~CppGen() { delete m_impl; }

  void CppGen::run() { m_impl->run(); }
}  // namespace cppgen