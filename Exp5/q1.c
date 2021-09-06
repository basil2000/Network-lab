#include <stdio.h>
#include <string.h>
#include <limits.h>

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
    int ne=2*m;
    int edge[2 * m][3];
    int par[n][n];
    int d_vec[n][n];

    for (int i = 0; i < m; i++)
    {
        scanf("%d %d %d", &edge[i][0], &edge[i][1], &edge[i][2]);
        edge[i + n][0] = edge[i][1];
        edge[i + n][1] = edge[i][0];
        edge[i + n][2] = edge[i][2];
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i == j)
            {
                d_vec[i][j] = 0;
                par[i][j] = i;
                continue;
            }
            d_vec[i][j] = INT_MAX;
            par[i][j] = INT_MAX;
        }
    }

    for (int src = 0; src < n; src++)
    {
        for (int i = 0; i < n - 1; i++)
        {
            for (int j = 0; j < ne; j++)
            {
                int u = edge[j][0];
                int v = edge[j][1];
                int w = edge[j][2];
                if (d_vec[src][u] == INT_MAX)
                {
                    continue;
                }
                if (d_vec[src][v] > d_vec[src][u] + w)
                {
                    d_vec[src][v] = d_vec[src][u] + w;
                    par[src][v] = u;
                }
            }
        }
    }
    /*
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            printf("%d\t",par[i][j]);
        } 

        printf("\n");
    }
    */

    for (int v = 0; v < n; v++)
    {
        printf("Routing table for vertex %d\n", v);

        printf("Dest\t\tCost\t\tPath\n");

        for (int j = 0; j < n; j++)
        {

            /*   if (v == j)
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