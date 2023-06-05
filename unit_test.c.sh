echo '#include <stdio.h>'
echo '#include <stdlib.h>'
echo '#include <string.h>'
echo
find . -name '*.h' | sed 's/.*/#include "&"/g'
echo
echo '#define DO_TEST(t) if(argc == 1 || strstr(#t, argv[1])) { t(); count++; printf("."); }'
echo 'int main(int argc, char* argv[]) {'
echo '  size_t count = 0;'
grep --no-filename 'void test_' *.h | sed -E 's/void (test_[A-Za-z_0-9]*)\(\);/  DO_TEST(\1);/g'
echo '  printf("\\n%zu TESTS PASSED\\n\\n", count);'
echo '  return 0;'
echo '}'
