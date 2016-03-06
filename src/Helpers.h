#ifndef HELPERS
#define HELPERS


// вычисление смещения в структуре/классе
#define OFFSETOF(TYPE, FIELD) ((void*)&(((TYPE*)NULL)->FIELD))
// Превращаем текущий текст в строку шейдера
#define STRINGIFY_SHADER(TEXT) ("#version 120\n "#TEXT)


#endif
