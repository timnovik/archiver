# Архиватор

Программа-архиватор имеет следующий интерфейс командной строки:
* `archiver -c archive_name file1 [file2 ...]` - заархивировать файлы `file1, file2, ...` и сохранить результат в файл `archive_name`.
* `archiver -d archive_name` - разархивировать файлы из архива `archive_name` и положить в текущую директорию.
* `archiver -h` - вывести справку по использованию программы.

Имена файлов (только имена файлов с расширениями, без дополнительного пути) сохраняются при архивации и разархивации.

# Алгоритм

Алгоритм сжатия устроен следующим образом:
1. Подсчитывается частотность 8-битных символов в файле. Кроме содержимого файла надо учесть частоты символов в имени файла, а также добавить три служебных символа `FILENAME_END=256`, `ONE_MORE_FILE=257`, `ARCHIVE_END=258` с частотами 1. Назначение этих символов будет описано позже. Таким образом, для кодирования расширенного алфавита необходимо 9 бит.
2. Строится бинарный [бор](https://en.wikipedia.org/wiki/Trie) кодирования следующей процедурой:
    1. Для каждого символа алфавита добавляется соответствующая вершина в очередь с приоритетом. Упорядочение вершин в очереди осуществляется по неубыванию частот символов в файле (в "начале" очереди всегда вершина с символом с наименьшей встречаемостью в файле), а при равенстве частот - по возрастанию символов (для вершин, не являющихся листьями, в качестве сивола для сравнения используется наименьший из символов их потомков).
    1. Пока в очереди больше одного элемента, из нее последовательно извлекаются две вершины A и B с минимальными приоритетами. Создается новая вершина С, детьми которой являются вершины A и B.
       Вершина C помещается в очередь с приоритетом, равным сумме приоритетов вершин A и B.
    1. По окончанию процедуры в очереди остается ровно одна вершина, которая является корнем построенного бора. Листовые вершины являются терминальными.
       В каждой терминальной вершине записан символ из исходного файла.
       Каждая нетерминальная вершина дерева содержит два ребра: левое и правое, которые помечаются битами 0 и 1 соответственно.
       Каждой терминальной вершине соответствует битовая последовательность, получающаяся спуском из корня бора к терминальной вершине и выписыванием битов всех пройденных ребер.
       Для наглядности можно воспользоваться следующими иллюстрациями:
    * [gif demo](https://commons.wikimedia.org/wiki/File:Huffmantree_ru_animated.gif?uselang=ru)
    * [gif demo](https://commons.wikimedia.org/wiki/File:Huffman_huff_demo.gif)
    * [graphic demo](https://people.ok.ubc.ca/ylucet/DS/Huffman.html).
3. Всем символам ставится в соответствие бинарная кодовая последовательность посредством построенного бора.
4. Код приводится к [канонической форме](https://en.wikipedia.org/wiki/Canonical_Huffman_code). Каноническая форма кода отличается тем, что позволяет однозначно восстановить коды по списку символов и длинам кодов для них. Алгоритм восстановления канонического кода есть [в википедии](https://en.wikipedia.org/wiki/Canonical_Huffman_code).
5. Все символы файла заменяются на соответствующие кодовые бинарные последовательности, и результ записывается вместе со вспомогательной информацией в файл. Формат файла архива описан ниже.

Алгоритм декодирования в целом обратен алгоритму кодирования и устроен следующим образом:
1. Из файла восстанавливается таблица кодирования (соответствие между сиволами и их кодами).
2. По таблице кодирования строится бинарный бор.
3. По бинарным последовательностям, прочитанным из входного файла, производится трассировка по бору от корня к листовым вершинам. При достижении очередной листовой вершины бора определяется соответсвующий ей символ, который записывается в выходной файл.

# Примечание

Этот проект я реализовал как задание в рамках своего обучения на первом курсе ФКН ВШЭ по программе ПМИ.

