#pragma once

#include "blockingconcurrentqueue.h"

#include <vl/vl.h>
#include <cstdlib>
#include <iostream>

template <typename T>
class Queue {
public:
	virtual void pushA(const T &t) = 0;
	virtual void pushB(const T &t) = 0;
	virtual void pop(T &) = 0;
};

template <typename T>
class SWQueue : public Queue<T> {
	moodycamel::BlockingConcurrentQueue<T> q;

public:
	void pushA(const T &t) override {
		q.enqueue(t);
	}
	void pushB(const T &t) override {
		q.enqueue(t);
	}
	void pop(T &t) override {
		q.wait_dequeue(t);
	}
};

template <typename T>
class VLQueue : public Queue<T> {
	int fd;
	vlendpt_t prodA, prodB, cons;

public:
	VLQueue() : fd(mkvl()) {
		std::cout << "newq=" << fd << std::endl;
		assert(!open_byte_vl_as_producer(fd, &prodA, 1));
		std::cout << "init prodA " << (void*)prodA.pcacheline << ", " << prodA.fd << std::endl;
		assert(!open_byte_vl_as_producer(fd, &prodB, 1));
		std::cout << "init prodB " << (void*)prodB.pcacheline << ", " << prodB.fd << std::endl;

		// I can't assert on this or it short-circuits, probably UB somewhere
		int c = open_byte_vl_as_consumer(fd, &cons, 1);
		std::cout << "init cons " << (void*)cons.pcacheline << ", " << cons.fd << std::endl;
	}

	~VLQueue() {
		close_byte_vl_as_producer(prodA);
		close_byte_vl_as_producer(prodB);
		close_byte_vl_as_consumer(cons);
	}

	void pushA(const T &t) override {
		std::cout << "pa " << (void*)prodA.pcacheline << ", " << prodA.fd << std::endl;
		line_vl_push_strong(&prodA, (uint8_t *)&t, sizeof(T));
	}
	void pushB(const T &t) override {
		std::cout << "pb " << (void*)prodB.pcacheline << ", " << prodB.fd << std::endl;
		line_vl_push_strong(&prodB, (uint8_t *)&t, sizeof(T));
	}
	void pop(T &t) override {
		size_t _sz;
		line_vl_pop_strong(&cons, (uint8_t *)&t, &_sz);
	}
};
