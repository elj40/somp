#ifndef SOMP_IO_H
#define SOMP_IO_H
#include <stdbool.h>
#include "somp_logic.h"

typedef struct PointForces PointForces;
typedef struct DistributedForces DistributedForces;

bool read_info_cli(FILE * file, Beam * beam, PointForces * pfs, DistributedForces * dfs);
bool read_beam_info_cli(char * line, Beam * beam);
bool read_pointforce_info_cli(char * line, PointForce * p);
bool read_distributedforce_info_cli(char * line, DistributedForce * d);

#ifdef SOMP_IO_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool read_info_cli(FILE * file, Beam * beam, PointForces * pfs, DistributedForces * dfs)
{
    char * line = NULL;
    size_t line_buffer_size = 0;

    getline(&line, &line_buffer_size, file);
    if (strcmp(line, "#B\n") != 0) return false;
    getline(&line, &line_buffer_size, file);
    if (!read_beam_info_cli(line, beam)) return false;

    getline(&line, &line_buffer_size, file);
    if (strcmp(line, "#PF\n") != 0) return false;
    getline(&line, &line_buffer_size, file);
    int count = 0;
    if (sscanf(line, "%d\n", &count) != 1) return false;
    for (int i = 0; i < count; i++)
    {
        PointForce point = {0};
        getline(&line, &line_buffer_size, file);
        if (!read_pointforce_info_cli(line, &point)) return false;
        DynamicArrayAppend(pfs, point);
    };
    
    getline(&line, &line_buffer_size, file);
    if (strcmp(line, "#DF\n") != 0) return false;
    getline(&line, &line_buffer_size, file);
    if (sscanf(line, "%d\n", &count) != 1) return false;
    for (int i = 0; i < count; i++)
    {
        DistributedForce distrib = {0};
        getline(&line, &line_buffer_size, file);
        if (!read_distributedforce_info_cli(line, &distrib)) return false;
        DynamicArrayAppend(dfs, distrib);
    };
    return true;
}
bool read_beam_info_cli(char * line, Beam * beam) 
{
    //#B
    //1.0 10
    if (sscanf(line, "%f %d\n", &beam->length, &beam->sectionsCount) != 2) return false;
    return true;
}
bool read_pointforce_info_cli(char * line, PointForce * p) 
{
    //#PF
    //4
    //0.0  1
    //0.25 2
    //0.5  3
    //1.0  4
    if (sscanf(line, "%f %f\n", &p->distance, &p->force) != 2) return false;
    return true;
}
bool read_distributedforce_info_cli(char * line, DistributedForce * d) 
{
    //#DF
    //4
    //0 0.5 [ 1 0 ]
    //0.25 0.75 [ 2 0 ]
    //0.75 1.0 [ 3 0 ]
    //0.65 0.95 [ 4 0 ]
    
    char * token;
    char * props = strtok(line, "[");
    char * poly = strtok(NULL, "]");

    if (props == NULL || poly == NULL) return false;

    token = strtok(props, " ");
    if (token == NULL) return false;
    d->start = atof(token);
    token = strtok(NULL, " ");
    if (token == NULL) return false;
    d->end = atof(token);
    token = strtok(NULL, " ");
    if (token != NULL) return false; // Expected end of props

    token = strtok(poly, " ");
    int index = 0;
    while (token != NULL && index < MAX_POLYNOMIAL_DEGREE)
    {
        d->polynomial[index] = atof(token);
        token = strtok(NULL, " ");
        index++;
    }

    while (index < MAX_POLYNOMIAL_DEGREE)
    {
        d->polynomial[index] = 0;
        index++;
    }

    return true;
}

#endif //SOMP_IO_IMPLEMENTATION
#endif // SOMP_IO_H
