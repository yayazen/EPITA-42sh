#include <utils/vec.h>

void vec_pushstr(struct vec *vec, const char *s)
{
    char c;
    while ((c = *s++) != '\0')
        vec_push(vec, c);
}
