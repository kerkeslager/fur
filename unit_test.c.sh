find . -name '*.h' | sed 's/.*/#include "&"/g'
echo
echo 'int main() {'
grep --no-filename 'void test_' *.h | sed -E 's/void (test_[A-Za-z_]*)\(\);/  \1();/g'
echo '  return 0;'
echo '}'
