#include <iostream>
#include <vector>
#include <time.h>
#include <chrono>
#include <thread>
#include <fstream>

int SIZE = 100;
int NUM_OF_THREADS = 1;


void print_matrix(std::vector<std::vector<int>>& matrix) {
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}


//void SEQ_fill_with_random_and_set_max(std::vector<std::vector<int>>& matrix) {
//    for (int i = 0; i < SIZE; i++)
//    {
//        int max = 0;
//        for (int j = 0; j < SIZE; j++)
//        {
//            matrix[i][j] = rand() % 1000;
//            if (matrix[i][j] > max) max = matrix[i][j];
//        }
//        matrix[i][i] = max;
//    }
//}


void THREADED_fill_with_random_and_set_max(std::vector<std::vector<int>>& matrix, int thread_ind) {
    for (int i = 0; i < SIZE/NUM_OF_THREADS; i++)
    {
        int row_ind = i * NUM_OF_THREADS + thread_ind;
        int max = 0;
        for (int j = 0; j < SIZE; j++)
        {
            matrix[row_ind][j] = rand() % 1000;
            if (matrix[row_ind][j] > max) max = matrix[row_ind][j];
        }
        matrix[row_ind][row_ind] = max;
    }
    int last_row_ind = (SIZE / NUM_OF_THREADS) * NUM_OF_THREADS + thread_ind;
    if (last_row_ind < SIZE) {
        int max = 0;
        for (int j = 0; j < SIZE; j++)
        {
            matrix[last_row_ind][j] = rand() % 1000;
            if (matrix[last_row_ind][j] > max) max = matrix[last_row_ind][j];
        }
        matrix[last_row_ind][last_row_ind] = max;

    }
}


void run_computations() {
    std::vector<std::vector<int>> matrix(SIZE, std::vector<int>(SIZE, 0));
    std::vector<std::thread> threads(NUM_OF_THREADS);
    print_matrix(matrix);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OF_THREADS; i++) {
        threads[i] = (std::thread(THREADED_fill_with_random_and_set_max, std::ref(matrix), i));
    }


    //THREADED_fill_with_random_and_set_max(matrix,0);
    for (int i = 0; i < NUM_OF_THREADS; i++) {
        threads[i].join();
    }


    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    //myfile << NUM_OF_THREADS << "," << SIZE << "," << duration.count() << "\n";
    std::cout << "Number of threads: " << NUM_OF_THREADS << " Matrix size: " << SIZE << " Duration: " << duration.count() << "\n";

    print_matrix(matrix);
}




int main()
{
    srand(time(NULL));
    std::ofstream myfile;
    myfile.open("example.csv");
    myfile << "number_of_threads,matrix_size,time_passed\n";
    
    int size_coef = 1,threads_coef = 0;
    for (size_coef = 1; size_coef < 5; size_coef++) {
        SIZE = 10 * size_coef;
        for (threads_coef = 0; threads_coef < 8; threads_coef++) {
            NUM_OF_THREADS = 1 + threads_coef;
            run_computations();
        }
    }


    myfile.close();

}


