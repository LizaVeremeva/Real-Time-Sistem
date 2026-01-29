#include <iostream>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void main() {
    // Инициализация WINSOCK
    WSADATA data;
    WORD version = MAKEWORD(2, 2);
    int wsOk = WSAStartup(version, &data);
    if (wsOk != 0) {
        cout << "Can't start Winsock! " << wsOk;
        return;
    }

    // Создание UDP сокета
    SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);

    // Настройка адреса сервера
    sockaddr_in serverHint;
    serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(54000);

    // Привязка сокета
    if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
        cout << "Can't bind socket! " << WSAGetLastError() << endl;
        return;
    }

    sockaddr_in client;
    int clientLength = sizeof(client);
    char buf[1024];

    // Запускаем жизненный цикл
    while (true) {
        ZeroMemory(&client, clientLength);
        ZeroMemory(buf, 1024);

        // Ждем сообщение
        int bytesIn = recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength);
        if (bytesIn == SOCKET_ERROR) {
            cout << "Error receiving from client " << WSAGetLastError() << endl;
            continue;
        }

        char clientIp[256];
        ZeroMemory(clientIp, 256);

        // Конвертируем адрес в строку
        inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
        // Показываем сообщение
        cout << "Message recv from " << clientIp << " : " << buf << endl;
    }

    // Закрываем socket
    closesocket(in);
    // Отключаем winsock
    WSACleanup();
}