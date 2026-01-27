#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <chrono>
#include <ctime>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {

    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");
    cout << "=== TCP СЕРВЕР ===\n";
    cout << "Условия выхода:\n";
    cout << "1. Получение '!exit' от клиента\n";
    cout << "2. Бездействие 30 секунд\n";
    cout << "3. Получение '!shutdown'\n\n";
    
    // Инициализация WinSock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    
    int wsOk = WSAStartup(ver, &wsData);
    if (wsOk != 0) {
        cerr << "Ошибка WinSock! Код: " << wsOk << endl;
        return 1;
    }
    
    // Создание TCP сокета (SOCK_STREAM!)
    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) {
        cerr << "Не создал сокет! Код: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    
    // Настройка адреса
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(55000);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;
    
    // Привязка
    if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        cerr << "Не привязал! Код: " << WSAGetLastError() << endl;
        closesocket(listening);
        WSACleanup();
        return 1;
    }
    
    // Слушаем
    if (listen(listening, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Не слушаю! Код: " << WSAGetLastError() << endl;
        closesocket(listening);
        WSACleanup();
        return 1;
    }
    
    cout << "Сервер на порту 55000\n";
    cout << "Жду клиента...\n\n";
    
    // Принимаем клиента
    sockaddr_in client;
    int clientSize = sizeof(client);
    
    SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Не принял! Код: " << WSAGetLastError() << endl;
        closesocket(listening);
        WSACleanup();
        return 1;
    }
    
    // Инфа о клиенте
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    
    ZeroMemory(host, NI_MAXHOST);
    ZeroMemory(service, NI_MAXSERV);
    
    if (getnameinfo((sockaddr*)&client, sizeof(client), 
                    host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
        cout << host << " подключен на порту " << service << endl;
    } else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " подключен на порту " << ntohs(client.sin_port) << endl;
    }
    
    closesocket(listening);
    
    // Таймаут 30 секунд
    DWORD timeout = 30000;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, 
               (const char*)&timeout, sizeof(timeout));
    
    auto lastActivity = chrono::steady_clock::now();
    bool activeConnection = true;
    int messageCount = 0;
    
    char buf[4096];
    
    cout << "\nЖду сообщений...\n";
    cout << "----------------------------------------\n";
    
    // Главный цикл
    while (activeConnection) {
        ZeroMemory(buf, 4096);
        
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        
        // Проверка таймаута
        auto now = chrono::steady_clock::now();
        auto idleTime = chrono::duration_cast<chrono::seconds>(now - lastActivity);
        
        if (idleTime.count() >= 30) {
            cout << "\nТаймаут! 30 секунд бездействия.\n";
            string timeoutMsg = "Сервер: разрыв по таймауту";
            send(clientSocket, timeoutMsg.c_str(), timeoutMsg.size() + 1, 0);
            break;
        }
        
        if (bytesReceived == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error == WSAETIMEDOUT) {
                cout << "Таймаут ожидания\n";
                continue;
            } else {
                cerr << "Ошибка! Код: " << error << endl;
                break;
            }
        }
        
        if (bytesReceived == 0) {
            cout << "Клиент отключился\n";
            break;
        }
        
        lastActivity = chrono::steady_clock::now();
        messageCount++;
        
        string clientMessage = string(buf, 0, bytesReceived);
        cout << "КЛИЕНТ: " << clientMessage << endl;
        
        // Проверка условий выхода
        if (clientMessage == "!exit") {
            cout << "Получена команда !exit\n";
            string response = "Сервер: завершение по команде !exit";
            send(clientSocket, response.c_str(), response.size() + 1, 0);
            break;
        }
        else if (clientMessage == "!shutdown") {
            cout << "Экстренное завершение !shutdown\n";
            string response = "Сервер: ЭКСТРЕННОЕ завершение";
            send(clientSocket, response.c_str(), response.size() + 1, 0);
            break;
        }
        
        // Отправка ответа
        string response = "Сервер получил: " + clientMessage;
        int sendResult = send(clientSocket, response.c_str(), response.size() + 1, 0);
        if (sendResult == SOCKET_ERROR) {
            cerr << "Не отправил ответ! Код: " << WSAGetLastError() << endl;
            break;
        }
        
        cout << "Отправил ответ\n";
        cout << "----------------------------\n";
    }
    
    cout << "\n=== ИТОГИ ===\n";
    cout << "Сообщений: " << messageCount << endl;
    
    closesocket(clientSocket);
    WSACleanup();
    
    cout << "\nСервер завершил\n";
    
    return 0;
}