#ifndef SOMP_IO_H
#define SOMP_IO_H

typedef struct Beam Beam;
typedef struct PointForce PointForce;
typedef struct DistributedForce DistributedForce;
typedef struct PointForces PointForces;
typedef struct DistributedForces DistributedForces;


void read_info_cli(FILE * file, Beam * beam, PointForces * pfs, DistributedForces * dfs);
void read_beam_info_cli(FILE * file, Beam * beam);
void read_pointforce_info_cli(FILE * file, PointForce pfs[]);
void read_distributedforce_info_cli(FILE * file, DistributedForce dfs[]);

#ifdef SOMP_IO_IMPLEMENTATION
#include <stdio.h>

void read_info_cli(FILE * file, Beam * beam, PointForces * pfs, DistributedForces * dfs)
{
    (void)beam;
    (void)pfs;
    (void)dfs;

    char * line = NULL;
    size_t line_buffer_size = 0;

    while (!feof(file))
    {
        getline(&line, &line_buffer_size, file);
        //fputs(line, stdout);
    };
}
void read_beam_info_cli(FILE * file, Beam * beam) 
{
    (void)file;
    (void)beam;
    //char buffer[512] = {0};
    //fgetline(buffer, sizeof(buffer), file);
    //puts(buffer);
}
void read_pointforce_info_cli(FILE * file, PointForce pfs[]) 
{
    (void)file;
    (void)pfs;
}
void read_distributedforce_info_cli(FILE * file, DistributedForce dfs[]) 
{
    (void)file;
    (void)dfs;
}

#endif //SOMP_IO_IMPLEMENTATION
#endif // SOMP_IO_H
