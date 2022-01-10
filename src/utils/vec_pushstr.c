#include <utils/vec.h>

void vec_pushstr(struct vec *vec, const char *s)
{
    if (s == NULL)
        return;

    char c;
    while ((c = *s++) != '\0')
        vec_push(vec, c);
}
