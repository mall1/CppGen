#pragma once
#include "cppgen/generator/codeGenerator.h"
#include "cppgen/parser/parser.h"
namespace cppgen {

  class DynamicIter : public ListIter {
  public:
    std::string name;
    std::vector<std::shared_ptr<Cursor::ASTContext>> m_contexts;
    DynamicIter(size_t index) : ListIter(nullptr, index) {}

    virtual size_t size() override { return m_contexts.size(); }

    virtual std::string getCur(int shift = 0) override {
      return Cursor(m_contexts[m_index + shift]->m_handle).getDisplayName();
    }
    virtual ~DynamicIter() {}
  };

  class DynamicData : public SimpleData {
    std::shared_ptr<MetaParser> m_parser;

  public:
    DynamicData(std::shared_ptr<MetaParser> parser) { m_parser = parser; }

    void gather_by_type(const std::string& type,
                        const std::vector<std::shared_ptr<Cursor::ASTContext>>& source,
                        std::vector<std::shared_ptr<Cursor::ASTContext>>& target);

    virtual std::shared_ptr<ListIter> getListIterByName(
        const std::string& name, const std::stack<std::shared_ptr<ListIter>>& stack) override {
      auto iter = std::make_shared<DynamicIter>(0);
      iter->name = name;

      if (stack.size() == 0) {
        gather_by_type(name, m_parser->m_contexts, iter->m_contexts);
      } else {
        std::shared_ptr<DynamicIter> top_iter = std::static_pointer_cast<DynamicIter>(stack.top());
        if (top_iter->m_contexts.size()) {
          gather_by_type(name, top_iter->m_contexts[top_iter->get_cur_index()]->contexts,
                         iter->m_contexts);
        }
      }
      std::cout << "getting " << iter->m_contexts.size() << " name:" << name << std::endl;
      return iter;
    }
    virtual ~DynamicData() override {}
  };
}  // namespace cppgen