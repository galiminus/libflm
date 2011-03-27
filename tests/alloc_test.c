#include <check.h>

#include "flm/flm.h"

#include "test_utils.h"

START_TEST(test_alloc)
{
    flm_BufferRelease (flm_BufferNew ("", 0, NULL));
}
END_TEST

START_TEST(test_alloc_check)
{
    setTestAlloc (0);
    flm_BufferRelease (flm_BufferNew ("", 0, NULL));
    fail_unless (getAllocSum () == 0);
}
END_TEST

Suite *
alloc_suite (void)
{
    Suite * s = suite_create ("alloc");

    TCase * tc_core = tcase_create("Core");
    tcase_add_test (tc_core, test_alloc);
    tcase_add_test (tc_core, test_alloc_check);
    suite_add_tcase (s, tc_core);

    return (s);
}
