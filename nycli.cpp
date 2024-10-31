#include <iostream>
#include <wiringSerial.h>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <cstring>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>

// 函数声明
std::vector<std::string> listSerialPorts();

int main() {
    // 列出所有可用的串口设备
    std::vector<std::string> ports = listSerialPorts();
    if (ports.empty()) {
        std::cerr << "No serial ports found." << std::endl;
        return 1;
    }

    // 显示可用的串口设备
    std::cout << "Available serial ports:" << std::endl;
    for (size_t i = 0; i < ports.size(); ++i) {
        std::cout << i + 1 << ". " << ports[i] << std::endl;
    }

    // 选择一个串口设备
    int portIndex;
    std::cout << "Choose a serial port by entering its number: ";
    std::cin >> portIndex;
    if (portIndex < 1 || portIndex > ports.size()) {
        std::cerr << "Invalid port number." << std::endl;
        return 1;
    }

    // 获取选择的串口设备
    std::string selectedPort = ports[portIndex - 1];

    // 初始化串口通信
    int fd = serialOpen(selectedPort.c_str(), 115200);
    if (fd == -1) {
        std::cerr << "Failed to open serial port: " << selectedPort << std::endl;
        return 1;
    }

    // 设置为非阻塞模式
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

    std::cout << "Connected to serial port: " << selectedPort << std::endl;
    std::cout << "Enter commands and press Enter to send them. Type 'exit' to quit." << std::endl;

    // 循环接收和发送数据
    char buffer[256];
    std::string input;
    while (true) {
        // 清空缓冲区
        buffer[0] = '\0';

        // 接收串口数据
        ssize_t bytesRead = serialDataAvail(fd);
        if (bytesRead > 0) {
            for (ssize_t i = 0; i < bytesRead; ++i) {
                char c = serialGetchar(fd);
                buffer[i] = c;
            }
            buffer[bytesRead] = '\0'; // 确保字符串以null结尾
            std::cout << "Received: " << buffer << std::endl;
        }

        // 读取用户输入
        std::cout << "> ";
        std::getline(std::cin, input); // 读取一行输入，包括换行符

        // 发送用户输入
        if (input == "exit") {
            break;
        }
        serialPuts(fd, input.c_str());

        // 等待一段时间以避免快速连续发送
        usleep(100000); // 0.1 seconds
    }

    // 关闭串口
    serialClose(fd);
    std::cout << "Exiting program." << std::endl;

    return 0;
}

// 列出所有可用的串口设备
std::vector<std::string> listSerialPorts() {
    std::vector<std::string> ports;
    const char* prefixes[] = {"/dev/ttyUSB", "/dev/ttyAMA", "/dev/ttyACM"};
    bool found = false;

    for (const char* prefix : prefixes) {
        for (int i = 0;; ++i) {
            std::string portName = std::string(prefix) + std::to_string(i);
            int fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
            if (fd < 0) {
                break; // No more devices
            }
            close(fd);
            ports.push_back(portName);
            found = true;
        }
    }

    if (!found) {
        std::cerr << "No serial ports found." << std::endl;
    }

    return ports;
}