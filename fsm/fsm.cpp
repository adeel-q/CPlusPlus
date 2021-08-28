#include "fsm.h"
#include <iostream>

#include <stack>
#include <queue>
#include <set>


using namespace std;

// Mapping of raw input token against known possible operation types.
Fsm::OperatorEnum Fsm::whichOperator(std::string& input)
{
    if (input == "FLIP_FLOP") { return OperatorEnum::FLIP_FLOP;}
    if (input == "OR") { return OperatorEnum::OR;}
    if (input == "AND") { return OperatorEnum::AND;}
    if (input == "NOT") { return OperatorEnum::NOT;}
    if (input == "XOR") { return OperatorEnum::XOR;}
    else { return OperatorEnum::NO_OPERATOR;}
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
void Fsm::getUserInput()
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
void Fsm::runOneCycle(std::map <std::string, struct signal_data>& signals_state)
{
    // cout << getSignalStr(signals_state) << endl;
    cout << "Pre-conditions: " << endl;
    for (auto& signal : signals_state)
    {
        cout << signal.first  << "=" << signal.second.current_value << endl;
    }

    displaySignals(signals_state);
    cout << " --> ";
    
    // DFS to find the first signal able to be evaluated
    
    std::stack<std::string> theStack;

    for (auto signal : signals_state)
    {
        theStack.push(signal.first);
    }

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
        if (temp.operation == OperatorEnum::FLIP_FLOP)
        {
            temp.calculated = true; // This has already been computed from a previous clock cycle (or zero initialized). simply use current_value
        }
        // If the signal has not been calculated, determine if we can calculate it. If we have operands not yet calculated, push them to the stack
        else
        {
            if (temp.operation != OperatorEnum::NO_OPERATOR)
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
    
    auto copy = signals_state; // Maintain the original signal values

    // Refresh signal states to non-calculated state. Refresh all FF's to new values and calculated state
    for (auto& signal : signals_state)
    {
        // Important: refresh any flip flops now
        if (signal.second.operation == OperatorEnum::FLIP_FLOP)
        {
            signal.second.current_value = copy[signal.second.arguments[0]].current_value;
            signal.second.calculated = true;
        }
        else
        {
            signal.second.calculated = false;
        }
    }

    displaySignals(signals_state);
    cout << endl << endl;
    // cout << getSignalStr(signals_state) << endl << endl;
};


/*
    Purpose: Given input signals, generate all possible input signal values such that all states may be exercised

    Description:    Permutes the inputs for every state transtiion possible. Attempts to transtiion state.
                    If the current state has already been transitioned before, it will be ignored. We can tell
                    which state has been transitioned by generating a bitstring tracking individual signal values.
                    // TODO: This isn't working 100% to ignore don't care states
    Inputs: None

    Outputs: Void
*/
void Fsm::generateOutputs()
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

    // For every permutation we generate a signal state. 
    // For every successive signal state, we must again permuate the inputs to generate the next signal state
    // We will have a lot of signal states so we reduce the amount by checking if the resultant next state has already ran through runOneCycle 
    auto start_state = signals; // Entry point, the initial state

    std::queue<std::map< std::string, struct signal_data>> theQueue; // Queue to hold the subsequent resultant arrays to runOneCycle on

    std::set<std::string> states_visited; // Set to track which state we have already attempted to runOneCycle on

    theQueue.push(start_state); // Begin by pushing the first state
    

    // cout << "start state " << getSignalStr(start_state) << endl;

    while (theQueue.size() > 0)
    {
        auto curr_state = theQueue.front(); // Deque state
        theQueue.pop();
        for (unsigned int kk = 0;  kk < lut.size(); kk++)
        {
            auto next_state = curr_state; // Save a version of the current state to permute
            // We need to perform calculations for each possible permutation of internal_signals (beware)
            // Fix intenal signals here

            for (int n = 0; n < internal_signal_keys.size(); n++)
            {
                next_state[internal_signal_keys[n]].current_value = lut[kk][n];
                next_state[internal_signal_keys[n]].calculated = true;
            }

            // Check if we have ran this state before
            auto str = getSignalStr(next_state);
            auto search = states_visited.find(str);
            if (search != states_visited.end()) 
            {
                // cout << "exists in set " << *search << endl << endl;
            } 
            else 
            {
                states_visited.insert(str); // This is now a state we are about to transition FROM, add to the set
                // cout << "add to set " << str << endl << endl;;
                // Populate next_state here
                runOneCycle(next_state);
                theQueue.push(next_state); // Push this state to calculate new resultant transitions
            }
        }
    }
};


/*
    Purpose: Given all signals state map and the state to evaluate, perform evaluation

    Description: Most operations rely on internal arguments list which refer to other signals in the
            map which have been presumably evaluated prior to making this call.

    Inputs: signals_state map and the signal_data to update

    Outputs: signals_state and temp (passed in by reference)

    TODO: Move method as struct signal_data's member
*/
void Fsm::doOperation(std::map< std::string, struct signal_data>& signals_state, struct signal_data& temp)
{
    // Attempt operation
    // Giant switch for now, optimize so that arguments list are passed in to a function pointer
    switch (temp.operation)
    {
        case OperatorEnum::FLIP_FLOP:
        {
            temp.current_value;
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
};


/*
    Purpose: Given a signals state map, output the bitstring representing their values

    Description: The user argument signals_to_display holds keys for the signals to display

    Inputs: signals_state map

    Outputs: std::string represetning all concatenated signal values.
*/
std::string Fsm::getSignalStr(std::map< std::string, struct signal_data>& signals_state)
{
    std::string ret;
    for ( auto& signal : signals_state )
    {
        ret += signal.second.current_value ? "1" : "0";
    }
    return ret;
};

/*
    Purpose: Given all signals state map, display the signals marked for display

    Description: The user argument signals_to_display holds keys for the signals to display

    Inputs: signals_state map

    Outputs: void

    TODO: Move method as struct signal_data's member
*/
void Fsm::displaySignals(std::map< std::string, struct signal_data>& signals_state)
{
    // Iterate thru signal and internal_signal map and print the current_value if display flag is set.
    for ( auto& signal : signals_to_display )
    {
        cout << signals_state[signal].current_value;
    }
};



