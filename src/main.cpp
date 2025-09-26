#include <iostream>
#include <thread>
#include <mutex>

#include "./headers/libtslog.h"

// Defines para testes iniciais
#define NUM_THREADS 5
#define NUM_LOGS_PER_THREAD 10

std::mutex coutMutex;
int main() {
    libtslog::init("log.txt");

    std::thread threads[NUM_THREADS];
    int threads_num_logs[NUM_THREADS] = {0};

    std::cout << "NUMERO DE THREADS: " << NUM_THREADS << std::endl;
    std::cout << "NUMERO DE LOGS POR THREAD: " << NUM_LOGS_PER_THREAD << std::endl;
    std::cout << std::endl;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads[i] = std::thread([i, &threads_num_logs]() {
            for (int j = 0; j < NUM_LOGS_PER_THREAD; ++j) {
                {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cout << "Thread " << i << " está gravando o log " << j << std::endl;
                }
                threads_num_logs[i]++;
                libtslog::log("Thread " + std::to_string(i) + " - Mensagem " + std::to_string(j));
            }
        });
    }
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads[i].join();
    }

    libtslog::close();
    return 0;
}