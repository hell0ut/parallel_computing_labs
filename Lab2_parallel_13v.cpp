#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>


int process_num = 0;
int process_killed = 0;
const int DELAY = 1000;
int NUM_OF_CPU = 2;
int free_processes = NUM_OF_CPU;

std::mutex console;
std::mutex queue;
std::mutex counter_m;

std::condition_variable new_proc;


void threaded_cout(std::string text) {
    std::lock_guard<std::mutex> con(console);
    std::cout << text << std::endl;
}

class Storage {
public:
    std::queue<int> processes;
    int processes_limit;
    bool has_cur_process=false;
    int cur_process;

    Storage(int processes_limit_) : processes_limit(processes_limit_) {  };
    void add_process(int process) {
        std::unique_lock<std::mutex> ul_queue(queue);
        if (processes.size() + 1 > processes_limit) {
            process_killed++;
            float percentage = float(process_killed) / float(process_num) * 100.00;
            threaded_cout("Process " + std::to_string(process) + " has been killed due to reaching a queue size limit. Percentage of killed processes is"+std::to_string(percentage) + " %");

        }
        else if (free_processes > 0) {
            threaded_cout("There are free CPUs, giving process " + std::to_string(process) + " to CPU");
            cur_process = process;
            has_cur_process = true;
            new_proc.notify_all();
        }
        else{
        processes.push(process);
        threaded_cout(std::to_string(processes.size())+ " processes are in queue");
        new_proc.notify_all();
        }
    }

    int get_process(int cpu_id) {
        std::unique_lock<std::mutex> ul_queue(queue);
        while (processes.empty() && !has_cur_process) {
            threaded_cout("No processes in queue. CPU " + std::to_string(cpu_id) + " is waiting. ");
            new_proc.wait(ul_queue);
        }
        if (has_cur_process) {
            has_cur_process = false;
            return cur_process;
        }
        int process = processes.back();
        processes.pop();
        return process;
    }

};


class ProcessGenerator {
public:
    Storage& st;
    std::thread& th;

    ProcessGenerator(std::thread& th_, Storage& st_) : th(th_), st(st_) {  };

    void run() {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (DELAY-600)));
            process_num++;
            threaded_cout( "Proccess " + std::to_string(process_num) + " has been generated");
            st.add_process(process_num);
        }
    }

    void start() {
        th = std::thread(&ProcessGenerator::run, this);
    }

};



class CPU {
public:
    std::thread& th;
    Storage& st;
    int cpu_id;
    bool busy;

    void run() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % DELAY));
            int cur_process = st.get_process(cpu_id);
            counter_m.lock();
            free_processes--;
            counter_m.unlock();
            threaded_cout("Proccess " + std::to_string(cur_process) + " is being worked on by CPU " + std::to_string(cpu_id));
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 700));
            threaded_cout("Proccess " + std::to_string(cur_process) + " has been processed by CPU " + std::to_string(cpu_id));
            counter_m.lock();
            free_processes++;
            counter_m.unlock();
        }
    }
    
    CPU(std::thread& th_, Storage& st_,int cpu_id_) : th(th_), st(st_),cpu_id(cpu_id_) {  };

    void start() {
        th = std::thread(&CPU::run, this);
    }


};



int main()
{
    srand(time(NULL));
    
    Storage storage(4);

    //std::vector<std::thread> threads(NUM_OF_CPU+1);
    //std::vector<CPU> CPUs;
    std::thread pg_th;
    std::thread cp1_th;
    std::thread cp2_th;
    
    ProcessGenerator PG(pg_th, storage);
    CPU cp1(cp1_th, storage, 1);
    CPU cp2(cp2_th, storage, 2);


    //for (int i=0; i < NUM_OF_CPU; i++) {
    //    CPUs.push_back(CPU(threads[i], storage, i + 1));
    //    CPUs[i].start();
    //}
    PG.start();
    cp1.start();
    cp2.start();

    pg_th.join();
    cp1_th.join();
    cp2_th.join();

    

    //for (int i = 0; i < NUM_OF_CPU + 1; i++) {
    //    threads[i].join();
    //}

    return 0;
}


