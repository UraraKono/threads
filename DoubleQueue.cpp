#include "./DoubleQueue.hpp"
#include <pthread.h>
#include <iostream>

// Constructor for a DoubleQueue
DoubleQueue::DoubleQueue() : head(nullptr), tail(nullptr), closed(false) {
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);
}

// Destructor for DoubleQueue
DoubleQueue::~DoubleQueue() {
  // std::cout << "destructor called" << std::endl;
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
  QueueNode* current = head;
  while (current != nullptr) {
    QueueNode* temp = current;
    current = current->next;
    delete temp;
  }
}

// Adds a double to the end of the queue
bool DoubleQueue::add(double val) {
  pthread_mutex_lock(&mutex);
  if (closed) {
    // std::cout << "closed, so return false in add" << std::endl;
    pthread_mutex_unlock(&mutex);
    return false;
  }

  QueueNode* newNode = new QueueNode();
  newNode->value = val;
  newNode->next = nullptr;
  if (head == nullptr && tail == nullptr) {
    head = newNode;
    tail = newNode;
  } else {
    tail->next = newNode;
    tail = newNode;
  }
  // std::cout << "added: " << val << std::endl;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);

  // std::cout << "added: " << val << std::endl;
  // std::cout << "head: " << head->value << std::endl;
  // std::cout << "tail: " << tail->value << std::endl;

  return true;
}

void DoubleQueue::close() {
  pthread_mutex_lock(&mutex);
  closed = true;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
}

bool DoubleQueue::is_closed() {
  pthread_mutex_lock(&mutex);
  bool result = closed;
  pthread_mutex_unlock(&mutex);
  return result;
}

std::optional<double> DoubleQueue::remove() {
  pthread_mutex_lock(&mutex);
  if (head == nullptr) {
    pthread_mutex_unlock(&mutex);
    return std::nullopt;
  }
  QueueNode* temp = head;
  head = head->next;
  tail = head == nullptr ? nullptr : tail;
  double value = temp->value;
  delete temp;
  pthread_mutex_unlock(&mutex);
  // std::cout << "remove value: " << value << std::endl;
  return value;
}

std::optional<double> DoubleQueue::wait_remove() {
  // std::cout << "start of wait remove" << std::endl;
  pthread_mutex_lock(&mutex);
  // std::cout << "head==nullptr: " << (head == nullptr) << std::endl;
  // std::cout << "closed: " << closed << std::endl;
  if (closed && head == nullptr) {
    // std::cout << "closed and head==nullptr, so return nullopt in wait_remove"
    //           << std::endl;
    pthread_mutex_unlock(&mutex);
    return std::nullopt;
  }
  // std::cout << "length" << length() << std::endl;
  while (head == nullptr && !closed) {
    // std::cout << "waiting" << std::endl;
    pthread_cond_wait(&cond, &mutex);
    // std::cout << "still waiting" << std::endl;
  }
  if (head == nullptr && closed) {
    pthread_mutex_unlock(&mutex);
    return std::nullopt;
  }
  // move to while loop
  QueueNode* temp = head;
  head = head->next;
  tail = head == nullptr ? nullptr : tail;
  double value = temp->value;
  delete temp;
  pthread_mutex_unlock(&mutex);
  // std::cout << "end of wait remove value: " << value << ", length:" << length()
  //           << std::endl;
  return value;
}

int DoubleQueue::length() {
  pthread_mutex_lock(&mutex);
  int count = 0;
  QueueNode* temp = head;
  // std::cout << "head: " << head->value << std::endl;
  while (temp != nullptr) {
    count++;
    // std::cout << "count: " << count << std::endl;
    temp = temp->next;
  }
  pthread_mutex_unlock(&mutex);
  return count;
}