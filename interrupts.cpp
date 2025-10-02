/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts.hpp>
#include <unordered_map>//doing this here because i dont think you would load my copy of interrupt.hpp

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file NOT USING THIS
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/
    const int MODE_SWITCH = 1;
    const int SAVE = 10;
    const int RESTORE = 10;
    const int FIND_VECTOR = 10;
    const int GET_ISR = 1;
    const int IRET = 1;


    const int VECTOR_ENTRY_BYTES = 2;

    struct event {
        enum Type {CPU,END_IO, SYSCALL};
        Type type;
        int dur;
    };

    struct logLine {
        long long start;
        int dur;
        std::string text;
    };

    long long t = 0;
    
    std::vector<logLine> out;
    std::vector<event> tracefile;
    std::unordered_map<int, long long> call_map;
    //helper functions

    //BRO I CANT MAKE MY OWN FUNCTIONS WHAT???
    // my ta whoever looks at this, you guys did this to yourself. this while loop will be deadly.
    // nvm thank cpp for lambdas, just learnt them.

    auto trace_convert = [](std::string type, int dur){
        event::Type t;

        if (type == "CPU")       t = event::CPU;
        else if (type == "END_IO") t = event::END_IO;
        else if (type == "SYSCALL") t = event::SYSCALL;
        else throw std::invalid_argument("Unknown type");

        return event{t, dur};
    };


    auto logger = [&t, &out](int dur, std::string activity){
        out.push_back({t, dur, activity});
        t += dur;
    };

    auto time_predict = [&t, &call_map, &delays](int d){
        long long when = t + delays.at(d);
        call_map[d] = when;
    };

    auto reconciler = [&call_map, &t](int d){
        if (call_map[d] == t) return;
        else call_map[d] = t;
    };


    // i could make this more resilient but i think thats outside the scope of the assignment




    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
        //std::cout << "Raw line: " << trace << std::endl;
        //std::cout << "Parsed: " << activity << " " << duration_intr << std::endl;
        event current = trace_convert(activity, duration_intr);
        //std::cout << current.type << " " << current.dur << "\n" << "------------------------";
        logger(duration_intr,activity);
        if (current.type == 0){
            execution += std::to_string(t) + ", " + std::to_string(duration_intr)+ ", CPU burst \n";
        }
        else{
            std::pair<std::string, int> boiler_plate = intr_boilerplate(t,duration_intr,SAVE,vectors);
            execution += boiler_plate.first;
            t = boiler_plate.second; 

            //ISR BODY STUFF
            execution += std::to_string(t) + "," + std::to_string(duration_intr) + ",ISR BODY\n";
            t += duration_intr;
            execution += std::to_string(t) + ",1, IRET \n";
        }    
        /************************************************************************/

    }
    /*
    for (auto &line : out) {
        std::cout << line.start << " "
                << line.dur   << " "
                << line.text  << std::endl
                << "-------------" << "\n";
    }
    */
    input_file.close();

    write_output(execution);

    return 0;
}