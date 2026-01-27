#include <iostream>
#include <string>
#include <time.h>  
#include <thread>
#include <mutex>


using namespace std;
/*
    1.1
    int main() {
    std::string s = "01234";
    for (unsigned int i = s.size() - 1; i >= 0; i--) {
        std::cout << s[i] << std::endl;
    }
    return 0;
}
*/


/*
1.2
// Функция вычисления факториала
long long factorial(int n) {
    long long result = 1;
    for (int i = 1; i <= n; i++) {
        result *= i;
    }
    return result;
}

int main() {
    const int ITERATIONS = 10000000; // 10 миллионов раз
    const int FACTORIAL_NUM = 10;

    cout << "Task: Calculate " << FACTORIAL_NUM << "! = " << factorial(FACTORIAL_NUM)
        << " exactly " << ITERATIONS << " times" << endl;

    // Замер времени с помощью clock()
    clock_t start = clock();

    long long total = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        total += factorial(FACTORIAL_NUM);
    }

    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;

    cout << "- Factorial of " << FACTORIAL_NUM << " is: " << factorial(FACTORIAL_NUM) << endl;
    cout << "- Total sum after " << ITERATIONS << " iterations: " << total << endl;
    cout << "- Execution time: " << seconds << " seconds" << endl;

    return 0;   
}
*/




mutex cout_mutex;

// Функция вычисления факториала
long long factorial(int n) {
    long long result = 1;
    for (int i = 1; i <= n; i++) {
        result *= i;
    }
    return result;
}

// Функция для потоков
void Func(string name) {
    long long total = 0;
    for (int i = 0; i < 5000000; i++) { // 5 миллионов раз каждый поток
        total += factorial(10);
    }
    lock_guard<mutex> lock(cout_mutex);
    cout << "Thread " << name << " finished. Total: " << total << endl;
}

int main() {
    // Последовательный запуск Func два раза
    cout << "--- Sequential execution (Func called twice) ---" << endl;
    clock_t start_seq = clock();

    Func("sequential1");
    Func("sequential2");

    clock_t end_seq = clock();
    double seq_seconds = (double)(end_seq - start_seq) / CLOCKS_PER_SEC;
    cout << "Sequential time: " << seq_seconds << " seconds" << endl << endl;

    // Параллельный запуск в двух потоках
    cout << "--- Parallel execution (two threads) ---" << endl;
    clock_t start_par = clock();

    thread thread1(Func, "t1");
    thread thread2(Func, "t2");

    thread1.join();
    thread2.join();

    clock_t end_par = clock();
    double par_seconds = (double)(end_par - start_par) / CLOCKS_PER_SEC;
    cout << "Parallel time: " << par_seconds << " seconds" << endl << endl;

    // Сравнение результатов
    cout << "Sequential time: " << seq_seconds << " seconds" << endl;
    cout << "Parallel time: " << par_seconds << " seconds" << endl;
    cout << "Speedup: " << (seq_seconds / par_seconds) << "x" << endl;

    system("pause");
    return 0;
}