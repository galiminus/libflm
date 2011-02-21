#include <stdlib.h>
#include <unistd.h>

#include <check.h>

#include "test.h"

int
main (void)
{
    int number_failed;

    Suite * bufferSuite = buffer_suite ();
    SRunner * bufferRunner = srunner_create (bufferSuite);

    Suite * containerSuite = container_suite ();
    SRunner * containerRunner = srunner_create (containerSuite);

    Suite * monitorSuite = monitor_suite ();
    SRunner * monitorRunner = srunner_create (monitorSuite);
    
    srunner_run_all (bufferRunner, CK_NORMAL);
    number_failed = 0;
    number_failed += srunner_ntests_failed (bufferRunner);
    srunner_free (bufferRunner);

    srunner_run_all (containerRunner, CK_NORMAL);
    number_failed = 0;
    number_failed += srunner_ntests_failed (containerRunner);
    srunner_free (containerRunner);

    srunner_run_all (monitorRunner, CK_NORMAL);
    number_failed = 0;
    number_failed += srunner_ntests_failed (monitorRunner);
    srunner_free (monitorRunner);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
