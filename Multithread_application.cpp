#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono> // for std::this_thread::sleep_for

// Simulated data structure
struct Data {
    // Add your data fields here
    int value;

    // Constructor
    Data(int v) : value(v) {}
};

// Shared data structure among threads
struct SharedData {
    std::vector<Data> dataBuffer;
    std::mutex bufferMutex;
    std::condition_variable bufferNotEmpty;
    bool isProcessingFinished = false;
};

// Function to simulate data processing
void processData(SharedData& sharedData, int threadID) {
    while (true) {
        // Acquire the lock
        std::unique_lock<std::mutex> lock(sharedData.bufferMutex);

        // Wait until data is available or processing is finished
        sharedData.bufferNotEmpty.wait(lock, [&sharedData] {
            return !sharedData.dataBuffer.empty() || sharedData.isProcessingFinished;
        });

        // Check if processing is finished
        if (sharedData.isProcessingFinished && sharedData.dataBuffer.empty()) {
            lock.unlock();
            break;
        }

        // Retrieve data from the buffer
        Data currentData = sharedData.dataBuffer.back();
        sharedData.dataBuffer.pop_back();

        // Release the lock
        lock.unlock();

        // Process the data (you can replace this with your actual processing logic)
        std::cout << "Thread " << threadID << " processing data: " << currentData.value << std::endl;

        // Simulate processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    const int numThreads = 2; // Adjust based on your requirements
    const int numDataItems = 10; // Adjust based on your requirements

    // Shared data among threads
    SharedData sharedData;

    // Create threads
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(processData, std::ref(sharedData), i);
    }

    // Simulate data input (replace this with your actual data source)
    for (int i = 0; i < numDataItems; ++i) {
        std::unique_lock<std::mutex> lock(sharedData.bufferMutex);
        sharedData.dataBuffer.emplace_back(i);
        lock.unlock();

        // Notify one of the waiting threads that data is available
        sharedData.bufferNotEmpty.notify_one();

        // Simulate data input rate
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Signal that processing is finished
    sharedData.isProcessingFinished = true;

    // Notify all waiting threads to wake up and finish
    sharedData.bufferNotEmpty.notify_all();

    // Join the threads
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
