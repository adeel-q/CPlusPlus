#ifndef Fsm_h
#define Fsm_h

#include <vector>
#include <string>
#include <map>

using namespace std;

class Fsm
{
public:

    Fsm() {};

    // Enum describing the type of operation
    enum OperatorEnum
    {
        NO_OPERATOR=0,
        FLIP_FLOP,
        OR,
        AND,
        NOT,
        XOR,
    };

    // Mapping of raw input token against known possible operation types.
    OperatorEnum whichOperator(std::string& input);

    // Storage of a signal type
    struct signal_data
    {
        OperatorEnum operation;          // The operation to perform
        vector<std::string> arguments;   // The dymamic list of arguments applied to the operation
        bool calculated = false;         // Whether this signal's value has been evaluated
        bool current_value = false;      // The current value of this signal
        bool previous_value = false;     // The previous value of this signal
        bool operator==(struct signal_data& other)
        {
            return other.current_value == current_value;
        };
    };

    /*
        Purpose: Tokenize and obtain user input as described by the specification.

        Description: Outputs are organized in the following format and piped into stdin:
            num_signals, num_gates, num_words_to_display
            <gate_name> = <gate_description>
            ...
            <gate_name> = <gate_description>
            <gate_name>
            <gate_name>

        Inputs: stdin

        Outputs: void
        // TODO: Make an argument here that excepts any type of stream buffer (including std::cin) so you can pass in file streams
    */
    void getUserInput();

    /*
        Purpose: Given signal state, execute one clock cycle according to the boolean logic described by each signal's signal_data

        Description: Outputs beginning state of signals that are marked for output, recalculates the next state, and prints the transition

        Inputs: signals_state, a reference to the current state of signal data

        Outputs: signals_state passed in by reference
    */
    void runOneCycle(std::map <std::string, struct signal_data>& signals_state);

    /*
        Purpose: Given input signals, generate all possible input signal values such that all states may be exercised

        Description:    Permutes the inputs for every state transtiion possible. Attempts to transtiion state.
                        If the current state has already been transitioned before, it will be ignored. We can tell
                        which state has been transitioned by generating a bitstring tracking individual signal values.
                        // TODO: This isn't working 100%, but I am able to generate all states at least once.
        Inputs: None

        Outputs: Void
    */
    void generateOutputs();
    
    /*
        Purpose: Given all signals state map and the state to evaluate, perform evaluation

        Description: Most operations rely on internal arguments list which refer to other signals in the
                map which have been presumably evaluated prior to making this call.

        Inputs: signals_state map and the signal_data to update

        Outputs: signals_state and temp (passed in by reference)

        TODO: Move method as struct signal_data's member
    */
    void doOperation(std::map< std::string, struct signal_data>& signals_state, struct signal_data& temp);

    /*
        Purpose: Given a signals state map, output the bitstring representing their values

        Description: The user argument signals_to_display holds keys for the signals to display

        Inputs: signals_state map

        Outputs: std::string represetning all concatenated signal values.
    */
    std::string getSignalStr(std::map< std::string, struct signal_data>& signals_state);

    /*
        Purpose: Given all signals state map, display the signals marked for display

        Description: The user argument signals_to_display holds keys for the signals to display

        Inputs: signals_state map

        Outputs: void

        TODO: Move method as struct signal_data's member
    */
    void displaySignals(std::map< std::string, struct signal_data>& signals_state);

    /*
        Purpose: Perform boolean logic given inputs

        Description: None

        Inputs: left operand 'a; and right operand 'b'

        Outputs: bool - the result of the operation
    */
    inline bool andGate(bool a, bool b)
    {
        return (a && b);
    };

    /*
        Purpose: Perform boolean logic given inputs

        Description: None

        Inputs: left operand 'a; and right operand 'b'

        Outputs: bool - the result of the operation
    */
    inline bool orGate(bool a, bool b)
    {
        return (a || b);
    };

    /*
        Purpose: Perform boolean logic given inputs

        Description: None

        Inputs: left operand 'a; and right operand 'b'

        Outputs: bool - the result of the operation
    */
    inline bool xorGate(bool a, bool b)
    {
        return (a || b) && !(a && b);
    };

    /*
        Purpose: Perform boolean logic given inputs

        Description: Note, b is disregarded to maintain function signature

        Inputs: left operand 'a; and right operand 'b'

        Outputs: bool - the result of the operation
    */
    inline bool notGate(bool a, bool b)
    {
        return !a;
    };


private:
    std::vector<std::string> signalsToDisplay;            // Track signal keys that will allowed for display
    std::vector< std::string> internalSignals;         // Track the internal signal keys
    std::map< std::string, struct signal_data> signals;     // Original Signal data
};

#endif