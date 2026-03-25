#pragma once

#include <optional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <stdexcept>


template <class T>
class UnbufferedChannel {
private:
    struct SendOp {
        T value;
        bool done = false;
        std::condition_variable cv;
    };
 
    struct RecvOp {
        std::optional<T> value;
        bool done = false;
        std::condition_variable cv;
    };
 
    std::mutex mtx_;
    bool closed_ = false;
    std::queue<SendOp*> senders_;
    std::queue<RecvOp*> receivers_;


public:
    void Send(const T& value) {
        std::unique_lock lock(mtx_);

        if (closed_) {
            throw std::runtime_error("Channel is closed");
        }

        if (!receivers_.empty()) {
            RecvOp* recv = receivers_.front();
            receivers_.pop();
 
            recv->value = value;
            recv->done = true;
            recv->cv.notify_one();
 
            return;
        }
 
        SendOp op{value};
 
        senders_.push(&op);
 
        op.cv.wait(lock, [&]() {
            return op.done || closed_;
        });
 
        if (!op.done) {
            throw std::runtime_error("Channel was closed");
        }
    }
 
    std::optional<T> Recv() {
        std::unique_lock lock(mtx_);
 
        if (!senders_.empty()) {
            SendOp* sender = senders_.front();
            senders_.pop();
 
            T value = std::move(sender->value);
            sender->done = true;
            sender->cv.notify_one();
 
            return value;
        }
 
        if (closed_) {
            return std::nullopt;
        }
 
        RecvOp op;
        receivers_.push(&op);
 
        op.cv.wait(lock, [&]() {
            return op.done || closed_;
        });
 
        if (!op.done) {
            return std::nullopt;
        }
 
        return std::move(op.value);
    }
 
 
    void Close() {
        std::unique_lock lock(mtx_);
        closed_ = true;
 
        while (!receivers_.empty()) {
            RecvOp* r = receivers_.front();
            receivers_.pop();
            r->done = true;
            r->cv.notify_one();
        }
 
        while (!senders_.empty()) {
            SendOp* s = senders_.front();
            senders_.pop();
            s->cv.notify_one();
        }
    }
};