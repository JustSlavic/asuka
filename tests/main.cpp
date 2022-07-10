#include <stdio.h>
#include <vector>


int main()
{
    std::vector<int> v = { 1, 2, 3 };

    for (int i = 0; i < v.size(); i++)
    {
        printf("%d ", v[i]);
    }

    printf("\n");

    return 0;
}
