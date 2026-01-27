#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <chrono>

// Глобальный мьютекс
//std::mutex m;

// Функция, выполняемая в потоке
void Func(std::string name) {
    long double i = 0;

    // Фиксируем время начала
    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::seconds(1); // Время окончания (через 1 секунду)

    // Цикл, работающий примерно 1 секунду
    while (std::chrono::steady_clock::now() < end) {
        i += 1e-9; // Прибавляем 10^(-9)
    }
    // Вывод результата с защитой мьютексом
    //m.lock();
    std::cout << name << ": " << i << std::endl;
    //m.unlock();
}

int main() {
    // Создаём три потока
    std::thread thread1(Func, "t1");
    std::thread thread2(Func, "t2");
    std::thread thread3(Func, "t3");

    // Ожидаем завершения всех потоков
    thread1.join();
    thread2.join();
    thread3.join();

    system("pause");
    return 0;
}