#include <pthread.h>
#include <unistd.h>
#include <array>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include "./DoubleQueue.hpp"

using std::array;
using std::cin;
using std::cout;
using std::endl;
using std::fixed;
using std::numeric_limits;
using std::setprecision;
using std::streamsize;

bool printout = false;
// globals
//
// NOTE: When you modify this to use DobuleQueue to communciate between threads
// one thread will need this array, and the threads should use DoubleQueue to
// communicate
array<double, 5> nums;
int len = 0;

pthread_mutex_t mutex{};

// Consumer thread - use DoubleQueue and "nums" array
// if there are more than 5 elements in the
// array, then the array is truncated down to
// 5 elements
// The array should never exceed 6 elements
void* print_nums_thread(void* arg);

// Producer thread
// Reads a number from the console
// and adds it to DoubleQueue.
//
// returns false when the EOF (ctrl + d)
// is read. Meaning that the overall
// program should now terminate..
void* read_nums_thread(void* arg);

int main() {
  DoubleQueue dq;
  pthread_t read_thread;
  pthread_t print_thread;
  pthread_mutex_init(&mutex, NULL);

  pthread_create(&read_thread, NULL, read_nums_thread, &dq);
  pthread_create(&print_thread, NULL, print_nums_thread, &dq);

  pthread_join(read_thread, NULL);
  pthread_join(print_thread, NULL);

  pthread_mutex_destroy(&mutex);

  return 0;
}

void* print_nums_thread(void* arg) {
  if (printout) {
    cout << "print thread started" << endl;
  }
  DoubleQueue* dq = (DoubleQueue*)arg;
  while (dq->length() > 0 || !dq->is_closed()) {

    if (len >= 5) {
      for (int i = 0; i < 4; i++) {
        nums.at(i) = nums.at(i + 1);
      }
      len--;
    }

    auto num = dq->wait_remove();
    if (num.has_value()) {
      double value = num.value();
      if (printout){
        cout << "wait_remove value: " << value << endl;
      }
      nums.at(len) = value;
      len++;

      double min = nums[0];
      double max = nums[0];
      double sum = nums[0];
      for (int i = 1; i < len; i++) {
        sum += nums.at(i);
        if (min > nums.at(i)) {
          min = nums.at(i);
        }
        if (max < nums.at(i)) {
          max = nums.at(i);
        }
      }
      if (printout) {
        cout << "sum: " << sum << ", len:" << len << endl;
      }
      double avg = sum / len;
      pthread_mutex_lock(&mutex);
      cout << setprecision(2) << std::fixed;
      cout << "Max: " << max << endl;
      cout << "Min: " << min << endl;
      cout << "Average: " << avg << endl;
      cout << "Last five: ";

      for (int i = 0; i < len; i++) {
        cout << nums.at(i) << " ";
      }
      cout << endl;
      pthread_mutex_unlock(&mutex);
    }
  }
  if (printout) {
    cout << "print thread exiting" << endl;
  }
  pthread_exit(NULL);
}

// Producer thread
void* read_nums_thread(void* arg) {
  if (printout) {
    cout << "read thread started" << endl;
  }
  DoubleQueue* dq = (DoubleQueue*)arg;
  double value;
  while (true) {
    pthread_mutex_lock(&mutex);
    cin >> value;
    pthread_mutex_unlock(&mutex);
    if (cin.eof()) {
      if (printout) {
        cout << "EOF" << endl;
      }
      dq->close();
      break;  // Exit loop on EOF
    }
    if (!cin) {  // Handle invalid input
      cin.clear();
      cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
      continue;
    }
    dq->add(value);
    sleep(1);
    if (printout) {
      cout << "added value: " << value << endl;
    }
    // pthread_mutex_unlock(&mutex);
  }
  if (printout) {
    cout << "read thread exiting" << endl;
  }
  pthread_exit(NULL);
}
