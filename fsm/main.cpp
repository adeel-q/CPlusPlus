/*
    Purpose: FSM implmentation

    Description: Outputs are organized in the following format and piped into stdin:
        num_signals, num_gates, num_words_to_display
        <gate_name> = <gate_description>
            ...
        <gate_name> = <gate_description>
        <gate_name>
        <gate_name>

    Author: Adeel Qursehi
    Date : 8/25/2021

    TODO: Move class implementation to header/source files!
*/

#include "fsm.h"

using namespace std;

/*
Example:

a = b OR c
d = FLIP_FLOP a
b = input_1 AND d
c = NOT d
d is initially 0



0 --> 1
1 --> 0
1 --> 1
*/

// Entry point
int main(int argc, char *argv[])
{
    Fsm fsm;
    fsm.getUserInput();
    fsm.generateOutputs();
    
    return 0;
}