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

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <stack>
#include <bitset>

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
    OperatorEnum whichOperator(std::string& input)
    {
        if (input == "FLIP_FLOP") { return OperatorEnum::FLIP_FLOP;}
        if (input == "OR") { return OperatorEnum::OR;}
        if (input == "AND") { return OperatorEnum::AND;}
        if (input == "NOT") { return OperatorEnum::NOT;}
        else { return OperatorEnum::NO_OPERATOR;}
    };

    // Storage of a signal type
    struct signal_data
    {
        OperatorEnum operation;         // The operation to perform
        vector<std::string> arguments;  // The dymamic list of arguments applied to the operation
        bool calculated = false;        // Whether this signal's value has been evaluated
        bool current_value = false;     // The current value of this signal
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
    void getUserInput()
    {
        char token;      

        vector<std::string> all_keys;

        cout << "Provide input according to spec. No error handling" << endl;
        // First get the first line which will tell us how many tokens to keep reading.

        std::string arg1[3];

        token = cin.get();
        while (token != ' ')
        {
            arg1[0] += token;
            token = cin.get();
        }
        token = cin.get(); // next
        while (token != ' ')
        {
            arg1[1] += token;
            token = cin.get();

        }
        token = cin.get(); // next
        while (token != '\n')
        {
            arg1[2]+= token;
            token = cin.get();
        }

        int num_signals = std::stoi(arg1[0]);
        int num_gates = std::stoi(arg1[1]);
        int num_words = std::stoi(arg1[2]);

        cout << "Now collect gate descriptions" << endl;
        // Format "Name = some_string"
        // We will tokenize each one
        for (int i = 0; i < num_gates; i++)
        {
            // Find the key part, ignore spaces
            // 'Name ='
            std::string key;
            struct signal_data temp_signal_data;

            std::vector<std::string> value;

            token = cin.get();
            while (token != '=')
            {
                if (token != ' ')
                {
                    key+=token;
                }
                token = cin.get();            
            }

            // Once we have a key, populate an entry
            signals.insert({key, temp_signal_data});
            
            // Find the value part, make sure to also store the op code
            // ' some_string\n'
            token = cin.get(); // Skip the first space until operand
            while ( token == ' ')
                token = cin.get();
            std::string word; // Temp word token to collect
            // Now start collecting oprand SPACE operator SPACE operand2
            while (token != '\n')
            {
                if (token == ' ')
                {
                    if ( OperatorEnum::NO_OPERATOR == whichOperator(word) )
                    {
                        signals[key].arguments.push_back(word);
                        all_keys.push_back(word); // Keep track of signal operand for later (this is to collect internal signals)
                    }
                    else
                    {
                        signals[key].operation = whichOperator(word);
                    }
                    word = "";
                }
                else if (token != ' ')
                {
                    word+=token;
                }
                token = cin.get();
            }
            // The last word is probably an argument
            if (word.length() > 0)
            {
                if ( OperatorEnum::NO_OPERATOR == whichOperator(word) )
                {
                    signals[key].arguments.push_back(word);
                    all_keys.push_back(word); // keep track of the operands for later
                }
            }
        }

        cout << "Collect internal signals. " << endl;
        for ( auto& key : all_keys )
        {   
            // Anything not in the signals map is an internal signal and we need a signal entry for it
            auto search  = signals.find(key); 
            if (search == signals.end())
            {
                struct signal_data temp;
                temp.operation = NO_OPERATOR;
                signals[key] = temp;
                internal_signal_keys.push_back(key);
            }
        }

        cout << "Now collect final input of signal names to output" << endl;
        for (int i = 0; i < num_words; i++)
        {
            std::string key;
            std::getline(std::cin, key);
            signals_to_display.push_back(key);
        }
    };

    /*
        Purpose: Given signal state, execute one clock cycle according to the boolean logic described by each signal's signal_data

        Description: Outputs beginning state of signals that are marked for output, recalculates the next state, and prints the transition

        Inputs: signals_state, a reference to the current state of signal data

        Outputs: signals_state passed in by reference
    */
    void runOneCycle(std::map <std::string, struct signal_data>& signals_state)
    {
        displaySignals(signals_state);
        cout << " --> ";
        
        // DFS to find the first signal able to be evaluated
        
        std::stack<std::string> theStack;

        theStack.push(signals_state.begin()->first);

        while (theStack.size() > 0)
        {
            struct signal_data & temp = signals_state[theStack.top()];
            std::string st = theStack.top(); // Take a signal from the stack
            theStack.pop();
            if (temp.calculated)
            {
                // Base case
                // See if this signal has been calculated. If so, we will continue to the top and pop another signal
            }
            // If the signal has not been calculated, determine if we can calculate it. If we have operands not yet calculated, push them to the stack
            else
            {
                // Special case : the operation of a FLIP_FLOP is already "complete" from a previous clock cycle
                // (or zero if first iteration)
                if (temp.operation == OperatorEnum::FLIP_FLOP)
                {
                    temp.calculated = true; // This has already been computed from a previous clock cycle (or zero initialized). simply use current_value
                }
                else
                {
                    // See if all operands are available. If they are, calculate the signal value. 
                    //      On the next iteration it will pop and the Base case will be hit.
                    // If not, push the current signal and the operand signal to evaluate to the stack
                    bool args_calculated = true;
                    for ( auto& arg : temp.arguments )
                    {
                        if (!signals_state[arg].calculated)
                        {
                            theStack.push(st); // Push this same node back on, we have not yet calculate its value
                            theStack.push(arg); // Push the argument that needs calculation
                            args_calculated = false;
                            break;
                        }
                    }
                    // Once both (or single) operands are calculated, calculate current signal
                    if (args_calculated)
                    {
                        doOperation(signals_state, temp);
                        temp.calculated = true;
                    }
                }
            }
        }

        // Refresh signal states including fli
        for (auto& signal : signals_state)
        {
            signal.second.calculated = false;
            // Important: refresh any flip flops now
            if (signal.second.operation == OperatorEnum::FLIP_FLOP)
            {
                doOperation(signals_state, signal.second);
            }
        }

        displaySignals(signals_state);
        cout << endl;
    };

    /*
        Purpose: Given input signals, generate all possible input signal values such that all states may be exercised

        Description: Incomplete - This is where my issues lie. I can't seem to generate every possible state.

        Inputs: None

        Outputs: Void
    */
    void generateOutputs()
    {
        // We have 2 ^ internal_signal_keys.length() possible permuations of inputs
        unsigned int max_permuations = 1 << internal_signal_keys.size();
        // (0b000 = 0b1...111) 0b000
        // Generate a look up table of the permuations we want to try. Each permuation is of internal_signal_keys.size()
        static std::vector < std::vector <bool> > lut; // Reduce stack usage
        
        for (unsigned int i = 0;  i < max_permuations; i++)
        {
            std::vector<bool> temp;
            for (unsigned int shift = 0; shift < internal_signal_keys.size(); shift++)
            {
                temp.push_back((i >> (shift & 0b1)) ? true : false);
            }
            lut.push_back(temp);
        }
        for (unsigned int outer = 0; outer < (max_permuations)*(max_permuations); outer++) //  N^2 number of possible states if in each state have two possible inputs
        {
            for (unsigned int kk = 0;  kk < lut.size(); kk++)
            {
                // We need to perform calculations for each possible permutation of internal_signals (beware)
                // Fix intenal signals here
                for (int n = 0; n < internal_signal_keys.size(); n++)
                {
                    signals[internal_signal_keys[n]].current_value = lut[kk][n];
                    signals[internal_signal_keys[n]].calculated = true;
                }
                auto prev_state = signals; // caching the previous state
                runOneCycle(signals);
            }
        }
    }
    
    /*
        Purpose: Given all signals state map and the state to evaluate, perform evaluation

        Description: Most operations rely on internal arguments list which refer to other signals in the
                map which have been presumably evaluated prior to making this call.

        Inputs: signals_state map and the signal_data to update

        Outputs: signals_state and temp (passed in by reference)

        TODO: Move method as struct signal_data's member
    */
    void doOperation(std::map< std::string, struct signal_data>& signals_state, struct signal_data& temp)
    {
        // Attempt operation
        // Giant switch for now, optimize so that arguments list are passed in to a function pointer
        switch (temp.operation)
        {
            case OperatorEnum::FLIP_FLOP:
            {
                temp.current_value = signals_state[temp.arguments[0]].current_value;
                break;
            }
            case OperatorEnum::AND:
            {
                temp.current_value = andGate(signals_state[temp.arguments[0]].current_value, signals_state[temp.arguments[1]].current_value);
                break;
            }
            case OperatorEnum::OR:
            {
                temp.current_value = orGate(signals_state[temp.arguments[0]].current_value, signals_state[temp.arguments[1]].current_value);
                break;
            }
            case OperatorEnum::NOT:
            {
                temp.current_value = notGate(signals_state[temp.arguments[0]].current_value, false);
                break;
            }
            case OperatorEnum::NO_OPERATOR:
            default:
            {
                cout << "Critical failure, operation for this external signal was not defined" << endl;
                break;
            }
        }
    }

    /*
        Purpose: Given all signals state map, display the signals marked for display

        Description: The user argument signals_to_display holds keys for the signals to display

        Inputs: signals_state map

        Outputs: void

        TODO: Move method as struct signal_data's member
    */
    void displaySignals(std::map< std::string, struct signal_data>& signals_state)
    {
        // Iterate thru signal and internal_signal map and print the current_value if display flag is set.
        for ( auto& signal : signals_to_display )
        {
            cout << signals_state[signal].current_value;
        }
    }

    /*
        Purpose: Perform boolean logic given inputs

        Description: None

        Inputs: left operand 'a; and right operand 'b'

        Outputs: bool - the result of the operation
    */
    bool andGate(bool a, bool b)
    {
        return a && b;
    };

    /*
        Purpose: Perform boolean logic given inputs

        Description: None

        Inputs: left operand 'a; and right operand 'b'

        Outputs: bool - the result of the operation
    */
    bool orGate(bool a, bool b)
    {
        return a || b;
    };

    /*
        Purpose: Perform boolean logic given inputs

        Description: None

        Inputs: left operand 'a; and right operand 'b'

        Outputs: bool - the result of the operation
    */
    bool xorGate(bool a, bool b)
    {
        return (a || b) && !(a && b);
    };

    /*
        Purpose: Perform boolean logic given inputs

        Description: Note, b is disregarded to maintain function signature

        Inputs: left operand 'a; and right operand 'b'

        Outputs: bool - the result of the operation
    */
    bool notGate(bool a, bool b)
    {
        return !a;
    };


private:
    std::vector<std::string> signals_to_display;            // Track signal keys that will allowed for display
    std::vector< std::string> internal_signal_keys;         // Track the internal signal keys
    std::map< std::string, struct signal_data> signals;     // Original Signal data
};



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