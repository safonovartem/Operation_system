
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

// Удаляем пробелы в начале и конце строки
string trim(const string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");

    if (first == string::npos) {
        return "";
    }

    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

// Создаём изменяемый буфер для strtok()
vector<char> makeBuffer(const string& s) {
    vector<char> buffer(s.begin(), s.end());
    buffer.push_back('\0');
    return buffer;
}

// Разбиваем строку на слова.
// Последний элемент массива указателей — nullptr.
vector<char*> parseWords(char* text) {
    vector<char*> args;

    char* word = strtok(text, " \t\r\n");

    while (word != nullptr) {
        args.push_back(word);
        word = strtok(nullptr, " \t\r\n");
    }

    args.push_back(nullptr);

    return args;
}

// Запускаем одну команду в дочернем процессе.
// outputFile пустой, если перенаправление не нужно.
void executeCommand(const string& command,
                    const string& outputFile = "") {
    vector<char> buffer = makeBuffer(command);
    vector<char*> args = parseWords(buffer.data());

    if (args.size() <= 1) {
        cerr << "ERROR!!!" << endl;
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Код дочернего процесса

        if (!outputFile.empty()) {
            int fd = open(outputFile.c_str(),
                          O_WRONLY | O_CREAT | O_TRUNC,
                          0644);

            if (fd == -1) {
                perror("open");
                _exit(1);
            }

            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2");
                close(fd);
                _exit(1);
            }

            close(fd);
        }

        execvp(args[0], args.data());

        // Эта строка выполняется, только если execvp не удался.
        cerr << "ERROR!!!" << endl;
        _exit(127);
    }

    // Код родительского процесса
    int status;
    while (waitpid(pid, &status, 0) == -1) {
        if (errno != EINTR) {
            perror("waitpid");
            break;
        }
    }
}

// Выполняем конвейер из двух команд
void executePipeline(const string& firstCommand,
                     const string& secondCommand) {
    vector<char> buffer1 = makeBuffer(firstCommand);
    vector<char> buffer2 = makeBuffer(secondCommand);

    vector<char*> args1 = parseWords(buffer1.data());
    vector<char*> args2 = parseWords(buffer2.data());

    if (args1.size() <= 1 || args2.size() <= 1) {
        cerr << "ERROR!!!" << endl;
        return;
    }

    int fd[2];

    if (pipe(fd) == -1) {
        perror("pipe");
        return;
    }

    // Первый дочерний процесс
    pid_t pid1 = fork();

    if (pid1 == -1) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return;
    }

    if (pid1 == 0) {
        // Вывод первой команды направляется в канал
        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }

        close(fd[0]);
        close(fd[1]);

        execvp(args1[0], args1.data());

        cerr << "ERROR!!!" << endl;
        _exit(127);
    }

    // Второй дочерний процесс
    pid_t pid2 = fork();

    if (pid2 == -1) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);

        int status;
        while (waitpid(pid1, &status, 0) == -1 &&
               errno == EINTR) {
        }
        return;
    }

    if (pid2 == 0) {
        // Ввод второй команды берётся из канала
        if (dup2(fd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }

        close(fd[0]);
        close(fd[1]);

        execvp(args2[0], args2.data());

        cerr << "ERROR!!!" << endl;
        _exit(127);
    }

    // Родитель не использует канал напрямую
    close(fd[0]);
    close(fd[1]);

    // Ждём завершения обоих потомков
    int status;

    while (waitpid(pid1, &status, 0) == -1 &&
           errno == EINTR) {
    }

    while (waitpid(pid2, &status, 0) == -1 &&
           errno == EINTR) {
    }
}

int main() {
    string line;

    while (true) {
        cout << "@_@ ready>>> " << flush;

        if (!getline(cin, line)) {
            break;
        }

        line = trim(line);

        if (line.empty()) {
            continue;
        }

        // Команда выхода
        if (line == "by") {
            break;
        }

        // Смена рабочего каталога
        if (line == "cd" ||
            line.rfind("cd ", 0) == 0 ||
            line.rfind("cd\t", 0) == 0) {

            vector<char> buffer = makeBuffer(line);
            vector<char*> args = parseWords(buffer.data());

            if (args.size() < 3) {
                cerr << "ERROR!!!" << endl;
            } else if (chdir(args[1]) == -1) {
                perror("cd");
            }

            continue;
        }

        // Команда echo
        if (line == "echo" ||
            line.rfind("echo ", 0) == 0 ||
            line.rfind("echo\t", 0) == 0) {

            if (line.size() == 4) {
                cout << endl;
            } else {
                cout << line.substr(5) << endl;
            }

            continue;
        }

        // Конвейер из двух команд
        size_t pipePos = line.find('|');

        if (pipePos != string::npos) {
            string firstCommand =
                trim(line.substr(0, pipePos));

            string secondCommand =
                trim(line.substr(pipePos + 1));

            executePipeline(firstCommand, secondCommand);
            continue;
        }

        // Перенаправление вывода в файл
        size_t redirectPos = line.find('>');

        if (redirectPos != string::npos) {
            string command =
                trim(line.substr(0, redirectPos));

            string outputFile =
                trim(line.substr(redirectPos + 1));

            // Имя файла должно быть указано
            if (command.empty() || outputFile.empty() ||
                outputFile.find_first_of(" \t\r\n") !=
                    string::npos) {
                cerr << "ERROR!!!" << endl;
                continue;
            }

            executeCommand(command, outputFile);
            continue;
        }

        // Обычная команда
        executeCommand(line);
    }

    return 0;
}
