#include "queue.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>


int main(int argc, char** argv) {
   if (argc < 2) {
       std::cout << "Use:\n";
       std::cout << "./consumer <type>\n";
       return 1;
   }

   uint32_t type = std::stoi(argv[1]);
   ConsumerNode consumer("/my_queue");

   while (true) {
       std::string msg;

       if (!consumer.receive(type, msg)) {
           std::this_thread::sleep_for(std::chrono::milliseconds(50));
           continue;
       }

       // string
       if (type == 1) {
           std::cout << "String: " << msg << std::endl;
       }
       // int
       else if (type == 2) {
           if (msg.size() != sizeof(int)) {
               std::cerr << "Invalid int size: " << msg.size() << std::endl;
               continue;
           }

           int value;
           memcpy(&value, msg.data(), sizeof(value));

           std::cout << "Int: " << value << std::endl;
       }
       // double
       else if (type == 3) {
           if (msg.size() != sizeof(double)) {
               std::cerr << "Invalid double size: " << msg.size() << std::endl;
               continue;
           }

           double value;
           memcpy(&value, msg.data(), sizeof(value));

           std::cout << "Double: " << value << std::endl;
       }
       else {
           std::cout << "Unknown type: " << type
                     << " raw size = " << msg.size() << std::endl;
       }
   }
}
