#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>
#include <stdexcept>

using namespace std;

vector<int> read_matrix(const string& file_name, int& n) {
	ifstream file(file_name);
	if (!file.is_open()) {
		throw runtime_error("Cannot open file: " + file_name);
	}
	vector<int> numbers;
	int value;
	while (file >> value) {
		numbers.push_back(value);
	}
	file.close();
	n = static_cast<int>(sqrt(numbers.size()));
	if (n * n != numbers.size()) {
		throw runtime_error("Matrix is not square");
	}
	return numbers;
}

void save_matrix(const string& file_name, const vector<int>& matrix, int n) {
	ofstream file(file_name);
	if (!file.is_open()) {
		throw runtime_error("Cannot create file: " + file_name);
	}
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			file << matrix[i * n + j];
			if (j < n - 1) {
				file << ' ';
			}
		}
		file << '\n';
	}

	file.close();
}

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);

	int rank;
	int size;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	vector<int> A;
	vector<int> B;
	vector<int> C;

	int n = 0;

	try {
		if (rank == 0) {
			int n1, n2;

			A = read_matrix("matrix_1.txt", n1);
			B = read_matrix("matrix_2.txt", n2);

			if (n1 != n2) {
				throw runtime_error("Matrix sizes are different");
			}

			n = n1;
		}

		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

		if (rank != 0) {
			B.resize(n * n);
		}

		MPI_Bcast(B.data(), n * n, MPI_INT, 0, MPI_COMM_WORLD);

		int rows_per_process = n / size;
		int remainder = n % size;

		vector<int> sendcounts(size);
		vector<int> displs(size);

		int offset = 0;

		for (int i = 0; i < size; ++i) {
			int rows = rows_per_process + (i < remainder ? 1 : 0);

			sendcounts[i] = rows * n;
			displs[i] = offset;

			offset += rows * n;
		}

		int local_size = sendcounts[rank];
		int local_rows = local_size / n;

		vector<int> local_A(local_size);
		vector<int> local_C(local_size, 0);

		MPI_Barrier(MPI_COMM_WORLD);

		double start = MPI_Wtime();

		MPI_Scatterv(
			A.data(),
			sendcounts.data(),
			displs.data(),
			MPI_INT,
			local_A.data(),
			local_size,
			MPI_INT,
			0,
			MPI_COMM_WORLD
		);

		for (int i = 0; i < local_rows; ++i) {
			for (int j = 0; j < n; ++j) {
				int sum = 0;

				for (int k = 0; k < n; ++k) {
					sum += local_A[i * n + k] * B[k * n + j];
				}

				local_C[i * n + j] = sum;
			}
		}
		if (rank == 0) {
			C.resize(n * n);
		}

		MPI_Gatherv(
			local_C.data(),
			local_size,
			MPI_INT,
			C.data(),
			sendcounts.data(),
			displs.data(),
			MPI_INT,
			0,
			MPI_COMM_WORLD
		);

		double end = MPI_Wtime();

		if (rank == 0) {
			save_matrix("result.txt", C, n);

			cout << "Execution time: " << (end - start) << " sec." << endl;
		}
	}
	catch (const exception& e) {
		if (rank == 0) {
			cerr << "Error: " << e.what() << endl;
		}

		MPI_Finalize();
		return 1;
	}

	MPI_Finalize();
	return 0;
}