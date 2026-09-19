#include <unistd.h>
#include <sys/wait.h>

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std;

// ====================
// Bubble Sort
// ====================
void bubble(int* v, int size)
{
    for (int i = 0; i < size - 1; i++)
    {
        for (int j = 0; j < size - 1 - i; j++)
        {
            if (v[j] > v[j + 1])
            {
                int tmp = v[j];
                v[j] = v[j + 1];
                v[j + 1] = tmp;
            }
        }
    }
}

// ====================
// Shell Sort
// ====================
void shell(int* v, int size)
{
    for (int gap = size / 2; gap > 0; gap /= 2)
    {
        for (int i = gap; i < size; i++)
        {
            int temp = v[i];
            int j = i;

            while (j >= gap && v[j - gap] > temp)
            {
                v[j] = v[j - gap];
                j -= gap;
            }

            v[j] = temp;
        }
    }
}

// ====================
// Quick Sort
// ====================
void qs(int* v, int low, int high)
{
    int left = low;
    int right = high;

    int middle = v[(low + high) / 2];

    while (left <= right)
    {
        while (v[left] < middle)
        {
            left++;
        }

        while (v[right] > middle)
        {
            right--;
        }

        if (left <= right)
        {
            int tmp = v[left];
            v[left] = v[right];
            v[right] = tmp;

            left++;
            right--;
        }
    }

    if (low < right)
    {
        qs(v, low, right);
    }

    if (left < high)
    {
        qs(v, left, high);
    }
}

void quicks(int* v, int size)
{
    if (size > 1)
    {
        qs(v, 0, size - 1);
    }
}

// ====================
// Вывод массива
// ====================
void printArray(int* v, int size)
{
    for (int i = 0; i < size; i++)
    {
        cout << v[i] << " ";
    }

    cout << endl;
}

// ====================
// main
// ====================
int main()
{
    int n;

    cout << "Enter array size: ";
    cin >> n;

    if (n <= 0)
    {
        cout << "Array size must be greater than 0." << endl;
        return 1;
    }

    // Создаём массив
    int* massiv = new int[n];

    // Инициализация генератора случайных чисел
    srand(time(nullptr));

    // Заполняем массив
    for (int i = 0; i < n; i++)
    {
        massiv[i] = rand() % 1000;
    }

    cout << "\nOriginal array:\n";

    if (n < 50)
    {
        printArray(massiv, n);
    }
    else
    {
        cout << "Array is too large to display." << endl;
    }

    // Создаём 3 дочерних процесса
    for (int i = 0; i < 3; i++)
    {
        pid_t pid = fork();

        // Ошибка fork
        if (pid < 0)
        {
            perror("fork");
            delete[] massiv;
            return 1;
        }

        // =========================
        // Дочерний процесс
        // =========================
        if (pid == 0)
        {
            auto start = chrono::high_resolution_clock::now();

            if (i == 0)
            {
                bubble(massiv, n);
            }
            else if (i == 1)
            {
                shell(massiv, n);
            }
            else if (i == 2)
            {
                quicks(massiv, n);
            }

            auto finish = chrono::high_resolution_clock::now();

            auto duration =
                chrono::duration_cast<chrono::microseconds>
                (finish - start);

            cout << "\nChild process PID: " << getpid() << endl;

            if (i == 0)
            {
                cout << "Bubble Sort" << endl;
            }
            else if (i == 1)
            {
                cout << "Shell Sort" << endl;
            }
            else
            {
                cout << "Quick Sort" << endl;
            }

            if (n < 50)
            {
                cout << "Sorted array: ";
                printArray(massiv, n);
            }

            cout << "Time: "
                 << duration.count()
                 << " microseconds"
                 << endl;

            // Очень важно!
            // Потомок должен завершиться,
            // чтобы не создавать новых потомков.
            _exit(0);
        }

        // Родитель продолжает цикл
    }

    // =========================
    // Родитель ждёт 3 процесса
    // =========================

    for (int i = 0; i < 3; i++)
    {
        wait(nullptr);
    }

    cout << "\nAll child processes finished." << endl;

    delete[] massiv;

    return 0;
}
