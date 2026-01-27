#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <locale.h> 
#include <chrono> 

// Общие монеты
int coins = 10;
int c = coins;

// Монеты воров
int Bob_coins = 0;
int Tom_coins = 0;

// Мьютекс для синхронизации
std::mutex m;

void coin_sharing(std::string name, int& thief_coins, int& companion_coins) {
    while (true) {
        m.lock(); // Захватываем мьютекс

        // Условия, при которых вор НЕ МОЖЕТ взять монету:
        // 1. Монет больше нет
        // 2. У этого вора уже БОЛЬШЕ монет, чем у напарника
        if (coins == 0 || thief_coins > companion_coins) {
            m.unlock();
            break;
        }

        // Пробуем взять монету
        coins--;
        thief_coins++;


        if (coins == 0 && thief_coins > companion_coins) {
            // Возвращаем монету покойнику
            coins++;
            thief_coins--;
            m.unlock();
            break;
        }

        std::cout << name << " взял монету. "
            << name << ": " << thief_coins << ", "
            << "Напарник: " << companion_coins << ", "
            << "Осталось: " << coins << std::endl;

        m.unlock();

        // Задержка для наглядности
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main() {
    setlocale(LC_ALL, "Russian");

    std::cout << "Начало дележа монет!" << std::endl;
    std::cout << "Всего монет: " << coins << std::endl << std::endl;

    // Создадим потоки для воров
    std::thread Bob(coin_sharing, "Боб", std::ref(Bob_coins), std::ref(Tom_coins));
    std::thread Tom(coin_sharing, "Том", std::ref(Tom_coins), std::ref(Bob_coins));

    Bob.join();
    Tom.join();

    // Вывод итогов
    std::cout << "\nИтог дележа:" << std::endl;
    std::cout << "Боб: " << Bob_coins << " монет" << std::endl;
    std::cout << "Том: " << Tom_coins << " монет" << std::endl;
    std::cout << "Покойник: " << (c - Bob_coins - Tom_coins) << " монет" << std::endl;
    std::cout << "Всего: " << (Bob_coins + Tom_coins + (c - Bob_coins - Tom_coins)) << std::endl;

    system("pause");
    return 0;
}