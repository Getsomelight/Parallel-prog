#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <omp.h> 


using namespace std;


vector<vector<int>> read_file(const string& file_name) {
    ifstream file(file_name);
    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + file_name);
    }
    vector<int> numbers;
    int num;
    while (file >> num) {
        numbers.push_back(num);
    }
    file.close();
    int n = static_cast<int>(sqrt(numbers.size()));
    vector<vector<int>> matrix(n, vector<int>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            matrix[i][j] = numbers[i * n + j];
        }
    }
    return matrix;
}


void save_file(const string& file_name, const vector<vector<int>>& matrix) {
    ofstream file(file_name);
    if (!file.is_open()) {
        throw runtime_error("Cannot create file: " + file_name);
    }
    int n = matrix.size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            file << matrix[i][j];
            if (j < n - 1) file << ' ';
        }
        file << '\n';
    }
    file.close();
}


vector<vector<int>> multiply_matrixes(const vector<vector<int>>& A, const vector<vector<int>>& B, int n) {
    vector<vector<int>> C(n, vector<int>(n, 0));
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    return C;
}


void print_matrix(const vector<vector<int>>& A) {
    int n = A.size();
    int max_value = numeric_limits<int>::min();
    for (const auto& row : A) {
        if (!row.empty()) {
            int current_row_max = *std::max_element(row.begin(), row.end());
            max_value = std::max(max_value, current_row_max);
        }
    }
    int count = 0;
    if (max_value == 0) count = 1;
    else {
        while (max_value > 0) {
            max_value /= 10;
            count++;
        }
    }
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cout << left << setw(count) << A[i][j] << ' ';
        }
        cout << endl;
    }
}


int main()
{
    try {
        vector<vector<int>> matrix_a, matrix_b, result;
        matrix_a = read_file("matrix_1.txt");
        matrix_b = read_file("matrix_2.txt");
        auto start = chrono::high_resolution_clock::now();
        result = multiply_matrixes(matrix_a, matrix_b, matrix_a.size());
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> diff = end - start;
        save_file("result.txt", result);
        ofstream time_file("execution_time.txt");
        if (time_file.is_open()) {
            time_file << diff.count() << "\n";
            time_file.close();
        }
        //cout << "Matrix A:" << endl;
        //print_matrix(matrix_a);
        //cout << "\n\nMatrix B:" << endl;
        //print_matrix(matrix_b);
        //cout << "\n\nResult of multiplication:" << endl;
        //print_matrix(result);
        cout << "\nExecution time: " << diff.count() << " sec." << endl;
    }
    catch (const exception& e){
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
