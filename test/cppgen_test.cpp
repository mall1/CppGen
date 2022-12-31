#include "cppgen/cppgen.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
int main(int argc, char* argv[])
{
    std::filesystem::path cppgen_yaml_path = CPPGEN_TEST_DIR;
    cppgen_yaml_path += "/SolidEngine/cppgen.yaml";

	std::filesystem::path cppgen_template_path = CPPGEN_TEST_DIR;
    cppgen_template_path += "/SolidEngine/test.cgt";

	std::ofstream cgt_fout(cppgen_template_path);
    cgt_fout << R"(#FILE aaa.txt
#LOOP ClassDecl
#FILE aaa.txt
A$&.underline$
#LOOP FieldDecl
in class $1&.toCamel$
Bt:$&.typename$
Bf:$&.fieldname$
Bf:$&.fieldname.toCamel$
#LOOP FieldDecl
C$&.underline$
#LOOPEND
#LOOPEND
#LOOPEND
)";
    cgt_fout.close();
    auto start_time = std::chrono::system_clock::now();
    cppgen::CppGen cppgen(cppgen_yaml_path.string().c_str());
    cppgen.run();

    auto duration_time = std::chrono::system_clock::now() - start_time;
    std::cout << "Completed in " << std::chrono::duration_cast<std::chrono::milliseconds>(duration_time).count() << "ms"
              << std::endl;
}