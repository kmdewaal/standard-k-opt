#include <iostream>
#include <stdio.h>

int main(int argc, char **argv)
{
    int nd = 4;

    if (argc > 1)
    {
        if (sscanf(argv[1], "%d", &nd) == 1)
        {
            // OK
        }
        else
        {
            printf("Usage: %s  or \'%s 20\'  for 20x20 points\n", argv[0], argv[0]);
            return -1;
        }
    }

    int dim = nd*nd;
    int ip = 0;

    double scale = 1000;            // Scale factor to get sizes in mm, needed for algorithm termination
    double alen = 0.25;
    double step = alen/nd;

    printf("TYPE : TSP\n");
    printf("DIMENSION : %d\n", dim);
    printf("NODE_COORD_SECTION\n");
    for (int i=0; i<nd; i++)
    {
        for (int j=0; j<nd; j++)
        {
            ip++;
            double px = (-0.125 + i*step) * scale;
            double py = (-0.125 + j*step) * scale;
            printf("%d %10.6f %10.6f\n", ip, px, py);
        }
    }
}

