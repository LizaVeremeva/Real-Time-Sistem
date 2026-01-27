#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {

    setlocale(LC_ALL, "Russian");
    cout << "=== TCP КЛИЕНТ ===\n";
    cout << "Команды:\n";
    cout << "  !exit - нормальный выход\n";
    cout << "  !shutdown - экстренный выход\n";
    cout << "  time - время сервера\n";
    cout << "  любой текст - эхо\n\n";
    
    // Инициализация WinSock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    
    int wsResult = WSAStartup(ver, &wsData);
    if (wsResult != 0) {
        cerr << "Ошибка! Код: " << wsResult << endl;
        return 1;
    }
    
    // Создание TCP сокета
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Не создал сокет! Код: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    
    // Настройка сервера
    string serverIP = "127.0.0.1";
    int port = 55000;
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
    
    cout << "Подключаюсь к " << serverIP << ":" << port << "...\n";
    
    // Подключение
    int connResult = connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (connResult == SOCKET_ERROR) {
        cerr << "Не подключился! Код: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    
    cout << "Успешно!\n";
    cout << "Вводи сообщения:\n\n";
    
    char buf[4096];
    
    // Главный цикл
    while (true) {
        string userInput;
        cout << "> ";
        getline(cin, userInput);
        
        if (userInput.empty()) {
            cout << "Введи что-нибудь!\n";
            continue;
        }
        
        // Отправка
        int sendResult = send(clientSocket, userInput.c_str(), userInput.size() + 1, 0);
        if (sendResult == SOCKET_ERROR) {
            cerr << "Не отправил! Код: " << WSAGetLastError() << endl;
            break;
        }
        
        cout << "Отправил (" << sendResult << " байт)\n";
        
        // Выход по спецсимволам
        if (userInput == "!exit" || userInput == "!shutdown") {
            cout << "Завершаю...\n";
            
            // Ждем ответ сервера
            ZeroMemory(buf, 4096);
            int bytesReceived = recv(clientSocket, buf, 4096, 0);
            if (bytesReceived > 0) {
                cout << "Сервер: " << buf << endl;
            }
            break;
        }
        
        // Получение ответа
        ZeroMemory(buf, 4096);
        
        // Таймаут 5 секунд
        timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, 
                  (const char*)&tv, sizeof(tv));
        
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        
        if (bytesReceived > 0) {
            cout << "Сервер: " << buf << endl;
        } else if (bytesReceived == 0) {
            cout << "Сервер отключился\n";
            break;
        } else {
            if (WSAGetLastError() == WSAETIMEDOUT) {
                cout << "Таймаут 5 сек\n";
            } else {
                cerr << "Ошибка приема! Код: " << WSAGetLastError() << endl;
                break;
            }
        }
        
        cout << "----------------------------\n";
        this_thread::sleep_for(chrono::milliseconds(50));
    }
    
    // Очистка
    closesocket(clientSocket);
    WSACleanup();
    
    cout << "\nКлиент завершил\n";
    
    return 0;
}