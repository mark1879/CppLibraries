#include <iostream>
#include <functional>
#include <thread>
#include <future>
#include <chrono>
#include "thread_pool.hpp"

using namespace std;

int sum1(int a, int b)
{
    this_thread::sleep_for(chrono::seconds(2));
    // 比较耗时
    return a + b;
}

int sum2(int a, int b, int c)
{
    this_thread::sleep_for(chrono::seconds(2));
    return a + b + c;
}

int main() {

    ThreadPool pool;
    pool.Start(1);

    char op = '2';

    do
    {
        for (int i = 0; i < 50; i++)
        {
            future<int> r3 = pool.SubmitTask([](int b, int e)->int {
                int sum = 0;
                for (int i = b; i <= e; i++)
                sum += i;
                return sum;
            }, 1, 100);

            std::cout << "task result: " << r3.get() << endl;
        }

        std::cout << "1. continue" << std::endl;
        std::cout << "2. exit" << std::endl;

        op = getchar();

        while(getchar() != '\n')
            ;

    } while (op == '1');
    
    return 0;
}