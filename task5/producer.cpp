#include "queue.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>


int main(int argc, char** argv) {
   if (argc < 3) {
       std::cout << "Use:\n";
       std::cout << "./producer <type> <value>\n";
       return 1;
   }

   uint32_t type = std::stoi(argv[1]);
   int count = 1000;

   ProducerNode producer("/my_queue", 256);

   for (int i = 0; i < count; ++i) {
       std::this_thread::sleep_for(std::chrono::milliseconds(100));

       if (type == 1) {
           std::string msg = argv[2];
           if (count > 1) {
               msg += " #" + std::to_string(i);
           }

           if (producer.send(1, msg.data(), msg.size())) {
               std::cout << "Sent string: " << msg << std::endl;
           } else {
               std::cout << "Queue full!\n";
           }
       }
       else if (type == 2) {
           int value = std::stoi(argv[2]) + i;

           if (producer.send(2, &value, sizeof(value))) {
               std::cout << "Sent int: " << value << std::endl;
           } else {
               std::cout << "Queue full!\n";
           }
       }
       else if (type == 3) {
           double value = std::stod(argv[2]) + i;

           if (producer.send(3, &value, sizeof(value))) {
               std::cout << "Sent double: " << value << std::endl;
           } else {
               std::cout << "Queue full!\n";
           }
       }
   }

   return 0;
}
