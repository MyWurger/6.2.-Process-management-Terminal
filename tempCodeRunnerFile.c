#include <stdio.h>                   // подключение стандартной библиотеки ввода/вывода
#include <stdlib.h>                  // содержит в себе функции, занимающиеся выделением памяти
#include <sys/types.h>               // определение различных типов данных, используемых системными вызовами и функциями ввода/вывода         
#include <unistd.h>                  // содержит символические константы и структуры, которые еще не были описаны в каких-либо других включаемых файлах
#include <stdbool.h>                 // функции для работы с типом данных bool
#include <signal.h>                  // обработка сигналов во время выполнения программы
#include <string.h>                  // функции для работы со строками
#include <sys/wait.h>                // функции для работы с процессами и их состояниями



/***************************************************/
/*             ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ               */
/***************************************************/

// дочерние процессы для терминала будут добавляться в структуру и соединяться в список
// в языке C для объявления указателя на структуру пишут слово struct
struct PID_list
{
    pid_t PID;                       // ID добавляемого дочернего процесса
    struct PID_list *next_process;   // указатель на следующий созданные по порядку дочерний процесс для формирования списка
};// PID_list

struct PID_list *head_list;          // указатель на начальный элемент структуры типа "дочерний процесс"




/***************************************************/
/*             РЕАЛИЗАЦИЯ ФУНКЦИЙ                  */
/***************************************************/

/*----------------------------------------------------------------------*/
/* Добавление нового элемента (дочернего процесса) в список */
/*----------------------------------------------------------*/

void add_elem (pid_t new_PID)        // PID добавляемого процесса
{
    // выделяем память под новый экземпляр списка
    struct PID_list *new_list = (struct PID_list *)malloc(sizeof(struct PID_list));

    new_list->PID = new_PID;         // добавляем ID дочернего процесса в созданный экземплер списка
    new_list->next_process = NULL;   // элемент добавляется в конец списка. Он указывает на NULL

    // Элемент - начало списка. Если список пустой, т.е. данный дочерний процесс - первый
    if(head_list == NULL)
    {
        head_list = new_list;        // запоминаем его как "голову" списка
    }

    // список уже не пустой
    else
    {
        // объявляем указатель на структуру списка для прохода до конца списка
        struct PID_list *current = head_list;
        // пока следующий элемент списка существует - проходим до конца списка
        while(current->next_process != NULL)
        {
            // сдвигаем указатель на этот элемент
            current = current->next_process;
        }// while

        // связываем старый список с новым добавляемым элементом списка. Путем присвоения указателю конца старого списка добавляемого элемента
        current->next_process = new_list;
    }// if(head_list == NULL)
    return;                          // вернули обещанное значение
}



/*----------------------------------------------------------------------*/
/*     Удаление элемента из начала списка для FIFO     */
/*-----------------------------------------------------*/

void delete_head_list()
{
    // список пуст - элементов не добавлялось или все удалены
    if(head_list == NULL)
    {
        return;                      // список пуст
    }// if(head_list == NULL)

    // объявили указатель на структуру списка
    struct PID_list *current = head_list;

    // сместили указатель начала списка после удаления элемента
    head_list = current->next_process;

    free(current);                   // удалили элемент структуры, выделенный ранее динамически
    return;                          // вернули обещанное значение
}



/*----------------------------------------------------------------------*/
/*     Перехват и обработка сигнала Ctrl+C     */
/*---------------------------------------------*/

void Ctrl_plus_C(int sig)            // сигнал, который нужно перехватить
{
    // дочерние элементы у процесса отсутствуют - удаляем сам процесс
    if(head_list == NULL)
    {
        // Оповещение, что убит - родительский процесс и вывод его PID
        printf("\nУбит родительский процесс с PID %d\n", getpid());
        kill(getpid(), SIGTERM);     // отправляем сигнал SIGTERM для завершения текущего процесса 
        waitpid(-1, NULL, 0);        // ждём его завершения для родительского процесса
    }

    else
    {
        // печатаем дочерние процессы, чтобы было видно, какой завершается. Объявляем указатель на элемент списка для печати PID всех процессов списка
        struct PID_list *current = head_list;
        int i = 1;                   // нумерация дочернего процесса в списке

        printf("\n\033[38;5;161mДочерние процессы для родительского процесса %d:\033[0m\n", getpid());

        // проходимся по списку дочерних элементов, пока не дойдём до конца списка
        while(current != NULL)
        {
            // печать номера дочернего процесса и его PID
            printf("\033[38;5;135m%i. Дочерний процесс PID:\033[0m \033[38;5;228m%d\033[0m\n", i, current->PID);
            // переходим к следующему дочернему элементу списка
            current = current->next_process;
            i = i + 1;               // увеличиваем номер
        }// while()

        // выводим PID процесса, который убит алгоритмом FIFO
         printf("Убит процесс %d\n", head_list->PID);
        // перемещаем дочерний процесс в состояние "зомби"
        kill(head_list->PID, SIGTERM);
        // ожидание завершения нашего зомби-процесса
        waitpid(head_list->PID, NULL, 0);

        // удаляем закончившийся дочерний процесс из списка дочерних процессов по FIFO.  
        delete_head_list();

    }// if(head_list == NULL)

    // просьба ввода новой команды. Т.к. буфер вывода неактивен до ближайшего ввода
    char cwd[256];                   // массив для хранения текущего рабочего каталога
    getcwd(cwd, sizeof(cwd)) != NULL;
    printf("\n\033[38;5;46m%s\033[0m \033[38;5;63m$\033[0m ", cwd);
    fflush(stdout);                  // вынимаем немедленно всё содержимое буфера вывода
    return;                          // вернули обещанное значение
}



/*----------------------------------------------------------------------*/
/*        Создание нового процесса         */
/*-----------------------------------------*/

void create_new_process(char **con_args)  // указатель на многомерный массив строки консоли, разделённый на отдельные аргументы
{
    pid_t pid;                       // идентификатор процесса
    int success = -1;                // проверка успеха замены образа дублированного процесса новым процессом

    // проверяем, что команда является "cd". Отдельные процессы создаются при вызове невстроенных команд оболочки
    if (strcmp(con_args[0], "cd") == 0) 
    {
        // проверяем, указан ли второй аргумент командной строки
        if (con_args[1] != NULL) 
        {
            // меняем директорию на указанную
            if (chdir(con_args[1]) != 0)
            {
                // выводим сообщение об ошибке, если изменение директории не удалось
                printf("\033[35mНе удалось изменить директорию\033[0m\n");

            }// if(chdir(con_args[1]) != 0)
        } 

        else 
        {
            // выводим сообщение об ошибке, если целевая директория не указана
            printf("\033[35mНе указана целевая директория\033[0m\n");

        }// if(con_args[1] != NULL)

        getchar();                   // ожидаем ввода символа
        return;                      // возвращаем обещанное значение без создания дочернего процеесса. Просто меняем директорию
    }// if(strcmp(con_args[0], "cd") == 0)

    pid = fork();                    // в pid ID дочернего процесса. Создаём копию родительского процесса

    // в зависимости от pid текущего рассматриваемого процесса
    switch(pid)
    {
        // аварийное завершение
        case -1:   
            // ошибка форкинга
            printf( "\033[35mОШИБКА при запуске процесса. Код ошибки 4. \033[0m\n");
            exit(0);                 // аварийное завершение
        break;
        
        // дочерний процесс
        case 0: 
            // вызывает функцию execvp(), которая пытается выполнить команду, указанную в con_args
            success = execvp(con_args[0], con_args);

            if(success == -1)        // проверяет, вернул ли execvp() ошибку. Если вернул, значит команда не найдена
            {
                printf("\033[38;5;196mКоманда терминала \'%s\' не найдена \033[0m\n", con_args[0]);
                // прекратить выполнение процесса в случае ошибки
                kill(getpid(), SIGTERM);
            }// if(success == -1)
        break;

        // родительский процесс
        default:
            // добавляем в список PID дочернего процесса
            add_elem(pid);
            printf("\033[38;5;135mДочерний процесс\033[0m = \033[38;5;228m%d\033[0m\n", pid);
            getchar();               // ожидаем ввода символа для разделения ввода команд
        break;

    }// switch()
    return;                          // вернули обещанное значение
}



/*----------------------------------------------------------------------*/
/*  Разделение входной строки на отдельные лексемы */
/*-------------------------------------------------*/

char **word_division(char *line)     // указатель на строку для разделения
{
    int length = strlen(line);       // определение длины строки line
    // выделение памяти для массива указателей на строки, в которых будут храниться отдельные лексемы. Максимальная длина length, если все слова из одной буквы
    char **parts = (char **) malloc((length + 1) * sizeof(char *));
    
    // проверка на успешность выделения памяти
    if (parts == NULL)
    {
        // ошибка выделения памяти
        return NULL;                  // возвращаем NULL-значение указателя
    }// if (parts == NULL)

    int i = 0;                        // вспомогательная переменная для цикла
    int word_start = 0;               // индекс начала текущего слова
    int word_count = 0;               // количество найденных слов

    // пока не дошли до конца строки line
    while (i <= length) 
    {
        // является ли текущий символ пробелом или концом строки
        if (line[i] == ' ' || line[i] == '\0') 
        {
            if (i > word_start)       // были ли найдены символы между текущим символом и предыдущим пробелом
            {
                // вычисление длины текущего слова
                int word_length = i - word_start;
                // выделение памяти для текущей строки в массиве строк на один сивол больше для записи '\0'
                parts[word_count] = (char *) malloc((word_length + 1) * sizeof(char));

                // проверка на успешность выделения памяти
                if (parts[word_count] == NULL)
                {
                    return NULL;      // возвращаем NULL-значение указателя
                }// if (parts[word_count] == NULL)

                // копирование текущего слова из строки line в массив строк parts 
                strncpy(parts[word_count], &line[word_start], word_length);
                // добавление нулевого символа в конец текущей строки, чтобы получить корректную строку
                parts[word_count][word_length] = '\0';
                // увеличение счетчика найденных слов
                word_count = word_count + 1;
            }// if (i > word_start)

            word_start = i + 1;       // обновление индекса начала следующего слова
        }// if(line[i] == ' ' || line[i] == '\0') 

        i = i + 1;                    // увеличение переменной i для перехода к следующему символу в строке
   
    }// while()

    parts[word_count] = NULL;         // присвоение нулевого указателя в последний элемент массива указателей, чтобы указать конец массива строк
    return parts;                     // возврат указателя на массив строк
}



/*----------------------------------------------------------------------*/
/*   Чтение введённой строки    */
/*------------------------------*/

void read_line(char **line)           // передача указателя на указатель на начало массива строки для его изменения
{
    size_t len = 0;                   // переменная для хранения размера буфера
    ssize_t read;                     // хранения количества считанных символов
    printf("\033[38;5;63m$\033[0m ");              // вывод приглашения для пользователя
    read = getline(line, &len, stdin);// считывание строки с ввода пользователя и сохранение в переменную line
    
    // проверка на ошибку считывания
    if (read == -1)
    {
        // вывод сообщения об ошибке
        printf("\033[35mОШИБКА при считывании строки. Код ошибки 1.\033[0m\n");
    } 

    // считывание прошло успешно
    else 
    {
        // находим позицию символа новой строки
        size_t newLinePos = strcspn(*line, "\n"); 
        // заменяем символ новой строки на символ окончания строки
        if ((*line)[newLinePos] == '\n') 
        {
            (*line)[newLinePos] = '\0';
        }// if ((*line)[newLinePos] == '\n') 

        // вывод количества считанных символов и строки
        printf("Было считанно %zu символов: %s\n", read - 1, *line);

        // если считана пустая строка
        if(read -1 == 0)
        {
            printf("\n");             // вывод пустой строки
        }// if(read -1 == 0)

    }// if(read == -1) 

    return;                           // вернули обещанное функцией значение
}