// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdio.h>
#include <stdlib.h>
#include "../Aquarium.hpp"

BOOL main_action(Message__ *m) {
    int i;

    Typeless_Vector__ *tv = (Typeless_Vector__*)m->args[0].VoidPtr;

    printf("Number of cmd_line_args: %i\n", tv->current_size);

    for (i = 0; i < tv->current_size; ++i) {
        char *arg = INDEX_AT__(tv, i, char*);
        printf("Arg %i: %s\n", i, arg);
    }

    exit(0);
}
int main(int argc, char *argv[]) {
    aquarium_main__(argc, argv, main_action, TRUE);

    return 0;
}
