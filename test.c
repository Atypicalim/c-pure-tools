#define TEST

#ifdef TEST
#include "test.h"
#else
#include "tools.h"
#endif


int main(int argc, char const *argv[])
{
    log_info("test...");
    //
    return 0;
}

