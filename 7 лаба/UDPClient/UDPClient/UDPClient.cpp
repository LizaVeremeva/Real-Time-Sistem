#include <iostream>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void main() {
    // Инициализация WINSOCK
    WSADATA data;
    int wsOk = WSAStartup(MAKEWORD(2, 2), &data);
    if (wsOk != 0) {
        cout << "Can't start Winsock! " << wsOk;
        return;
    }

    // Создание hint сервера
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    // Создание UDP сокета
    SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);

    string msg = "";
    while (true) {
        msg.clear();
        cout << "> ";
        cin >> msg; // читаем из консоли (до пробела!)

        if (msg.empty()) break;

        // Отправляем сообщение
        int sendOk = sendto(out, msg.c_str(), msg.size() + 1, 0,
            (sockaddr*)&server, sizeof(server));
        if (sendOk == SOCKET_ERROR) {
            cout << "That didn't work! " << WSAGetLastError() << endl;
        }
    }

    // Закрываем сокет
    closesocket(out);
    WSACleanup();
}