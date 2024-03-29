# Управление процессом

---

## Задание: 

1. Написать программу «терминал», которая анализирует входную строку и при обнаружении ключевых слов «ls» и «cat» запускает соответствующие процессы.

2. В программе реализовать возможность запуска процессов других программ, например, браузера.

3. Написать обработчики сигналов, например, при получении сигнала CTRL+C завершить запущенный программой процесс.

4. Заменить системный bash на собственный терминал.

## Дополнительно:

Запустить 3 программы через терминал, сигнал CTRL+C должен закрывать программы в последовательности FIFO

---

**Литература:** Основы программирования в Linux. Автор: Мэтью Нейл, Стоунс Ричард. Глава 11. Процессы и сигналы.

---
### Интересные команды:

1. /home/your_username/.local/share/Trash *- перейти в директорию корзины*
2. gio trash [FILE or DIR]                *- отправить файл/директорию в корзину*
3. gio trash --empty                      *- очистить корзину*
---
1. google-chrome www.example.com          *-открыть браузер,например chrome, по заданному URL*
2. google-chrome http://www.google.com </dev/null >/dev/null 2>&1 & disown - *-открыть браузер и перейти из терминала*
---
1. bc                                     *-запуск встроенного калькулятора Linux*
2. quit                                   *-выйти из встроенного калькулятора Linux*
---
1. killall <имя процесса>                 *-закрывает все процессы с заданным именем
2. killall                                *-закрывает все дочерние процессы для данного терминала и останавливает его работу
3. kill <PID>                             *-удаляет процесс по заданному PID, но не убирает его из списка дочерних процессов
---
1. renice -n 10 -p <PID>                  *-изменяет приоритет процесса со значения по умолчанию
2. ps -eo pid,rtprio,ni,cmd               *-посмотреть все основные параметры по процессу, в том числе и приоритеты
---
1. gcc -o my_terminal main.c              *-создать исполняемый файл
2. sudo cp my_terminal /usr/local/bin/    *-копирование исполняемого файла в системыный путь
3. /usr/local/bin/my_terminal             *-запуск нового терминала из системного пути, куда он был помещён ранее
