#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#define N 25000000
#define NUM_OF_THREADS 4

int arr[N];
std::mutex lock_sum;
std::atomic<int> sum_atomic{ 0 };
int sum_par = 0;
int sum_seq = 0;

struct Boundaries { int min; int max; };

void init_array() {
	for (int i=0; i < N; i++) {
		arr[i] = rand() % 1000;
	}
}

void compute_sum_sequential() {
	for (int i = 0; i < N; i++) {
		if (i % 2 == 0) sum_seq ^= arr[i];
	}
}

Boundaries compute_boundaries(int ind) {
	Boundaries boundaries;
	int step = N / NUM_OF_THREADS;
	boundaries.min = ind * step;
	if (ind != NUM_OF_THREADS - 1) boundaries.max = (ind + 1) * step;
	else boundaries.max = N;
	return boundaries;
}

void compute_sum_parralel(int ind) {
	int sum_par_th = 0;
	Boundaries bs = compute_boundaries(ind);
	for (int i =bs.min; i < bs.max; i++) {
		if (i % 2 == 0) {
			sum_par_th ^= arr[i];
		} 
	}
	lock_sum.lock();
	sum_par ^= sum_par_th;
	lock_sum.unlock();
}

void compute_sum_atomic(int ind) {
	std::atomic<int> sum_atomic_th{ 0 };
	Boundaries bs = compute_boundaries(ind);
	for (int i = bs.min; i < bs.max; i++) {
		if (i % 2 == 0) sum_atomic_th ^= arr[i];
	}
	sum_atomic ^= sum_atomic_th;
}


int main()
{
	srand(time(NULL));
	init_array();
	auto start = clock();
	compute_sum_sequential();
	auto stop = clock();
	auto duration = stop - start;
	std::cout << "Sequential duration: " << duration << std::endl;
	std::cout << "Sequential sum is: " << sum_seq << std::endl;

	start = clock();
	std::thread threads[NUM_OF_THREADS];
	for (int i = 0; i < NUM_OF_THREADS; i++) threads[i] = std::thread(compute_sum_parralel, i);
	for (int i = 0; i < NUM_OF_THREADS; i++) threads[i].join();
	stop = clock();
	duration = stop - start;
	std::cout << "Parallel mutex duration: " << duration << std::endl;
	std::cout << "Paralel sum is: " << sum_par << std::endl;


	start = clock();
	for (int i = 0; i < NUM_OF_THREADS; i++) threads[i] = std::thread(compute_sum_atomic, i);
	for (int i = 0; i < NUM_OF_THREADS; i++) threads[i].join();
	stop = clock();
	duration = stop - start;
	std::cout << "Parallel atomic duration: " << duration << std::endl;
	std::cout << "Atomic sum is: " << sum_atomic << std::endl;

	return 0;
}

