#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>


static const uint32_t PROTOCOL_VERSION = 1;


struct MessageHeader {
   uint32_t type;
   uint32_t length;
};

struct QueueMeta {
   uint32_t version;
   uint32_t capacity;

   std::atomic<uint64_t> head;
   std::atomic<uint64_t> tail;
};

struct SharedQueue {
   QueueMeta* meta;
   uint8_t* buffer;
};


inline size_t align8(size_t x) {
   return (x + 7) & ~size_t(7);
}


class ProducerNode {
private:
   int fd;
   QueueMeta* meta;
   uint8_t* buffer;

public:
   ProducerNode(const std::string& name, size_t capacity) {
       fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
       if (fd < 0) throw std::runtime_error("shm_open failed");

       size_t total_size = sizeof(QueueMeta) + capacity;

       if (ftruncate(fd, total_size) != 0)
           throw std::runtime_error("ftruncate failed");

       void* ptr = mmap(nullptr, total_size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);

       if (ptr == MAP_FAILED)
           throw std::runtime_error("mmap failed");

       meta = reinterpret_cast<QueueMeta*>(ptr);
       buffer = reinterpret_cast<uint8_t*>(ptr) + sizeof(QueueMeta);

       // Инициализация (только если первый запуск)
       if (meta->version != PROTOCOL_VERSION) {
           meta->version = PROTOCOL_VERSION;
           meta->capacity = capacity;
           meta->head.store(0);
           meta->tail.store(0);
       }
   }

   bool send(uint32_t type, const void* data, uint32_t len) {
       size_t total = align8(sizeof(MessageHeader) + len);
  
       uint64_t head = meta->head.load(std::memory_order_acquire);
       uint64_t tail = meta->tail.load(std::memory_order_acquire);
  
       if (head - tail + total > meta->capacity) {
           return false; 
       }
  
       uint64_t pos = meta->head.fetch_add(total, std::memory_order_acq_rel);
  
       size_t offset = pos % meta->capacity;
       write_data(offset, type, data, len);
  
       return true;
   }

private:
   void write_data(size_t offset, uint32_t type, const void* data, uint32_t len) {
       MessageHeader hdr{type, len};

       size_t capacity = meta->capacity;

       write_bytes(offset, &hdr, sizeof(hdr));
       write_bytes((offset + sizeof(hdr)) % capacity, data, len);
   }

   void write_bytes(size_t offset, const void* src, size_t len) {
       size_t capacity = meta->capacity;

       size_t first = std::min(len, capacity - offset);
       memcpy(buffer + offset, src, first);

       if (len > first) {
           memcpy(buffer, (uint8_t*)src + first, len - first);
       }
   }
};


class ConsumerNode {
private:
   int fd;
   QueueMeta* meta;
   uint8_t* buffer;

public:
   ConsumerNode(const std::string& name) {
       fd = shm_open(name.c_str(), O_RDWR, 0666);
       if (fd < 0) throw std::runtime_error("shm_open failed");

       size_t meta_size = sizeof(QueueMeta);

       void* ptr = mmap(nullptr, meta_size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);

       if (ptr == MAP_FAILED)
           throw std::runtime_error("mmap failed");

       meta = reinterpret_cast<QueueMeta*>(ptr);

       if (meta->version != PROTOCOL_VERSION)
           throw std::runtime_error("Protocol mismatch");

       size_t total_size = sizeof(QueueMeta) + meta->capacity;

       munmap(ptr, meta_size);

       ptr = mmap(nullptr, total_size,
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED, fd, 0);

       if (ptr == MAP_FAILED)
           throw std::runtime_error("mmap failed");

       meta = reinterpret_cast<QueueMeta*>(ptr);
       buffer = reinterpret_cast<uint8_t*>(ptr) + sizeof(QueueMeta);
   }

   bool receive(uint32_t expected_type, std::string& out) {
       uint64_t tail = meta->tail.load(std::memory_order_acquire);
       uint64_t head = meta->head.load(std::memory_order_acquire);

       if (tail == head) return false;

       size_t offset = tail % meta->capacity;

       MessageHeader hdr;
       read_bytes(offset, &hdr, sizeof(hdr));

       size_t total = align8(sizeof(hdr) + hdr.length);

       if (tail + total > head) return false;

       // Очищаем прочитанные данные (затираем заголовок)
       MessageHeader zero_hdr{0, 0};
       write_bytes(offset, &zero_hdr, sizeof(zero_hdr));

       meta->tail.store(tail + total, std::memory_order_release);

       if (hdr.type == expected_type) {
           std::string payload(hdr.length, '\0');
           read_bytes((offset + sizeof(hdr)) % meta->capacity, payload.data(), hdr.length);
           out = payload;
           return true;
       }

       return false;
   }

private:
   void read_bytes(size_t offset, void* dst, size_t len) {
       size_t capacity = meta->capacity;

       size_t first = std::min(len, capacity - offset);
       memcpy(dst, buffer + offset, first);

       if (len > first) {
           memcpy((uint8_t*)dst + first, buffer, len - first);
       }
   }

   void write_bytes(size_t offset, const void* src, size_t len) {
       size_t capacity = meta->capacity;
       size_t first = std::min(len, capacity - offset);
       memcpy(buffer + offset, src, first);

       if (len > first) {
           memcpy(buffer, (uint8_t*)src + first, len - first);
       }
   }
};
