#include "clintrinsics/database.hpp"

#include <string>

clintrinsics::task<std::string> testco(std::string s, uint32_t delay_ms)
{
    std::string result;
    for (int i = 0; i < 10; ++i)
    {
        printf("s = \"%s\", i = %d\n", s.c_str(), i);
        result += "(" + s + ")";
        co_await clintrinsics::later(delay_ms);
    }
    printf("s = \" %s \", finished\n", s.c_str());
    co_return result;
}

clintrinsics::task<> testco2(uint32_t delay_ms)
{
    printf("loop 1 returned: %s\n", (co_await testco("loop 1", delay_ms)).c_str());
    printf("loop 2 returned: %s\n", (co_await testco("loop 2", delay_ms)).c_str());
    printf("testco2 finished\n");
}

clintrinsics::task<> testdb()
{
    auto db = co_await clintrinsics::open_db("foo");
    printf("> database handle: %p\n", db.handle);
    auto trx = db.create_transaction(true);
    printf("> trx handle: %p\n", trx.handle);
    co_await trx.set_kv("abcd", "efgh");
    co_await trx.set_kv("ijkl", "mnop");

    // for co_await(auto x: co_await trx.everything())
    //     printf("... blob handle %p\n", x.handle);

    trx.commit();
}

int main()
{
    printf("starting coroutines...\n");
    //testco("delay 1s", 1000).start();
    //testco("delay 2s", 2000).start();
    //testco("delay 3s", 3000).start();
    // testco2(200).start();
    // testco2(250).start();
    testdb().start();
    printf("main returned\n");
}
