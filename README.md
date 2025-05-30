*AdvancedVector* — более продвинутая версия *SimpleVector*. Здесь используется обёртка для работы с динамической сырой памятью *RawMemory*, а сам контейнер хранит лишь информацию о его текущем размере. Этот вектор более эффективен, использует меньше памяти и меньшее количество операций копирования, а также позволяет использовать в качестве шаблонного параметра типы без конструктора по умолчанию.

*AdvancedVector* обладает следующим функционалом:

1. Конструктор по умолчанию для пустого вектора;
2. Конструктор из *size* элементов;
3. Конструктор копирования;
4. Конструктор перемещения;
5. Оператор копирующего присваивания;
6. Оператор копирующего перемещения;
7. Get методы получения информации о размере и вместимости;
8. Конс тантный и неконстантный доступ к элементу вектора через *operator[]*;
9. Изменение размера вектора (метод *Resize*);
10. Добавление элемента вектора в конец (методом *PushBack*);
11. Добавление элемента перемещением в конец вектора;
12. Добавление элемента методами *EmplaceBack* и *Emplace*;
13. Удаление элемента вектора с конца (методом *PopBack*);
14. Вставка значения в позицию *pos* (метод *Insert*);
15. Удаление элемента из позиции *pos* (метод *Erase*);
16. Константные и неконстантные методы получения итераторов начала и конца вектора;
17. Обмен содержимым между векторами (метод *Swap*);
18. Резервирование места под элементы вектора (метод *Reserve*).