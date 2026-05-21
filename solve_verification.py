import os
import re
import subprocess

import numpy as np

np.random.seed(1000)
cpp_executable = os.environ.get("CPP_EXECUTABLE", "C:/Users/Максим/PycharmProjects/math/Lab2.exe")

threads_list = [1, 2, 4, 8]

def gen_matrix(i):
    random_matrix_1 = np.random.randint(0, 100, (i, i))
    random_matrix_2 = np.random.randint(0, 100, (i, i))
    file_name = f"matrix_1.txt"
    np.savetxt(file_name, random_matrix_1, delimiter=' ', fmt='%.0f')
    file_name = f"matrix_2.txt"
    np.savetxt(file_name, random_matrix_2, delimiter=' ', fmt='%.0f')


def read_file(file_name: str):
    try:
        matrix = np.loadtxt(file_name, dtype=int)
        return matrix
    except Exception as e:
        print(f"Error: {e}")
        return None


def main():
    for threads in threads_list:
        print(f"\nЯдра: 4,  Потоки: {threads}")
        env = os.environ.copy()
        env["OMP_NUM_THREADS"] = str(threads)
        env["OMP_PROC_BIND"] = "TRUE"
        for n in range(200, 2200, 200):
            summ = 0
            for i in range(0, 10):
                gen_matrix(n)
                if os.path.exists("execution_time.txt"):
                    os.remove("execution_time.txt")
                try:
                    subprocess.run(cpp_executable, check=True, capture_output=True, text=True, env=env)
                    with open("execution_time.txt", "r") as f:
                        summ += float(f.read().strip())
                except subprocess.CalledProcessError as e:
                    print("Ошибка при выполнении C++ программы:", e)
                a = np.array(read_file("matrix_1.txt"))
                b = np.array(read_file("matrix_2.txt"))
                c = np.array(read_file("result.txt"))
                if np.array_equal(c, a @ b):
                    pass
                else:
                    print("Результат проверки: ", False)
            print(f"Среднее время выполнения для матрицы размера {n}*{n}: {summ / 10} сек.")


if __name__ == "__main__":
    main()
