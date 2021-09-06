#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

void find_p(int n, int p[][n], int v, int j)
{

    int arr[n];
    for (int i = 0; i < n; i++)
    {
        arr[i] = INT_MAX;
    }

    int i1 = j;
    int pos = 0;
    while (p[v][i1] != i1)
    {

        int k = p[v][i1];
        arr[pos] = i1;
        pos++;
        i1 = k;
    }
    for (int i = n - 1; i >= 0; i--)
    {
        if (arr[i] == INT_MAX)
        {
            continue;
        }
        printf("%d  ", arr[i]);
    }
}

void main()
{
    int m, n;

    scanf("%d %d\n", &n, &m);

    int g[n][n];
    int par[n][n];
    int d_vec[n][n];
    int f[n][n];

    int q[n];

    for (int i = 0; i < n; i++)
    {

        for (int j = 0; j < n; j++)
        {
            if (i == j)
            {
                f[i][j] = 0;
                g[i][j] = 0;
                d_vec[i][j] = 0;
                par[i][j] = i;
                continue;
            }
            f[i][j] = 0;
            g[i][j] = 0;
            d_vec[i][j] = INT_MAX;
            par[i][j] = INT_MAX;
        }
    }

    int u, v, w;

    for (int i = 0; i < m; i++)
    {
        scanf("%d %d %d", &u, &v, &w);
        g[u][v] = w;
        g[v][u] = w;
    }

    // Dijstras algo
    int s = 0;
    int t = 0;
    for (int src = 0; src < n; src++)
    {

        f[src][src] = 1;

        while (1)
        {
            int a;
            int empty = 1;
            for (a = 0; a < n; a++)
            {
                if (f[src][a] == 1)
                {
                    empty = 0;
                    break;
                }
            }
            if (empty == 1)
            {
                break;
            }
            if (f[src][a] != 2)
            {
                f[src][a] = 2;
                for (int j = 0; j < n; j++)
                {
                    int w = g[a][j];
                    if (w != 0)
                    {
                        if (d_vec[src][a] != INT_MAX)
                        {
                            if (d_vec[src][a] + w < d_vec[src][j])
                            {
                                d_vec[src][j] = d_vec[src][a] + w;
                                par[src][j] = a;
                                f[src][j] = 1;
                                s++;
                            }
                        }
                    }
                }
            }
        }
    }

    /*for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            printf("%d\t",d_vec[i][j]);
        } 

        printf("\n");
    }*/

    for (int v = 0; v < n; v++)
    {
        printf("Routing table for vertex %d\n", v);

        printf("Dest\t\tCost\t\tPath\n");

        for (int j = 0; j < n; j++)
        {

            /*  if (v == j)
            {
                continue;
            }

        */
            printf("%d\t\t%d\t\t", j, d_vec[v][j]);
            if (d_vec[v][j] != INT_MAX)
            {
                if (v != j)
                {
                    find_p(n, par, v, j);
                }
                else
                {
                    printf("%d (src=dest)", v);
                }
            }
            else
            {
                printf("No Path");
            }
            printf("\n");
        }
    }
}
