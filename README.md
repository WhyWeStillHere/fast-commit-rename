## fast-commit-rename
Fast git rebase --interactive analog, for renaming commits

# Requirements 
* libgit2 v1.0.0
# Installation
```
gcc main.c -o git-fast-reword $(pkg-config libgit2)
```
## Usage
```
./git-fast-reword <commit-ref> <new commit message> [repository adress]
```
Default repository address is current folder

## О реализации
Программа берет ветку на которую указывает HEAD. Находит переданный ей коммит,меняем комментарий, далее меняем все последующие коммиты по порядку от него (т.к. при перезаписи текущего коммита мы должны переписать всю последующую истроию, т.к. хеши коммитов операются на хеши их предков). После этого мы заменяем старую ветку на новую с тем же названием, и перевешиваем HEAD

## О скорости
Скрипт приложенный к репозиторию позволяет быстро создать директорию с указанным именем и создать там 20 коммитов, каждый коммит добавляет 2 файла по 10M. Файлы рандомизированны, поэтому не возникнет совпадения хешей и все они будет добавлены в хранилище гита как отдельные файлы.
Далее сравнение скорости изменения текста коммитов (Проводилость с помощью команды time)
В git rebase -i  <commit-ref> изменялся комментарий к самому раннему коммиту. Так же на real время, смотреть нерелевантно, т.к. при rebase уходит время на работу в текстовом редакторе
```
$ time ./git-fast-reword HEAD~10 "HEAD~10" test_repo/
real	0m0,163s
user	0m0,146s
sys	0m0,016s

$ time git rebase -i HEAD~11
real	0m46,002s
user	0m2,828s
sys	0m0,247s
```
```
$ time ./git-fast-reword HEAD~18 "HEAD~18" test_repo/
real	0m0,022s
user	0m0,018s
sys	0m0,004s
$ time git rebase -i HEAD~19
real	0m22,489s
user	0m3,907s
sys	0m0,456s
```
```
$ time ./git-fast-reword HEAD "HEAD" test_repo/
real	0m0,208s
user	0m0,156s
sys	0m0,037s
$ time git rebase -i HEAD~1
real	0m14,492s
user	0m0,340s
sys	0m0,175s
```
