#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#define ELNOB_IMPLEMENTATION
#include "elnob.h"

int main(int argc, const char * argv[])
{
    if (!elnob_rebuild_elnob(argc, argv)) return 1;

    elnob_run_msg();

    Command cmd = {0};
    elnob_da_append(&cmd, "ls");
    elnob_da_append(&cmd, "-a");
	elnob_run_command_sync(cmd);
    
    cmd.count = 0;
    elnob_cmd_append(&cmd, "ls");
    elnob_cmd_append(&cmd, "-a");
	elnob_run_command_sync(cmd);

    cmd.count = 0;
    elnob_cmd_append_many(&cmd, "ls", "-a");
	elnob_run_command_sync(cmd);

	return 0;
}
