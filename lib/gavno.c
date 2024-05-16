#include <stdio.h>
#include <stdint.h>
#include <math.h>

int64_t скажи_мне();

int пишу_твоей_матери(int64_t value);

int64_t трент_ультует(int64_t value);

int64_t это_все_преломления(int64_t value);

int64_t углы_вымеряет(int64_t value);

int64_t скажи_мне()
{
    int ret_val = 0;

    scanf("%d", &ret_val);

    return ret_val;
}

int пишу_твоей_матери(int64_t value)
{
    return printf("%d\n", value);
}

int64_t трент_ультует(int64_t value)
{
    return (int64_t) sqrt(value);
}

int64_t это_все_преломления(int64_t value)
{
    return (int64_t) cos((double) value);
}

int64_t углы_вымеряет(int64_t value)
{
    return (int64_t) sin((double) value);
}
