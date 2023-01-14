#include <tau/tau.h>
#include "../src/lib/strpls.h"


TEST(StringPlusLibraryTestSuite, StringSwapTest) {
    char first[3] = "ABC";
    char second[3] = "def";
    strswp(first, second);
    CHECK_EQ(strcmp(first, "def"), 0);
    CHECK_EQ(strcmp(second, "ABC"), 0);
}

TEST(StringPlusLibraryTestSuite, RemoveAllAfterTest) {
    char example[10] = "ABAACBACAB";
    removeAllAfter(example, 'C');
    CHECK_EQ(strcmp(example, "ABAACBA"), 0);
}

TEST(StringPlusLibraryTestSuite, RemoveAllOccurencesTest) {
    char example[10] = "ABCACBAACB";
    removeAllOccurences(example, 'A');
    CHECK_EQ(strstr(example, "BCCBCB"), example);
}
