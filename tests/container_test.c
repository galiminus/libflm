#include <check.h>

#include "flm/flm.h"

#include "test_utils.h"

START_TEST(test_container_create)
{
    if (flm_ContainerNew (NULL, NULL) == NULL) {
        fail ("Container creation failed");
    }
}
END_TEST

START_TEST(test_container_content)
{
    flm_Container * container;
    
    if ((container = flm_ContainerNew ((void*)42, NULL)) == NULL) {
        fail ("Container creation failed");
    }
    fail_unless (flm_ContainerContent (container) == ((void*)42));
}
END_TEST

int _has_freed;

void
_container_free_handler (void * content) {
    _has_freed = 1;
}

START_TEST(test_container_free)
{
    flm_Container * container;
    
    _has_freed = 0;
    if ((container = flm_ContainerNew (NULL, _container_free_handler)) == NULL) {
        fail ("Container creation failed");
    }
    flm_Release (FLM_OBJ (container));
    fail_if(_has_freed == 0);
}
END_TEST

START_TEST(test_container_alloc_fail)
{
    flm_Container * container;

    setTestAlloc (1);
    fail_if (flm_ContainerNew (NULL, NULL) != NULL);
}
END_TEST

START_TEST(test_container_destruct)
{
    flm_Container * container;

    setTestAlloc (0);
    if ((container = flm_ContainerNew (NULL, NULL)) == NULL) {
        fail ("Container creation failed");
    }
    flm_Release (FLM_OBJ (container));
    fail_unless (getAllocSum () == 0);
}
END_TEST

Suite *
container_suite (void)
{
  Suite * s = suite_create ("container");

  /* Container test case */
  TCase *tc_core = tcase_create ("Container");

  tcase_add_test (tc_core, test_container_create);
  tcase_add_test (tc_core, test_container_content);
  tcase_add_test (tc_core, test_container_free);
  tcase_add_test (tc_core, test_container_alloc_fail);
  tcase_add_test (tc_core, test_container_destruct);
  suite_add_tcase (s, tc_core);

  return s;
}
