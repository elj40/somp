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


/**
 * Reads info about a beam from a file stream which can also be stdin
 * Fills out the beam, pfs and dfs parameters with info read from file
 * Distances are measured from left of beam
 * Format:
 *  #B (beam section)
 *  (length of beam: float) [max number of sections the beam could have: int]
 *  #PF (point force section)
 *  (distance: float) (force: float)
 *  ....
 *  #DF (distributed force section)
 *  (start dist: float) (end dist: float) [ (coeff. of x^0: float) (coeff. of x^1) ... ]
 *  ....
 *  \n or EOF
*/
bool read_info_cli(FILE * file, Beam * beam, PointForces * pfs, DistributedForces * dfs)
{
    //TODO: this should become a state machine so point force or distrib force
    //sections arent required
    //TODO: exit on ^D
    char * line = NULL;
    int line_num = 1;
    size_t line_buffer_size = 0;
#define FAIL(l, ln) do { printf(">> Failed reading input on line %d\n", (ln)); free((l)); return false; } while(0)

    getline(&line, &line_buffer_size, file); line_num++;
    if (strcmp(line, "#B\n") != 0) FAIL(line, line_num);
    if (getline(&line, &line_buffer_size, file) == -1) FAIL(line, line_num);
    line_num++;
    if (!read_beam_info_cli(line, beam)) FAIL(line, line_num);

    if (getline(&line, &line_buffer_size, file) == -1) FAIL(line, line_num);
    line_num++;
    if (strcmp(line, "#PF\n") != 0) FAIL(line, line_num);
    while (getline(&line, &line_buffer_size, file) != -1 && line[0] != '#')
    {
        line_num++;
        PointForce point = {0};
        if (!read_pointforce_info_cli(line, &point)) FAIL(line, line_num);
        DynamicArrayAppend(pfs, point);
    };

    if (strcmp(line, "#DF\n") != 0) FAIL(line, line_num);
    while (getline(&line, &line_buffer_size, file) != -1 && line[0] != '\n')
    {
        line_num++;
        DistributedForce distrib = {0};
        if (!read_distributedforce_info_cli(line, &distrib)) FAIL(line, line_num);
        DynamicArrayAppend(dfs, distrib);
    };

    free(line);
    return true;
}
bool read_beam_info_cli(char * line, Beam * beam) 
{
    //#B
    //1.0 10
    int num_reads = sscanf(line, "%f %d\n", &beam->length, &beam->sections_count);
    if (num_reads == 1) beam->sections_count = MAX_SECTIONS;
    if (num_reads < 1 || num_reads > 2) return false;
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
