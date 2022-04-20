#include <iostream>
#include <future>
#include <mutex>
#include <string>
#include <vector>

#define ARRAY_SIZE 20



std::vector<int> process_array1() {
    std::vector<int> arr(ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = rand() % 61;
        arr[i] *= 4;
    }
    std::sort(arr.begin(), arr.end());
    return arr;
}

std::vector<int> process_array2() {
    int arr[ARRAY_SIZE];
    std::vector<int> processed_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = rand() % 666;
        if (arr[i] % 2 == 0) processed_arr.push_back(arr[i]);
    }
    std::sort(processed_arr.begin(), processed_arr.end());
    return processed_arr;
}

std::vector<int> process_array3() {
    int arr[ARRAY_SIZE];
    float mean = 0;
    std::vector<int> processed_arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = rand() % 228;
        mean += arr[i];
    }
    mean /= ARRAY_SIZE;
    int lb = 0.4 * mean;
    int hb = 1.4 * mean;
    for (int i = 0; i < ARRAY_SIZE; i++) if (arr[i]<hb && arr[i]>lb) processed_arr.push_back(arr[i]);
    std::sort(processed_arr.begin(), processed_arr.end());
    return processed_arr;
}

void print_vec(std::vector<int>& vec) {
    for (auto i : vec)
        std::cout << i << ' ';
}

std::vector<int> merge_arrays(
    std::vector<int>& const vec1,
    std::vector<int>& const vec2,
    std::vector<int>& const vec3) {

    int m = vec1.size();
    int n = vec2.size();

    std::vector<int> vec;
    vec.reserve(m + n);

    int i = 0, j = 0;
    while (i < m && j < n) {
        if (vec1[i] <= vec2[j])
        { 
            bool is_in_vec3 = std::binary_search(vec3.begin(), vec3.end(), vec1[i]);
            if (is_in_vec3) i++;
            else vec.push_back(vec1[i++]);
        }
        else{
            bool is_in_vec3 = std::binary_search(vec3.begin(), vec3.end(), vec2[j]);
            if (is_in_vec3) j++;
            else vec.push_back(vec2[j++]);
        }
    }

    while (i < m)
    { 
        if (std::binary_search(vec3.begin(), vec3.end(), vec1[i])) i++;
        else vec.push_back(vec1[i++]);
    }
    while (j < n){
        if (std::binary_search(vec3.begin(), vec3.end(), vec1[j])) j++;
        else vec.push_back(vec2[j++]);
    }
    return vec;
    
}


int main()
{
    std::future<std::vector<int>> res1 = std::async(std::launch::async, process_array1);
    std::future<std::vector<int>> res2 = std::async(std::launch::async, process_array2);
    std::future<std::vector<int>> res3 = std::async(std::launch::async, process_array3);

    auto vec1 = res1.get();
    auto vec2 = res2.get();
    auto vec3 = res3.get();

    std::cout << "\nArr 1 processed: ";
    print_vec(vec1);
    std::cout << "\nArr 2 processed: ";
    print_vec(vec2);
    std::cout << "\nArr 3 processed: ";
    print_vec(vec3);
    std::vector<int> FINAL_VEC = merge_arrays(vec1, vec2, vec3);
    std::cout << "\nFinal array: ";
    print_vec(FINAL_VEC);
}

