#include "cppgen/generator/codeGenerator.h"
using namespace cppgen;
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

int main() {
  auto bbb = SimpleGen();

  bbb.m_converters = {
      {"toCamelUp", [](std::string former, std::shared_ptr<ListIter>&,
                       size_t) { return toCamel(former, true); }},
      {"toCamel",
       [](std::string former, std::shared_ptr<ListIter>&, size_t) { return toCamel(former); }},
      {"underline", [](std::string former, std::shared_ptr<ListIter>&, size_t) { return former; }},
      {"lowerall",
       [](std::string former, std::shared_ptr<ListIter>&, size_t) { return lowerall(former); }}};
  bbb.m_lists = std::make_shared<SimpleData>();
  bbb.m_lists->m_lists = {
      {"LAYERNAMES",
       {"platform_independence", "core_systems", "resources", "logical", "presentation",
        "game_specific_interface"}},
      {"platform_independence", {{"clock"}, {"memory_allocator"}, "Window"}},
      {"core_systems", {{"random_number"}, {"movie_player"}}},
      {"resources",
       {{"model_resource_manager"},
        {"texture_resource_manager"},
        {"material_resource_manager"},
        {"font_esource_manager"},
        {"skeleton_resource_manager"},
        {"collision_resource_manager"},
        {"physics_resource_manager"},
        {"world_resource_manager"}}},
      {"logical",
       {{"gameplay_foundations"},
        {"online_multiplayer"},
        {"profiling"},
        {"collision"},
        {"human_interface_devices"}}},
      {"presentation",
       {{"front_end"},
        {"visual_effects"},
        {"scene_graph"},
        {"low_level_renderer"},
        {"skeleton_animation"},
        {"audio"}}},
      {"game_specific_interface",
       {{"game_specific_rendering_interface"},
        {"player_mechanics_interface"},
        {"game_cameras_interface"},
        {"AI_interface"}}},
  };

  bbb.parse(R"(
#LOOP LAYERNAMES
#FILE source/abstraction/$&.toCamel$.hpp
#pragma once
#include "macroCandy.hpp"
using namespace solid::macro::property;
namespace solid::abstraction::$&.lowerall$ {

#LOOP $&.underline$
    META(enable)
    class I$&.toCamelUp$ {};
#LOOPEND

    META(enable)
    class Layer$&.toCamelUp$  final : public WriteableBeforeInit {
#LOOP $&.underline$
        ModulePointerProperty(I$&.toCamelUp$, $&.underline$,  nullptr);
#LOOPEND
      public:
        void init();
    };
}  // namespace solid::abstraction::$&.lowerall$
#LOOPEND
#FILE source/engineModel.hpp
#pragma once

#LOOP LAYERNAMES
#include "abstraction/$&.toCamel$.hpp"
#LOOPEND

namespace solid::enginemodel {

    META(enable)
    class EngineModel final {

#LOOP LAYERNAMES
        solid::abstraction::$&.lowerall$::Layer$&.toCamelUp$ m_layer_$&.underline$;
#LOOPEND

      public:
        void init();
    };
}  // namespace solid::enginemodel
#FILE source/engineModel.cpp
#include "engineModel.hpp"

#LOOP LAYERNAMES
#LOOP $&.underline$
#include "minimalImplement/$1&.toCamel$/minimal$&.toCamelUp$.hpp"
#LOOPEND
#LOOPEND

using namespace solid::enginemodel;

#LOOP LAYERNAMES
using namespace solid::abstraction::$&.lowerall$;
#LOOPEND

#LOOP LAYERNAMES
using namespace solid::minimalimplement::$&.lowerall$;
#LOOPEND

void EngineModel::init() {
#LOOP LAYERNAMES
    m_layer_$&.underline$.init();
#LOOPEND
}

#LOOP LAYERNAMES
void Layer$&.toCamelUp$::init() {
#LOOP $&.underline$
    if (!m_$&.underline$_module) 
        set_$&.underline$_module_before_init(std::make_shared<Minimal$&.toCamelUp$>());
#LOOPEND
    set_inited();
}

#LOOPEND
#LOOP LAYERNAMES
#LOOP $&.underline$
#FILE source/minimalImplement/$1&.toCamel$/minimal$&.toCamelUp$.hpp
#pragma once
#include "../../abstraction/$1&.toCamel$.hpp"
namespace solid::minimalimplement::$1&.lowerall$ {

    META(enable)
    class Minimal$&.toCamelUp$ : public abstraction::$1&.lowerall$::I$&.toCamelUp$ {
      public:
    };
}  // namespace solid::minimalimplement::$1&.lowerall$
#LOOPEND
#LOOPEND
#FILE source/macroCandy.hpp
#pragma once

#if defined(__REFLECTION_PARSER__)
#define META(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#define META(...)
#endif // __REFLECTION_PARSER__

#include <memory>

#include "assert.h"
namespace solid::macro::platform {
    enum class SystemType { Windows, LINUX, Android, UNKNOWN };
#ifdef _WIN32  // Windows (32-bit and 64-bit, this part is common)
#    define SystemTypeWindows
    const SystemType System = SystemType::Windows;
#    ifdef _WIN64  // Windows (64-bit only)
#    endif
#elif __APPLE__
#    include "TargetConditionals.h"
#    if TARGET_IPHONE_SIMULATOR
    // iOS Simulator
#    elif TARGET_OS_IPHONE
    // iOS device
#    elif TARGET_OS_MAC
    // Other kinds of Mac OS
#    else
#        error "Unknown Apple platform"
#    endif
#elif __linux__
#    define SystemTypeLinux
    const SystemType System = SystemType::LINUX;
    // linux
#elif __unix__  // all unix not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#    error "Unknown compiler"
#endif
}  // namespace solid::macro::platform


namespace solid::macro::assert {

#ifdef NDEBUG
#    define Assert(e) ((void)0)
#else
#    define Assert(e) ((void)((e) ? ((void)0) : (void)0))  // __assert(#    e, __FILE__, __LINE__)))
#endif
}  // namespace solid::macro::assert


namespace solid::macro::property {

#define ReadProperty(type, name, val)            \
  protected:                                     \
    META(enable)                                 \
    type m_##name{val};                          \
                                                 \
  public:                                        \
    const type get_##name() { return m_##name; } \
                                                 \
  private:

#define ReadWriteProperty(type, name, val)                \
    ReadProperty(type, name, val);                        \
                                                          \
  public:                                                 \
    void set_##name(type _##name) { m_##name = _##name; } \
                                                          \
  private:

    META(enable)
    class WriteableBeforeInit {
        bool m_writable{false};

      public:
        void set_inited() { m_writable = true; }
        bool get_writable() { return m_writable; }
    };
#define ReadWriteBeforeInitProperty(type, name, val) \
    ReadProperty(type, name, val);                   \
                                                     \
  public:                                            \
    void set_##name##_before_init(type _##name) {    \
        if (get_writable())                          \
            Assert(false);                           \
        else                                         \
            m_##name = _##name;                      \
    }                                                \
                                                     \
  private:

#define ModulePointerProperty(type, name, val) \
    ReadWriteBeforeInitProperty(std::shared_ptr<type>, name##_module, val)

}  // namespace solid::macro::property
#FILE CMakeLists.txt
cmake_minimum_required(VERSION 3.24)
project(SolidEngine)

file(GLOB_RECURSE HEADERS "source/*.h" "source/*.hpp")
file(GLOB_RECURSE SOURCES "source/*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADERS} ${SOURCES})
add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)
#FILE cppgen.yaml
sys_include: "*"
module_name: MetaParserTest
is_show_errors: false
source_files:
  - "./source/"
source_extensions:
  - .h
  - .cpp
  - .hpp
template_files:
  - "test1.cgt"
  - "test2.cgt"
include_paths:
  - "source"
output_root: "generated"
arguments:
  - -x
  - c++
  - -std=c++20
  - -D__REFLECTION_PARSER__
  - -DNDEBUG
  - -D__clang__
  - -w
  - -MG
  - -M
  - -ferror-limit=0
  - -o clangLog.txt
gather_contexts:
  - StructDecl
  - ClassDecl
)",
            R"(SolidEngine)");
  return 0;
}
