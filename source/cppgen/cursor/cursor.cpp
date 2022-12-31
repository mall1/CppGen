
#include "cppgen/cursor/cursor.h"

#include <clang-c/Index.h>

#include <string>
namespace cppgen {

  void toString(const CXString& str, std::string& output) {
    auto cstr = clang_getCString(str);

    output = cstr;

    clang_disposeString(str);
  }
  std::vector<std::string> split(std::string input, std::string pat) {
    std::vector<std::string> ret_list;
    while (true) {
      size_t index = input.find(pat);
      std::string sub_list = input.substr(0, index);
      if (!sub_list.empty()) {
        ret_list.push_back(sub_list);
      }
      input.erase(0, index + pat.size());
      if (index == -1) {
        break;
      }
    }
    return ret_list;
  }

  std::string trim(std::string& source_string, const std::string trim_chars) {
    size_t left_pos = source_string.find_first_not_of(trim_chars);
    if (left_pos == std::string::npos) {
      source_string = std::string();
    } else {
      size_t right_pos = source_string.find_last_not_of(trim_chars);
      source_string = source_string.substr(left_pos, right_pos - left_pos + 1);
    }
    return source_string;
  }

  Cursor::Cursor(const CXCursor& handle)
      : m_handle(handle)
#ifdef Debug
        ,
        type(getType()),

        name(getDisplayName()),
        spelling(getSpelling())
#endif
  {
  }

  CXCursorKind Cursor::getKind(void) const { return m_handle.kind; }

  std::string Cursor::getSpelling(void) const {
    std::string spelling;

    toString(clang_getCursorSpelling(m_handle), spelling);

    return spelling;
  }

  std::string Cursor::getDisplayName(void) const {
    std::string display_name;

    toString(clang_getCursorDisplayName(m_handle), display_name);

    return display_name;
  }
  CXFile Cursor::getSourceFile(void) const {
    auto range = clang_Cursor_getSpellingNameRange(m_handle, 0, 0);

    auto start = clang_getRangeStart(range);

    CXFile file;
    unsigned line, column, offset;
    clang_getFileLocation(start, &file, &line, &column, &offset);
    return file;
  }

  bool Cursor::isDefinition(void) const { return clang_isCursorDefinition(m_handle); }

  CursorType Cursor::getType(void) const { return clang_getCursorType(m_handle); }

  Cursor::List Cursor::getChildren(void) const {
    List children;

    auto visitor = [](CXCursor cursor, CXCursor parent, CXClientData data) {
      auto container = static_cast<List*>(data);

      container->emplace_back(cursor);

      if (cursor.kind == CXCursor_LastPreprocessing) return CXChildVisit_Break;

      return CXChildVisit_Continue;
    };

    clang_visitChildren(m_handle, visitor, &children);

    return children;
  }

  void Cursor::visitChildren(Visitor visitor, void* data) {
    clang_visitChildren(m_handle, visitor, data);
  }

  std::vector<Cursor::Attribute> Cursor::extractAttributes() const

  {
    std::vector<Attribute> ret_list;

    auto propertyList = this->getDisplayName();

    auto&& properties = split(propertyList, ",");

    static const std::string white_space_string = " \t\r\n";

    for (auto& property_item : properties) {
      auto&& item_details = split(property_item, ":");
      auto&& temp_string = trim(item_details[0], white_space_string);
      if (temp_string.empty()) {
        continue;
      }
      ret_list.emplace_back(
          temp_string, item_details.size() > 1 ? trim(item_details[1], white_space_string) : "");
    }
    return ret_list;
  }
}  // namespace cppgen