#include <iostream>
#include <exception>
#include <windows.h>

int main() {
    try {
        std::cout << "=== SMS Service Debug Mode ===" << std::endl;
        std::cout << "Step 1: Starting..." << std::endl;
        std::cout.flush();
        
        std::cout << "Step 2: Initializing Windows Sockets..." << std::endl;
        std::cout.flush();
        
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return 1;
        }
        std::cout << "Winsock initialized successfully" << std::endl;
        std::cout.flush();
        
        std::cout << "Step 3: Loading MySQL..." << std::endl;
        std::cout.flush();
        
        HMODULE hMysql = LoadLibrary("libmysql.dll");
        if (!hMysql) {
            std::cerr << "Failed to load libmysql.dll: " << GetLastError() << std::endl;
            return 1;
        }
        std::cout << "MySQL DLL loaded successfully" << std::endl;
        std::cout.flush();
        
        std::cout << "\nAll basic checks passed!" << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        
        FreeLibrary(hMysql);
        WSACleanup();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        std::cin.get();
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception!" << std::endl;
        std::cin.get();
        return 1;
    }
    
    return 0;
}
