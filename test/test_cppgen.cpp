#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "cppgen/cppgen.h"
int main(int argc, char* argv[]) {
  std::filesystem::path cppgen_yaml_path = CPPGEN_TEST_DIR;
  cppgen_yaml_path += "/SolidEngine/cppgen.yaml";

  std::filesystem::path cppgen_template_path1 = CPPGEN_TEST_DIR;
  cppgen_template_path1 += "/SolidEngine/test1.cgt";

  std::ofstream cgt_fout1(cppgen_template_path1);
  cgt_fout1 << R"(#LOOP ClassDecl
#FILE $&.underline$.txt
in class $&.toCamel$:
#LOOP FieldDecl
$&.fieldname$ :
    it's a $&.typename$ of class $1&.toCamel$
#LOOP FieldDecl
this will not display because the loop is empty
C$&.underline$
#LOOPEND
#LOOPEND
#LOOPEND
)";
  std::filesystem::path cppgen_template_path2 = CPPGEN_TEST_DIR;
  cppgen_template_path2 += "/SolidEngine/test2.cgt";

  std::ofstream cgt_fout2(cppgen_template_path2);
  cgt_fout2 << R"(#FILE sumary.txt
#LOOP ClassDecl
in class $&.toCamel$:
#LOOP FieldDecl
$&.fieldname$ :
    it's a $&.typename$ of class $1&.toCamel$
#LOOP FieldDecl
this will not display because the loop is empty
C$&.underline$
#LOOPEND
#LOOPEND
#LOOPEND
)";
  cgt_fout1.close();
  cgt_fout2.close();
  auto start_time = std::chrono::system_clock::now();
  cppgen::CppGen cppgen(cppgen_yaml_path.string().c_str());
  cppgen.run();

  auto duration_time = std::chrono::system_clock::now() - start_time;
  std::cout << "Completed in "
            << std::chrono::duration_cast<std::chrono::milliseconds>(duration_time).count() << "ms"
            << std::endl;
}