#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <cstdlib>

using namespace std;

// Обработчик SIGINT для родителя
void parent_signal_handler(int signal)
{
    cout << "Main process: I catched signal #" << signal
         << "... finished..." << endl;

    exit(0);
}

// Обработчик SIGINT для дочернего процесса
void child_signal_handler(int signal)
{
    cout << "Child process: I catched signal... finished..."
         << endl;

    exit(0);
}

// Обработчик SIGCHLD для родителя
void child_finished_handler(int signal)
{
    cout << "Main process: My child killed..." << endl;

    // Чтобы получить статус завершившегося ребёнка
    waitpid(-1, nullptr, WNOHANG);
}

int main()
{
    // Обработчик SIGINT
    signal(SIGINT, parent_signal_handler);

    // Обработчик SIGCHLD
    signal(SIGCHLD, child_finished_handler);

    // SIGKILL перехватить невозможно!
    // signal(SIGKILL, ...); // делать нельзя

    cout << "Main process: I'm started... PID = "
         << getpid() << endl;

    pid_t pid = fork();

    if (pid < 0)
    {
        cerr << "Error: fork() failed!" << endl;
        return 1;
    }

    if (pid == 0)
    {
        // Дочерний процесс

        // Меняем обработчик SIGINT для ребёнка
        signal(SIGINT, child_signal_handler);

        cout << "Child process: I'm started... PID = "
             << getpid() << endl;

        // Ждём сигнал
        pause();

        return 0;
    }
    else
    {
        // Родительский процесс

        // Ждём сигнал
        pause();

        return 0;
    }
}
