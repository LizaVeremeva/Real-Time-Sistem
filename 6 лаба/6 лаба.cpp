#include <iostream>
#include <coroutine>
#include <optional>
#include <chrono>
#include <thread>
#include <string>

/*
1.1
// Структура, которую будет возвращать корутина
struct CoroutineResult {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        int current_value;

        CoroutineResult get_return_object() {
            return CoroutineResult{ handle_type::from_promise(*this) };
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}

        std::suspend_always yield_value(int value) {
            current_value = value;
            return {};
        }
    };

    handle_type coro_handle;

    // Конструктор
    explicit CoroutineResult(handle_type h) : coro_handle(h) {}

    // Деструктор
    ~CoroutineResult() {
        if (coro_handle) {
            coro_handle.destroy();
        }
    }

    // Получить текущее значение
    int value() const {
        return coro_handle.promise().current_value;
    }

    // Продолжить выполнение
    bool resume() {
        if (!coro_handle.done()) {
            coro_handle.resume();
        }
        return !coro_handle.done();
    }

    // Проверить, завершена ли
    bool done() const {
        return coro_handle.done();
    }
};

// Корутина, которая генерирует числа
CoroutineResult generate_numbers() {
    int my_number = 72;

    std::cout << "Генерация чисел на основе: " << my_number << std::endl;
    co_yield my_number;           // 1-е число
    co_yield my_number * 2;       // 2-е число  
    co_yield my_number + 10;      // 3-е число
    co_yield my_number - 5;       // 4-е число
    co_yield my_number * my_number; // 5-е число
}

int main() {
    setlocale(LC_ALL, "Russian");

    // Создаем корутину
    CoroutineResult numbers = generate_numbers();

    std::cout << "Сгенерированные числа:" << std::endl;

    // Получаем все значения из корутины
    int counter = 1;
    while (numbers.resume()) {
        std::cout << "Число " << counter << ": " << numbers.value() << std::endl;
        counter++;
    }

    std::cout << std::endl;
    std::cout << "Корутина завершила выполнение." << std::endl;

    return 0;
}
*/

//1.2
// Структура promise для прогресс-бара
struct progress_promise {
    int current_value = 0;

    auto get_return_object() {
        return std::coroutine_handle<progress_promise>::from_promise(*this);
    }

    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}

    std::suspend_always yield_value(int value) {
        current_value = value;
        return {};
    }
};

// Структура задачи
struct task {
    std::coroutine_handle<progress_promise> handle;

    task(std::coroutine_handle<progress_promise> h) : handle(h) {}

    ~task() {
        if (handle) handle.destroy();
    }

    void resume() {
        handle.resume();
    }

    bool done() const {
        return handle.done();
    }

    int get_value() const {
        return handle.promise().current_value;
    }
};

// корутина принимает const std::string&
namespace std {
    template<>
    struct coroutine_traits<task, const std::string&> {
        using promise_type = ::progress_promise;
    };
}

// Функция для отрисовки прогресс-бара с именем
void draw_progress_bar(int progress, int total, const std::string& name) {
    const int bar_width = 50;
    float percentage = static_cast<float>(progress) / total;
    int filled_width = static_cast<int>(bar_width * percentage);

    std::cout << "[";

    // Заполняем прогресс-бар символами из имени
    for (int i = 0; i < filled_width; i++) {
        // Циклически используем символы имени
        int name_index = i % name.length();
        std::cout << name[name_index];
    }

    std::cout << "] " << int(percentage * 100.0) << "%\r";
    std::cout.flush();
}

// Корутина, которая имитирует работу
task simulate_work(const std::string& worker_name) {
    const int total_steps = 100;

    for (int i = 1; i <= total_steps; i++) {
        // Имитация работы (задержка)
        std::this_thread::sleep_for(std::chrono::milliseconds(30));

        // Возвращаем текущий прогресс
        co_yield i;

        // Отрисовываем прогресс-бар
        draw_progress_bar(i, total_steps, worker_name);
    }
    std::cout << std::endl;
}

int main() {
    setlocale(LC_ALL, "Russian");

    std::string my_name = "LIZA";

    std::cout << "Прогресс выполнения задачи:\n";
    std::cout << "Имя в прогресс-баре: " << my_name << "\n\n";
    std::cout << "Начинаем выполнение...\n";

    // Запускаем корутину
    auto coro = simulate_work(my_name);

    // Пока корутина не завершила работу
    while (!coro.done()) {
        coro.resume();
    }

    std::cout << "\n\nЗадача ВЫПОЛНЕНА на 100%\n";
    std::cout << "Прогресс-бар полностью заполнен именем: " << my_name << "\n";

    return 0;
}