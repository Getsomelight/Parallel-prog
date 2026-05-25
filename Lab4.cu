#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <cuda_runtime.h>

using namespace std;

vector<int> read_file(const string& file_name, int& n) {
    ifstream file(file_name);

    if (!file.is_open()) {
        throw runtime_error("Cannot open file");
    }

    vector<int> numbers;
    int num;

    while (file >> num) {
        numbers.push_back(num);
    }

    file.close();

    n = static_cast<int>(sqrt(numbers.size()));

    return numbers;
}

void save_file(const string& file_name, const vector<int>& matrix, int n) {
    ofstream file(file_name);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            file << matrix[i * n + j];

            if (j != n - 1)
                file << ' ';
        }

        file << '\n';
    }

    file.close();
}

__global__
void matrixMultiplyKernel(int* A, int* B, int* C, int n) {

    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < n && col < n) {

        int sum = 0;

        for (int k = 0; k < n; ++k) {
            sum += A[row * n + k] * B[k * n + col];
        }

        C[row * n + col] = sum;
    }
}

int main(int argc, char* argv[]) {

    try {

        if (argc != 3) {
            cerr << "Usage: program block_x block_y" << endl;
            return 1;
        }

        int block_x = atoi(argv[1]);
        int block_y = atoi(argv[2]);

        int n1, n2;

        vector<int> A = read_file("matrix_1.txt", n1);
        vector<int> B = read_file("matrix_2.txt", n2);

        if (n1 != n2) {
            throw runtime_error("Matrix sizes are different");
        }

        int n = n1;

        size_t bytes = n * n * sizeof(int);

        int* d_A;
        int* d_B;
        int* d_C;

        cudaMalloc(&d_A, bytes);
        cudaMalloc(&d_B, bytes);
        cudaMalloc(&d_C, bytes);

        cudaMemcpy(d_A, A.data(), bytes, cudaMemcpyHostToDevice);
        cudaMemcpy(d_B, B.data(), bytes, cudaMemcpyHostToDevice);

        dim3 block(block_x, block_y);

        dim3 grid(
            (n + block.x - 1) / block.x,
            (n + block.y - 1) / block.y
        );

        cudaEvent_t start, stop;

        cudaEventCreate(&start);
        cudaEventCreate(&stop);

        cudaEventRecord(start);

        matrixMultiplyKernel<<<grid, block>>>(d_A, d_B, d_C, n);

        cudaEventRecord(stop);

        cudaEventSynchronize(stop);

        float milliseconds = 0;

        cudaEventElapsedTime(&milliseconds, start, stop);

        vector<int> C(n * n);

        cudaMemcpy(C.data(), d_C, bytes, cudaMemcpyDeviceToHost);

        save_file("result.txt", C, n);

        cout << "Execution time: "
             << milliseconds / 1000.0
             << " sec." << endl;

        cout << "Grid: "
             << grid.x << "x" << grid.y
             << endl;

        cout << "Block: "
             << block.x << "x" << block.y
             << endl;

        cudaFree(d_A);
        cudaFree(d_B);
        cudaFree(d_C);

    }
    catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}