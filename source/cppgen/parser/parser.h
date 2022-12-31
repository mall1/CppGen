#pragma once
#include <filesystem>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>

#include "cppgen/cursor/cursor.h"
namespace cppgen {
  class Config {
  public:
    Config(std::filesystem::path filename);

    std::filesystem::path m_root_dir;

    std::string m_project_input_file;
    std::vector<std::filesystem::path> m_work_paths;
    std::vector<std::filesystem::path> m_template_files;
    std::set<CXCursorKind> m_gather_contexts;
    std::string m_module_name;
    std::string m_sys_include;
    std::filesystem::path m_output_root;
    std::vector<std::filesystem::path> m_parse_files;

    std::vector<std::filesystem::path> m_source_files;
    std::vector<std::filesystem::path> m_header_files;
    std::vector<std::string> m_arguments;
    std::vector<std::string> m_source_extensions;

    bool m_is_show_errors;
  };

  class MetaParser {
  public:
    MetaParser(std::filesystem::path profile_full_path);
    ~MetaParser();
    int parse();

    std::vector<std::shared_ptr<Cursor::ASTContext>> m_contexts;
    std::unique_ptr<Config> m_config;

  private:
    CXIndex m_index{nullptr};
    std::vector<CXTranslationUnit> m_translation_units;

    std::unordered_map<std::string, std::string> m_type_table;

  private:
    bool parseProject(void);
    void gatherASTContext(Cursor& cursor, CXFile curfile);
    void printASTContext(std::shared_ptr<Cursor::ASTContext>& contexts);
    std::string getIncludeFile(std::string name);
  };
}  // namespace cppgen
