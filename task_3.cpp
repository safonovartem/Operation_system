#include <iostream>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>

int main(int argc, char* argv[])
{
    // Проверяем наличие имени файла
    if (argc != 2)
    {
        std::cerr << "Использование: "
                  << argv[0]
                  << " <имя_файла>\n";

        return 1;
    }

    // Структура для хранения информации о файле
    struct stat fileInfo;

    // Получаем информацию о файле
    if (stat(argv[1], &fileInfo) == -1)
    {
        std::cerr << "Ошибка: не удалось получить информацию о файле\n";
        return 1;
    }

    // st_mode
    std::cout << "Режим доступа и тип файла: "
              << fileInfo.st_mode << '\n';

    // st_uid
    std::cout << "UID пользователя: "
              << fileInfo.st_uid << '\n';

    // st_gid
    std::cout << "GID группы: "
              << fileInfo.st_gid << '\n';

    // st_size
    std::cout << "Размер файла: "
              << fileInfo.st_size
              << " байт\n";

    // st_mtime
    std::cout << "Последнее изменение: "
              << ctime(&fileInfo.st_mtime);

    // Проверяем, является ли каталогом
    std::cout << "Является каталогом: "
              << (S_ISDIR(fileInfo.st_mode) ? "да" : "нет")
              << '\n';

    // Проверяем, является ли обычным файлом
    std::cout << "Является обычным файлом: "
              << (S_ISREG(fileInfo.st_mode) ? "да" : "нет")
              << '\n';

    // Получаем информацию о пользователе
    struct passwd* userInfo = getpwuid(fileInfo.st_uid);

    if (userInfo != nullptr)
    {
        std::cout << "Имя пользователя: "
                  << userInfo->pw_name << '\n';
    }
    else
    {
        std::cout << "Имя пользователя не найдено\n";
    }

    // Получаем информацию о группе
    struct group* groupInfo = getgrgid(fileInfo.st_gid);

    if (groupInfo != nullptr)
    {
        std::cout << "Имя группы: "
                  << groupInfo->gr_name << '\n';
    }
    else
    {
        std::cout << "Имя группы не найдено\n";
    }

    return 0;
}
