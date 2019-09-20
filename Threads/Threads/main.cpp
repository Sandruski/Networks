#include <thread>
#include <iostream>
#include <stdio.h>
#include <Windows.h>

void thread_function(int arg, int arg2)
{
	Sleep(1000);
	std::cout << "Arg1: " << arg << std::endl;
	std::cout << "Arg2: " << arg2 << std::endl;
	Sleep(1000);
}

// main thread
int main()
{
	std::thread t = std::thread(thread_function, 30, 40);

	t.join(); // waits until thread t finishes

	system("pause");
	return 0;
}