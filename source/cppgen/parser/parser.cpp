#include "cppgen/parser/parser.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "magic_enum.hpp"
#include "yaml-cpp/yaml.h"
namespace cppgen {

  Config::Config(std::filesystem::path filename) {
    YAML::Node root = YAML::LoadFile(filename.string());
    auto get_str
        = [&root](const char* key, std::string& str) { str = root[key].as<std::string>(); };

    auto get_strs = [&root](const char* key, std::vector<std::string>& strs) {
      strs.clear();
      for (YAML::const_iterator it = root[key].begin(); it != root[key].end(); ++it) {
        std::string path = it->as<std::string>();
        strs.push_back(path);
      }
    };
    auto get_paths = [&root](const char* key, std::filesystem::path root_dir,
                             std::vector<std::filesystem::path>& paths) {
      paths.clear();
      for (YAML::const_iterator it = root[key].begin(); it != root[key].end(); ++it) {
        std::string&& str = it->as<std::string>();
        std::filesystem::path path = str;

        if (path.is_absolute() && std::filesystem::exists(path)) {
          paths.push_back(path);
        } else {
          path = root_dir;
          path += "/";
          path += str;
          if (std::filesystem::exists(path)) {
            paths.push_back(path);
          }
        }
      }
    };
    auto get_kinds = [&root](const char* key, std::set<CXCursorKind>& kinds) {
      kinds.clear();
      for (YAML::const_iterator it = root[key].begin(); it != root[key].end(); ++it) {
        std::string&& path = it->as<std::string>();

        auto aaa = magic_enum::enum_cast<CXCursorKind>("CXCursor_" + path);
        if (aaa.has_value()) {
          kinds.insert(aaa.value());
        }
      }
    };

    m_root_dir = filename.parent_path();
    get_paths("include_paths", m_root_dir, m_work_paths);
    get_paths("template_files", m_root_dir, m_template_files);
    get_strs("arguments", m_arguments);
    get_strs("source_extensions", m_source_extensions);
    get_str("sys_include", m_sys_include);
    std::string output_root;
    get_str("output_root", output_root);
    m_output_root = m_root_dir;
    m_output_root += '/';
    m_output_root += output_root;
    get_paths("source_files", m_root_dir, m_parse_files);
    get_kinds("gather_contexts", m_gather_contexts);
    m_is_show_errors = root["is_show_errors"].as<bool>();

    std::queue<std::filesystem::path> dir_paths;
    for (auto& path : m_parse_files) {
      if (std::filesystem::is_directory(path)) {
        dir_paths.push(path);
      } else {
        auto extionsion = path.extension();
        if (std::find_if(m_source_extensions.begin(), m_source_extensions.end(),
                         [extionsion](auto& e) { return extionsion == e; })
            != m_source_extensions.end()) {
          m_source_files.push_back(path);
        }
      }
    }
    while (dir_paths.size()) {
      for (auto const& dir_entry : std::filesystem::directory_iterator{dir_paths.front()}) {
        auto path = dir_entry.path();
        if (std::filesystem::is_directory(path)) {
          dir_paths.push(path);
        } else {
          auto extionsion = path.extension();
          if (std::find_if(m_source_extensions.begin(), m_source_extensions.end(),
                           [extionsion](auto& e) { return extionsion == e; })
              != m_source_extensions.end()) {
            m_source_files.push_back(path);
          }
        }
      }
      dir_paths.pop();
    }

    std::string pre_include = "-I";
    std::string sys_include_temp;
    if (!(m_sys_include == "*")) {
      sys_include_temp = pre_include + m_sys_include;
      m_arguments.emplace_back(sys_include_temp);
    }

    for (auto& path : m_work_paths) {
      auto tmp = pre_include + path.string();
      m_arguments.emplace_back(tmp);
    }
  }
  std::string MetaParser::getIncludeFile(std::string name) {
    auto iter = m_type_table.find(name);
    return iter == m_type_table.end() ? std::string() : iter->second;
  }

  MetaParser::MetaParser(std::filesystem::path profile_full_path)
      : m_config(std::make_unique<Config>(profile_full_path)) {}

  MetaParser::~MetaParser() {
    for (auto translation_unit : m_translation_units) {
      clang_disposeTranslationUnit(translation_unit);
    }

    if (m_index) clang_disposeIndex(m_index);
  }

  int MetaParser::parse() {
    m_index = clang_createIndex(true, m_config->m_is_show_errors);

    std::vector<const char*> arguments_c;
    for (auto& a : m_config->m_arguments) {
      arguments_c.push_back(a.c_str());
    }

    for (auto& f : m_config->m_source_files) {
      auto translation_unit = clang_createTranslationUnitFromSourceFile(
          m_index, f.string().c_str(), static_cast<int>(arguments_c.size()), arguments_c.data(), 0,
          nullptr);
      auto cursor = clang_getTranslationUnitCursor(translation_unit);
      CXFile file = clang_getFile(translation_unit, f.string().c_str());
      m_translation_units.push_back(translation_unit);

      Cursor aa(cursor);
      gatherASTContext(aa, file);
    }

    for (auto& c : m_contexts) {
      printASTContext(c);
    }
    return 0;
  }

  void MetaParser::printASTContext(std::shared_ptr<Cursor::ASTContext>& context) {
    Cursor cursor = context->m_handle;
    switch (context->m_handle.kind) {
      case CXCursor_ClassDecl:
      case CXCursor_StructDecl:
        std::cout << cursor.getSpelling() << std::endl;
        break;
      case CXCursor_FieldDecl:
        bool is_const = cursor.getType().IsConst();
        std::string m_name = cursor.getSpelling();
        std::string m_type = cursor.getType().GetDisplayName();
        std::cout << is_const << m_type << " " << m_name << std::endl;
        break;
    }

    for (auto& c : context->contexts) {
      printASTContext(c);
    }
  }

  void MetaParser::gatherASTContext(Cursor& cursor, CXFile curfile) {
    auto file = cursor.getSourceFile();
    if (curfile != file && file != nullptr) return;
    for (auto& child : cursor.getChildren()) {
      child.setFather(&cursor);
      gatherASTContext(child, curfile);
    }

    auto kind = cursor.getKind();
    if (kind == CXCursor_AnnotateAttr) {
      cursor.m_father->newContext(cursor.extractAttributes());
    } else if (cursor.m_context != nullptr) {
      // gather contexts
      if (m_config->m_gather_contexts.contains(kind)) {
        m_contexts.push_back(cursor.m_context);
      }

      // add to father's contexts
      if (cursor.m_father != nullptr) {
        if (cursor.m_father->m_context == nullptr) {
          cursor.m_father->newContext(std::vector<Cursor::Attribute>());
        }
        cursor.m_father->m_context->contexts.push_back(cursor.m_context);
      }
    }
  }
}  // namespace cppgen