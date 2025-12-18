#include "esp32-hal-cpu.h"
#include <Arduino.h>

const int ARRAY_SIZES[] = {50, 100, 500, 1000};
const int NUM_SIZES = sizeof(ARRAY_SIZES) / sizeof(ARRAY_SIZES[0]);
const uint32_t CPU_FREQUENCIES[] = {240, 160, 80, 40};
const int NUM_FREQUENCIES = sizeof(CPU_FREQUENCIES) / sizeof(CPU_FREQUENCIES[0]);

#define START_TIMER() start_time = esp_timer_get_time() 
#define END_TIMER(time_var) time_var = esp_timer_get_time() - start_time

String results_table = "Size\tType\tFreq(MHz)\tAlgorithm\tTime(us)\tHeap_Before(B)\tHeap_After(B)\n";

template <typename T>
int partition(T arr[], int low, int high) {
    T pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] <= pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return (i + 1);
}

template <typename T>
void quickSort(T arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

template <typename T>
struct Node {
    T data;
    Node<T>* left;
    Node<T>* right;
    Node(T value) : data(value), left(nullptr), right(nullptr) {}
};

template <typename T>
Node<T>* insert(Node<T>* root, T data);

template <typename T>
void deleteTree(Node<T>* node);

template <typename T>
Node<T>* insert(Node<T>* root, T data) { 
    if (root == nullptr) {
        return new Node<T>(data);
    }
    if (data < root->data) {
        root->left = insert(root->left, data);
    } else if (data > root->data) {
        root->right = insert(root->right, data);
    }
    return root;
}

template <typename T>
void deleteTree(Node<T>* node) { 
    if (node == nullptr) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

template <typename T>
void generate_array(T arr[], int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = (T)random(1, 10000); 
    }
    if (std::is_same<T, char>::value) {
        for (int i = 0; i < size; i++) {
            arr[i] = (T)random(33, 126); 
        }
    }
}

template <typename T>
void run_test(const char* type_name, int size, uint32_t freq) {
    uint64_t start_time;
    
    if (!setCpuFrequencyMhz(freq)) {
        if (freq == 40) return;
    }
    
    uint32_t current_freq = getCpuFrequencyMhz();
    if (current_freq != freq) {
      Serial.printf("Warning: Requested Freq %d MHz, Actual Freq %d MHz. Skipping test.\n", freq, current_freq);
      return;
    }

    T* arr = new T[size];
    generate_array(arr, size);
    
    T* arr_copy = new T[size];
    memcpy(arr_copy, arr, size * sizeof(T));

    size_t heap_before_qs = ESP.getFreeHeap();
    uint64_t time_qs;
    
    START_TIMER(); 
    quickSort(arr, 0, size - 1);
    END_TIMER(time_qs);

    size_t heap_after_qs = ESP.getFreeHeap();

    results_table += String(size) + "\t" + type_name + "\t" + String(current_freq) + 
                     "\tQuickSort\t" + String(time_qs) + "\t" + String(heap_before_qs) + 
                     "\t" + String(heap_after_qs) + "\n";
    
    delete[] arr; 

    Node<T>* root = nullptr;
    size_t heap_before_bst = ESP.getFreeHeap();
    uint64_t time_bst;
    
    START_TIMER(); 
    for (int i = 0; i < size; i++) {
        root = insert(root, arr_copy[i]);
    }
    END_TIMER(time_bst);

    size_t heap_after_bst = ESP.getFreeHeap();
    
    results_table += String(size) + "\t" + type_name + "\t" + String(current_freq) + 
                     "\tBinaryTree\t" + String(time_bst) + "\t" + String(heap_before_bst) + 
                     "\t" + String(heap_after_bst) + "\n";
    
    deleteTree(root);
    delete[] arr_copy;
}

void run_all_tests() {
    for (int i = 0; i < NUM_FREQUENCIES; i++) {
        uint32_t freq = CPU_FREQUENCIES[i];
        
        for (int j = 0; j < NUM_SIZES; j++) {
            int size = ARRAY_SIZES[j];
            
            run_test<int>("int", size, freq);
            run_test<float>("float", size, freq);
            run_test<double>("double", size, freq); 
            run_test<char>("char", size, freq);
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(5000); 
    Serial.println("\n--- Завдання: Дослідження алгоритмів сортування та бінарного дерева на ESP32 ---");
    randomSeed(analogRead(0));
    run_all_tests();
    Serial.println("\n--- Зведені результати вимірювань ---");
    Serial.println("(Час у мікросекундах (us), Пам'ять Heap у байтах (B))");
    Serial.println(results_table);
    Serial.println("--- Тестування завершено. Скопіюйте дані для побудови графіків ---");
}

void loop() {}

