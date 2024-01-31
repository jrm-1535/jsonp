
#ifndef __UTEST_H__
#define __UTEST_H__

static char *test_suite = "";
static bool verbose = true; //false;

static unsigned int test_attempted = 0;
static unsigned int test_failure = 0;

#define NO_SETUP
#define NO_TEAR_DOWN

#define PRINT_NORMAL(...) do { if(verbose) printf( __VA_ARGS__ ); } while (0)
#define PRINT_IMPORTANT(...) do { if(verbose) { printf( "\x1b[01;39m" ); \
                                                printf( __VA_ARGS__ );   \
                                                printf( "\x1b[00;39m" ); \
                                              } } while (0)

// printf("\x1b[32m[ TEST %s ......... ]\x1b[39m\n", __func__ );

#define START_TEST( _name, _s )                                        \
static void _name( void ) {                                            \
    bool failure = false;                                              \
    ++test_attempted;                                                  \
    if ( verbose ) printf("    [ TEST %s ......... ]\n", __func__ ); \
    _s;

#define END_TEST( _t )                                                 \
exit:  _t;                                                             \
    if ( verbose ) printf( "%s    [ TEST %s %s ]\x1b[00;39m\n",        \
                           (failure) ? "\x1b[01;31m" : "\x1b[32m",     \
                           __func__,                                   \
                           (failure) ? "FAILED   " : "SUCCEEDED" );    \
    if ( failure ) ++test_failure;                                     \
}

#define CHECK( _cond ) do { if ( ! (_cond) ) {                                     \
                                        printf("\x1b[01;31m%s:%d:Assert( %s ) failed" \
                                                "\x1b[00;39m\n",                   \
                                               __func__, __LINE__, #_cond);        \
                                         failure = true; } } while(0)

#define ASSERT( _cond ) do { if ( ! (_cond) ) {                                    \
                                        printf("\x1b[01;31m%s:%d:Assert( %s ) failed" \
                                                "\x1b[00;39m\n",                   \
                                               __func__, __LINE__, #_cond);        \
                                         failure = true; goto exit; } } while(0)
#define CHECK_EQUAL( _v1, _v2 ) do { if (_v1 != _v2) {                           \
                                         printf("\x1b[01;31m%s:%d:Assert equal failed" \
                                                "\x1b[00;39m\n",                 \
                                               __func__, __LINE__ );             \
                                         failure = true; } } while(0)

#define ASSERT_EQUAL( _v1, _v2 ) do { if (_v1 != _v2) {                          \
                                         printf("\x1b[01;31m%s:%d:Assert equal failed" \
                                                "\x1b[00;39m\n",                 \
                                               __func__, __LINE__ );             \
                                         failure = true; goto exit; } } while(0)

#define CHECK_DIFFERENT( _v1, _v2 ) do { if (_v1 == _v2) {                        \
                                         printf("\x1b[01;31m%s:%d:Assert different failed" \
                                                "\x1b[00;39m\n",                   \
                                               __func__, __LINE__ );               \
                                        failure = true; } } while(0)

#define ASSERT_DIFFERENT( _v1, _v2 ) do { if (_v1 == _v2) {                        \
                                         printf("\x1b[01;31m%s:%d:Assert different failed" \
                                                "\x1b[00;39m\n",                   \
                                               __func__, __LINE__ );               \
                                        failure = true; goto exit; } } while(0)

#define BEGIN_TEST_SUITE( _name )  do { test_suite = #_name; \
                     test_attempted = test_failure = 0;      \
                     printf("\x1b[01;39mUnit test suite <%s> begins\x1b[00;39m\n", \
                            test_suite ); } while(0);

#define END_TEST_SUITE()                                                           \
    do { printf("\x1b[01;39mUnit test suite <%s> completed: passed %d, failed %d\x1b[00;39m\n", \
            test_suite, test_attempted - test_failure, test_failure ); } while(0);

#endif /* __UTEST_H__ */

