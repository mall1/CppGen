#pragma once

#include "clang-c/Index.h"

#include <memory>
#include <string>
#include <vector>

#include "cppgen/cursor/cursor_type.h"
#define Debug
namespace cppgen {

  void toString(const CXString& str, std::string& output);

  class Cursor {
  public:
    typedef std::pair<std::string, std::string> Attribute;
    typedef std::vector<Cursor> List;
    struct ASTContext {
      CXCursor m_handle;
      std::vector<Attribute> attrs;
      std::vector<std::shared_ptr<ASTContext>> contexts;
    };
    typedef CXCursorVisitor Visitor;

    Cursor(const CXCursor& handle);
    void setFather(Cursor* father) { m_father = father; }

    CXCursorKind getKind(void) const;

    std::string getSpelling(void) const;
    std::string getDisplayName(void) const;

    CXFile getSourceFile(void) const;

    bool isDefinition(void) const;

    CursorType getType(void) const;

    List getChildren(void) const;
    void visitChildren(Visitor visitor, void* data = nullptr);
    std::vector<Attribute> extractAttributes() const;
    Cursor* m_father = nullptr;
    void newContext(std::vector<Attribute> attrs) {
      m_context = std::make_shared<ASTContext>();
      m_context->attrs = attrs;
      m_context->m_handle = m_handle;
    }
    std::shared_ptr<ASTContext> m_context;

  private:
    CXCursor m_handle;
#ifdef Debug
    std::string name;
    std::string spelling;
    CursorType type;

#endif
  };
}  // namespace cppgen