#pragma once
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <stack>
#include <string>
#include <vector>

namespace cppgen {
  class ListIter;
  struct Converter {
    std::string m_name;
    std::function<std::string(std::string, std::shared_ptr<ListIter>&, size_t)> f_convert;
  };
  struct List {
    std::string m_name;
    std::vector<std::string> m_values;
  };
  class ListIter {
  protected:
    List* m_list;
    size_t m_index;

  public:
    size_t get_cur_index() { return m_index; }
    virtual size_t size() { return m_list->m_values.size(); }
    size_t m_begin_line;
    ListIter(List* list, size_t index) : m_list(list), m_index(index) {}
    virtual std::string getCur(int shift = 0) { return m_list->m_values[m_index + shift]; }
    virtual ~ListIter() {}

    bool next() { return ++m_index < size(); }
  };

  class SimpleData {
  public:
    std::vector<List> m_lists;

    virtual std::shared_ptr<ListIter> getListIterByName(
        const std::string& name, const std::stack<std::shared_ptr<ListIter>>& stack) {
      auto res = std::find_if(m_lists.begin(), m_lists.end(),
                              [name](auto& l) { return l.m_name == name; });
      if (res == m_lists.end()) {
        return std::make_shared<ListIter>(nullptr, 0);
      }

      else {
        return std::make_shared<ListIter>(&(*res), 0);
      }
    }
    virtual void reset() {}
    virtual ~SimpleData() {}
  };

  class SimpleGen {
  public:
    std::vector<Converter> m_converters;
    std::stack<std::shared_ptr<ListIter>> m_stack;
    std::shared_ptr<SimpleData> m_lists;
    std::vector<std::string> m_lines;
    void split(const std::string& s, std::vector<std::string>& tokens,
               const std::string& delimiters = " ") {
      std::string::size_type lastPos = 0;  // s.find_first_not_of(delimiters, 0);
      std::string::size_type pos = s.find_first_of(delimiters, lastPos);
      while (std::string::npos != pos || std::string::npos != lastPos) {
        tokens.emplace_back(s.substr(lastPos, pos - lastPos));
        lastPos = pos + 1;
        if (lastPos >= s.size()) break;
        // lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
      }
    }
    std::string getIndex(int outter, int inner, std::shared_ptr<ListIter>& layer) {
      std::stack<std::shared_ptr<ListIter>> tmp_stack;
      for (int i = 0; i < outter; i++) {
        tmp_stack.push(m_stack.top());
        m_stack.pop();
      }
      layer = m_stack.top();

      while (tmp_stack.size()) {
        m_stack.push(tmp_stack.top());
        tmp_stack.pop();
      }

      return layer->getCur(inner);
    }
    std::string replace(std::string source) {
      std::smatch replacement_match;
      auto replacement = std::regex("[$]([0-1]*)&([0-1]*)(.([a-zA-Z_0-9]+))*[$]");
      while (std::regex_search(source, replacement_match, replacement)) {
        std::string raw = replacement_match[0].str();
        int outter = std::atoi(replacement_match[1].str().c_str());
        int inner = std::atoi(replacement_match[2].str().c_str());
        std::shared_ptr<ListIter> layer;
        std::string value = getIndex(outter, inner, layer);

        std::string converter_name = replacement_match[4].str();
        for (auto& converter : m_converters) {
          if (converter.m_name == converter_name) {
            auto pos = source.find(raw);
            source = source.replace(pos, raw.size(), converter.f_convert(value, layer, inner));
            break;
          }
        }
      }
      return source;
    }
    void parsefile(std::filesystem::path filepath, std::filesystem::path rootFolder) {
      // Sanity check
      if (!std::filesystem::is_regular_file(filepath)) return;

      // Open the file
      // Note that we have to use binary mode as we want to return a string
      // representing matching the bytes of the file on the file system.
      std::ifstream file(filepath, std::ios::in | std::ios::binary);
      if (!file.is_open()) return;

      // Read contents
      std::string filecontent{std::istreambuf_iterator<char>(file),
                              std::istreambuf_iterator<char>()};

      // Close the file
      file.close();

      parse(filecontent, rootFolder);
    }
    void parse(std::string ofs, std::filesystem::path rootFolder) {
      m_lists->reset();
      ofs += '\n';
      auto loop_begin = std::regex("#LOOP ([a-zA-Z0-9_.$&]+)");
      auto file = std::regex("#FILE ([a-zA-Z0-9_.$&/]+)");
      auto loop_end = std::regex("#LOOPEND");
      std::smatch loop_name_match;
      std::smatch file_match;
      std::ofstream out;
      split(ofs, m_lines, "\n");
      for (size_t i = 0; i < m_lines.size(); i++) {
        if (m_lines[i].size() && m_lines[i].back() == '\r') {
          m_lines[i] = m_lines[i].substr(0, m_lines[i].size() - 1);
        }
        auto generated = m_lines[i];
        if (std::regex_match(generated, file_match, file)) {
          if (file_match.size() == 2) {
            std::string name = file_match[1].str();
            name = replace(name);
            auto file = rootFolder / name;
            std::filesystem::create_directories(file.parent_path());
            out = std::ofstream(file);
          }
        } else if (std::regex_match(generated, loop_name_match, loop_begin)) {
          if (loop_name_match.size() == 2) {
            std::string name = loop_name_match[1].str();
            name = replace(name);
            auto listiter = m_lists->getListIterByName(name, m_stack);
            listiter->m_begin_line = i;
            m_stack.push(listiter);
          }
        } else if (std::regex_match(generated, loop_end)) {
          if (m_stack.top()->next()) {
            i = m_stack.top()->m_begin_line;
          } else {
            m_stack.pop();
          }
        } else {
          bool empty_loop = false;
          if (!m_stack.empty() && m_stack.top()->size() == 0) {
            empty_loop = true;
          }
          if (!empty_loop) {
            out << replace(generated) << std::endl;
          }
        }
      }
    }
  };

}  // namespace cppgen