#include "cppgen/generator/dynamicData.h"

#include "magic_enum.hpp"
namespace cppgen {

  void cppgen::DynamicData::gather_by_type(
      const std::string& type, const std::vector<std::shared_ptr<Cursor::ASTContext>>& source,
      std::vector<std::shared_ptr<Cursor::ASTContext>>& target) {
    for (auto& c : source) {
      auto aaa = magic_enum::enum_cast<CXCursorKind>("CXCursor_" + type);
      if (c->m_handle.kind == aaa) {
        target.push_back(c);
      }
    }
  }
}  // namespace cppgen