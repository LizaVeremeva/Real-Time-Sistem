#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <locale.h>
#include <condition_variable>

using namespace std;

// КЛАСС СОБСТВЕННОГО МЬЮТЕКСА
class CustomMutex {
private:
    atomic<bool> locked;  // флаг блокировки

public:
    CustomMutex() : locked(false) {}

    // Метод блокировки
    void lock() {
        // Пытаемся установить флаг из false в true
        bool expected = false;
        while (!locked.compare_exchange_weak(expected, true,
            memory_order_acquire,
            memory_order_relaxed)) {
            // Если не получилось, значит мьютекс уже занят
            expected = false;
            this_thread::yield();  // отдаем квант времени другим потокам
        }
        // Успешно захватили мьютекс
    }

    // Метод разблокировки
    void unlock() {
        // Сбрасываем флаг блокировки
        locked.store(false, memory_order_release);
    }

    // Метод попытки захвата (нужен для condition_variable)
    bool try_lock() {
        bool expected = false;
        return locked.compare_exchange_weak(expected, true,
            memory_order_acquire,
            memory_order_relaxed);
    }

    // Деструктор
    ~CustomMutex() {
        unlocked();
    }

private:
    // Гарантированная разблокировка
    void unlocked() {
        locked.store(false, memory_order_release);
    }
};

// КОНСТАНТЫ 
const int SIMULATION_DAYS = 5;
const int MAX_NUGGETS_PER_FATMAN = 10000;
const int INITIAL_NUGGETS = 3000;

// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ДЛЯ СИНХРОНИЗАЦИИ
CustomMutex mtx;                        // СОБСТВЕННЫЙ МЬЮТЕКС
// Простая реализация condition variable на основе busy-wait
// (так как стандартная требует try_lock с определенной семантикой)
bool cook_can_work = true;
bool fatmen_can_eat = false;

// Атомарные переменные для результатов
atomic<bool> running(true);
atomic<bool> cook_fired(false);
atomic<bool> cook_no_salary(false);
atomic<bool> cook_quit(false);

// Общие данные, защищаемые мьютексом
int dishes[3] = { INITIAL_NUGGETS, INITIAL_NUGGETS, INITIAL_NUGGETS };
int eaten[3] = { 0, 0, 0 };
bool alive[3] = { true, true, true };
int current_fatman = 0;
int day_counter = 0;
int cycles_counter = 0;

// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ СИНХРОНИЗАЦИИ
void wait_for_condition(bool& condition) {
    while (running && !condition) {
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void notify_all() {
    // Просто сбрасываем состояние для пробуждения
}

// ФУНКЦИЯ ТОЛСТЯКА
void fatman_func(int id, int gluttony) {
    while (running && alive[id]) {
        // Ждем своей очереди
        while (running && alive[id]) {
            {
                lock_guard<CustomMutex> lock(mtx);
                if (fatmen_can_eat && current_fatman == id && alive[id]) {
                    break;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(10));
        }

        if (!running || !alive[id]) break;

        {
            lock_guard<CustomMutex> lock(mtx);

            // Едим наггетсы
            if (dishes[id] >= gluttony) {
                dishes[id] -= gluttony;
                eaten[id] += gluttony;

                if (eaten[id] >= MAX_NUGGETS_PER_FATMAN) {
                    alive[id] = false;
                    cout << "  Толстяк " << (id + 1) << " съел " << eaten[id]
                        << " и самоуничтожился!" << endl;

                    // Если все толстяки мертвы
                    if (!alive[0] && !alive[1] && !alive[2]) {
                        cook_no_salary = true;
                        running = false;
                        cout << "\n=== Все толстяки самоуничтожились! Кук без зарплаты! ===" << endl;
                        return;
                    }
                }
            }
            else if (dishes[id] > 0) {
                cout << "  Толстяк " << (id + 1) << " хочет " << gluttony
                    << ", но только " << dishes[id] << "! Кука увольняют!" << endl;
                cook_fired = true;
                running = false;
                return;
            }
            else {
                cout << "  Тарелка " << (id + 1) << " пуста! Кука увольняют!" << endl;
                cook_fired = true;
                running = false;
                return;
            }

            // Переход к следующему толстяку
            current_fatman = (current_fatman + 1) % 3;

            // Если все толстяки поели, передаем очередь повару
            if (current_fatman == 0) {
                fatmen_can_eat = false;
                cook_can_work = true;
                cycles_counter++;
            }
        }

        // Небольшая пауза
        this_thread::sleep_for(chrono::milliseconds(5));
    }
}

// ФУНКЦИЯ ПОВАРА (КУКА) - УПРОЩЕННАЯ ВЕРСИЯ
void cook_func(int efficiency, int gluttony, int cycles_per_day) {
    day_counter = 0;
    cycles_counter = 0;

    while (running && day_counter < SIMULATION_DAYS) {
        // Ждем, пока можно готовить
        while (running) {
            {
                lock_guard<CustomMutex> lock(mtx);
                if (cook_can_work) {
                    break;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(10));
        }

        if (!running) break;

        {
            lock_guard<CustomMutex> lock(mtx);

            // Готовим наггетсы
            dishes[0] += efficiency;
            dishes[1] += efficiency;
            dishes[2] += efficiency;

            // Передаем очередь толстякам
            cook_can_work = false;
            fatmen_can_eat = true;
            current_fatman = 0;

            // Проверка на завершение дня
            if (cycles_counter >= cycles_per_day) {
                day_counter++;
                cycles_counter = 0;

                if (day_counter <= SIMULATION_DAYS) {
                    cout << "\nДень " << day_counter << ":" << endl;
                    cout << "  Тарелки: " << dishes[0] << ", " << dishes[1] << ", " << dishes[2] << endl;
                    cout << "  Съедено: " << eaten[0] << ", " << eaten[1] << ", " << eaten[2] << endl;

                    bool all_dead = true;
                    for (int i = 0; i < 3; i++) {
                        if (!alive[i]) {
                            cout << "  Толстяк " << (i + 1) << " - самоуничтожился!" << endl;
                        }
                        else {
                            all_dead = false;
                        }
                    }

                    if (all_dead) {
                        cook_no_salary = true;
                        running = false;
                        cout << "\n=== Все толстяки самоуничтожились! Кук без зарплаты! ===" << endl;
                        return;
                    }
                }
            }

            // Проверка на завершение 5 дней
            if (day_counter >= SIMULATION_DAYS) {
                cook_quit = true;
                running = false;
                cout << "\n=== Прошло " << SIMULATION_DAYS << " дней! Кук увольняется ===" << endl;
                return;
            }
        }

        // Имитация работы повара
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

// ФУНКЦИЯ СБРОСА ГЛОБАЛЬНЫХ ПЕРЕМЕННЫХ
void reset_globals() {
    lock_guard<CustomMutex> lock(mtx);

    dishes[0] = dishes[1] = dishes[2] = INITIAL_NUGGETS;
    eaten[0] = eaten[1] = eaten[2] = 0;
    alive[0] = alive[1] = alive[2] = true;

    cook_can_work = true;
    fatmen_can_eat = false;
    current_fatman = 0;
    day_counter = 0;
    cycles_counter = 0;

    running = true;
    cook_fired = false;
    cook_no_salary = false;
    cook_quit = false;
}

// ФУНКЦИЯ ЗАПУСКА ЭКСПЕРИМЕНТА
void run_experiment(int gluttony, int efficiency, int cycles_per_day,
    const string& name, const string& expected) {
    reset_globals();

    cout << "\n" << string(50, '=') << endl;
    cout << name << endl;
    cout << string(50, '=') << endl;
    cout << "gluttony = " << gluttony << ", efficiency = " << efficiency << endl;
    cout << "циклов в день = " << cycles_per_day << endl;
    cout << "Ожидается: " << expected << endl;

    thread cook(cook_func, efficiency, gluttony, cycles_per_day);
    thread f1(fatman_func, 0, gluttony);
    thread f2(fatman_func, 1, gluttony);
    thread f3(fatman_func, 2, gluttony);

    auto start = chrono::steady_clock::now();
    while (running) {
        this_thread::sleep_for(chrono::milliseconds(100));

        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(now - start).count() > 5) {
            running = false;
            cout << "\n=== ТАЙМАУТ ===" << endl;
            break;
        }
    }

    cook.join();
    f1.join();
    f2.join();
    f3.join();

    cout << "\nИтог: ";
    if (cook_fired) cout << "Кука уволили!";
    else if (cook_no_salary) cout << "Кук без зарплаты!";
    else if (cook_quit) cout << "Кук уволился сам!";
    else cout << "Таймаут/ошибка";
    cout << endl;

    cout << "Съедено: " << eaten[0] << ", " << eaten[1] << ", " << eaten[2] << endl;

    bool success = false;
    if (expected == "Кука уволили" && cook_fired) success = true;
    if (expected == "Кук без зарплаты" && cook_no_salary) success = true;
    if (expected == "Кук уволился сам" && cook_quit) success = true;

    cout << (success ? "✓ УСПЕХ" : "✗ НЕУДАЧА") << endl;

    this_thread::sleep_for(chrono::seconds(1));
}

// ОСНОВНАЯ ФУНКЦИЯ
int main() {
    setlocale(LC_ALL, "Russian");

    // Эксперимент 1: Кука увольняют
    run_experiment(180, 15, 5,
        "ЭКСПЕРИМЕНТ 1: КУКА УВОЛИЛИ",
        "Кука уволили");

    // Эксперимент 2: Кук без зарплаты
    run_experiment(180, 210, 12,
        "ЭКСПЕРИМЕНТ 2: КУК БЕЗ ЗАРПЛАТЫ",
        "Кук без зарплаты");

    // Эксперимент 3: Кук увольняется сам
    run_experiment(10, 100, 2,
        "ЭКСПЕРИМЕНТ 3: КУК УВОЛИЛСЯ САМ",
        "Кук уволился сам");

    return 0;
}