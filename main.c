#include <stdio.h>                   // подключение стандартной библиотеки ввода/вывода
#include <stdlib.h>                  // содержит в себе функции, занимающиеся выделением памяти
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>                 // функции для работы с типом данных bool
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>

// дочерние процессы для терминала будут добавляться в структуру
// в языке C для объявления указателя на структуру пишут слово struct
struct PID_list
{
    pid_t PID;
    struct PID_list *next_process;
};


struct PID_list *head_list;



// добавление нового элемента (дочернего процесса) в список
void add_elem (pid_t new_PID)                                  // PID добавляемого процесса
{
    // выделяем память под новый экземпляр списка
    struct PID_list *new_list = (struct PID_list *)malloc(sizeof(struct PID_list));
    new_list->PID = new_PID;
    new_list->next_process = NULL;

    // начало списка - изначально список путой
    if(head_list == NULL)
    {
        head_list = new_list;
    }

    // список уже не пустой
    else
    {
        // объявляем указатель на список для прохода до конца списка
        struct PID_list *current = head_list;
        // следующий элемент списка существует
        while(current->next_process != NULL)
        {
            // переходим на него
            current = current->next_process;
        }

        // связываем старый список с новым добавляемым элементом списка
        current->next_process = new_list;
    }
}



// удаление элемента из начала списка для FIFO
void delete_head_list()
{
    if(head_list == NULL)
    {
        return;                       // список пуст
    }
    // объявили указатель на структуру списка
    struct PID_list *current = head_list;
    // сместили указатель начала списка после удаления элемента
    head_list = current->next_process;
}



void Ctrl_plus_C(int sig)
{
    if(head_list == NULL)
    {
        printf("\nУбит родительский процесс с PID %d\n", getpid());
        kill(getpid(), SIGTERM);
    }

    else
    {
        // печатаем дочерние процессы, чтобы было видно, какой завершаем
        struct PID_list *current = head_list;
        int i = 1;
        printf("\nДочерние процессы родительского процесса %d:\n", getpid());
        while(current != NULL)
        {
            printf("%i. Дочерний процесс PID: %d\n", i, current->PID);
            current = current->next_process;
            i = i + 1;
        }

        if(head_list != NULL)
        {
            printf("Убит процесс %d\n", head_list->PID);
            kill(head_list->PID, SIGTERM);
        }
        delete_head_list();
    }
    printf("\nКоманда $> ");
    fflush(stdout);

    return;
}



void create_new_process(char **con_args)
{
    pid_t pid;
    pid_t waiting;
    int status;
    int success = -1;

    // Проверяем, что команда является "cd". Отдельные процессы создаются при вызове невстроенных команд оболочки
    if (strcmp(con_args[0], "cd") == 0) 
    {
        if (con_args[1] != NULL) 
        {
            if (chdir(con_args[1]) != 0) 
            {
                printf("Не удалось изменить директорию\n");
            }
        } 

        else 
        {
            printf("Не указана целевая директория\n");
        }
        getchar();
        return;
    }

    pid = fork();     // в pid ID дочернего процесса

    switch(pid)
    {
        case -1:
            // ошибка форкинга
            printf( "Ошибка при запуске процесса");
            // аварийное завершение
            exit(0);
        break;
        
        // дочерний процесс
        case 0: 

            success = execvp(con_args[0], con_args);
            if(success == -1)
            {
                printf("Команда терминала \'%s\' не найдена \n", con_args[0]);
                kill(getpid(), SIGTERM);
            }
        break;

        // родительский процесс
        default:
        // с waitpid для автозакрывания дочерних процессов
            while (!WIFEXITED(status) && (waiting = wait(&status)) != -1) 
            {
                printf("Дочерний процесс = %d\n", waiting);
            }

            pid_t new_pid = -1;
            if(waiting != 0 && waiting != -1)
            {
                new_pid = waiting;
            }
            else
            {
                new_pid = getpid();
            }
            add_elem(new_pid);
            getchar();
        break;

    }
    
}



// разделение входной строки на отдельные лексемы
char **word_division(char *line)
{
    int length = strlen(line);
    char **parts = (char **) malloc((length + 1) * sizeof(char *));
    
    if (parts == NULL) 
    {
        // обработайте ошибку выделения памяти
        return NULL;
    }

    int i = 0;
    int j = 0;
    int word_start = 0;
    int word_count = 0;

    while (i <= length) 
    {
        if (line[i] == ' ' || line[i] == '\0') 
        {
            if (i > word_start) 
            {
                int word_length = i - word_start;
                parts[word_count] = (char *) malloc((word_length + 1) * sizeof(char));
               
                if (parts[word_count] == NULL) 
                {
                    // обработайте ошибку выделения памяти
                    return NULL;
                }
                strncpy(parts[word_count], &line[word_start], word_length);
                parts[word_count][word_length] = '\0';
                word_count++;
            }
            word_start = i + 1;
        }
        i = i + 1;
    }

    parts[word_count] = NULL;
    return parts;
}



void read_line(char **line)
{
    size_t len = 0;
    ssize_t read;
    printf("Команда $> ");
    read = getline(line, &len, stdin);
    
    if (read == -1) 
    {
        printf("Ошибка при считывании строки\n");
    } 
    else 
    {
        // находим позицию символа новой строки
        size_t newLinePos = strcspn(*line, "\n"); 
        // убираем символ перевода строки
        // заменяем символ новой строки на символ окончания строки
        if ((*line)[newLinePos] == '\n') 
        {
            (*line)[newLinePos] = '\0';
        }// if ((*line)[newLinePos] == '\n') 

        printf("Было считанно %zu символов: %s\n", read - 1, *line);

        if(read -1 == 0)
        {
            printf("\n");
        }
    }
    return;
}



int main()
{
    char *line = NULL;                // PID процесса
    char **con_args;

    // сигнал на удаление
    signal(SIGINT, Ctrl_plus_C);
    printf("\n");

    while(true)
    {
        read_line(&line);
        
        if(strcmp(line, "stop") == 0)   // выход из терминала
        {
            break;
        }

        // показать PID родительского процесса
        if(strcmp(line, "parent") == 0)
        {
            printf("PID родительского процесса: %d\n", getpid());
            printf("PID родительского процесса для родителя: %d\n", getppid());
            char cwd[256];
            if (getcwd(cwd, sizeof(cwd)) != NULL) 
            {
            printf("Текущий рабочий каталог: %s\n", cwd);
            } 
            else 
            {
            perror("Ошибка при получении текущего рабочего каталога");
            }
            printf("\n");
        }

        else if(strcmp(line, "child") == 0)
        {
            if(head_list == NULL)
            {
                printf("У родителя %d нет детей \n", getpid());
            }
            else
            {
                struct PID_list *current = head_list;
                int i = 1;
                printf("Дочерние процессы родительского процесса %d:\n", getpid());
                while(current != NULL)
                {
                    printf("%i. Дочерний процесс PID: %d\n", i, current->PID);
                    printf("   PID родительского процесса для дочернего: %d\n", getpid());
                    char cwd[256];
                    if (getcwd(cwd, sizeof(cwd)) != NULL) 
                    {
                        printf("   Текущий рабочий каталог: %s\n", cwd);
                    } 
                    else 
                    {
                        perror("Ошибка при получении текущего рабочего каталога");
                    }

                    current = current->next_process;
                    i = i + 1;
                }
            }
            printf("\n");
        }

        else if (line[0] != '\0') 
        {
            con_args = word_division(line);
            create_new_process(con_args);
        }
    }


    return 0;
}