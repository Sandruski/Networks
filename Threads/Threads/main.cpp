#include <thread>
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <mutex>

void thread_function(int arg, int arg2)
{
	Sleep(1000);
	std::cout << "Arg1: " << arg << std::endl;
	std::cout << "Arg2: " << arg2 << std::endl;
	Sleep(1000);
}

// main thread
/*
int main()
{
	std::thread t = std::thread(thread_function, 30, 40);

	t.join(); // waits until thread t finishes

	system("pause");
	return 0;
}
*/

//----------------------------------------------------------------------------------------------------

int counter = 0; // Global var
std::mutex mtx; // Mutex object

void increment(int iterations) 
{
	for (int i = 0; i < iterations; ++i) 
	{
		std::unique_lock<std::mutex> lock(mtx); // Lock the mutex
		// What is the smallest piece of code that I can block?
		counter++;
	}
}

/*
mov reg1 counter;
inc counter;
mov counter reg1;
*/

int main() 
{
	std::thread t1(increment, 100000); // Execute 'increment' in a thread
	std::thread t2(increment, 100000); // Execute 'increment' in a thread
	t1.join(); // Wait for t1
	t2.join(); // Wait for t2
	std::cout << "Counter = " << counter << std::endl;
	return 0;
}
// -> BE CAREFUL WITH SHARED MEMORY!