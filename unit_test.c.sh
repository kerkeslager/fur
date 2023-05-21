echo '#include <stdio.h>'
echo '#include <stdlib.h>'
echo
find . -name '*.h' | sed 's/.*/#include "&"/g'
echo
echo 'int main() {'
echo '  size_t count = 0;'
grep --no-filename 'void test_' *.h | sed -E 's/void (test_[A-Za-z_]*)\(\);/  \1(); count++; printf(".");/g'
echo '  printf("\\n%zu TESTS PASSED\n\n", count);'
echo '  return 0;'
echo '}'
