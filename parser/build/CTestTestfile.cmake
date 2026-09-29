# CMake generated Testfile for 
# Source directory: C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser
# Build directory: C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(parser_tests "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/build/Debug/parser_unit_tests.exe" "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/test/cases/prueba1.txt")
  set_tests_properties(parser_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/CMakeLists.txt;23;add_test;C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(parser_tests "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/build/Release/parser_unit_tests.exe" "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/test/cases/prueba1.txt")
  set_tests_properties(parser_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/CMakeLists.txt;23;add_test;C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(parser_tests "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/build/MinSizeRel/parser_unit_tests.exe" "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/test/cases/prueba1.txt")
  set_tests_properties(parser_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/CMakeLists.txt;23;add_test;C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(parser_tests "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/build/RelWithDebInfo/parser_unit_tests.exe" "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/test/cases/prueba1.txt")
  set_tests_properties(parser_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/CMakeLists.txt;23;add_test;C:/Users/USUARIO/developer/compiladores/CompiladoresTeamUCSP/parser/CMakeLists.txt;0;")
else()
  add_test(parser_tests NOT_AVAILABLE)
endif()
