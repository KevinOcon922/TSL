#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <stack>
#include <cstdlib>

int main(int argc, char* argv[]){
    //Check if the compile command has the correct number of arguments
    if(argc != 2){
        std::cout<<"Error: Invalid Compile Command. Use: ./compile <file name>.<file extension>"<<std::endl;
    }
    
    std::string extension = "";
    std::string fileName = argv[1];
    int extensionStart = fileName.find(".");
    //Check if the passed file name contains an extension
    if(extensionStart == -1){
        std::cout<<"Invalid file name"<<std::endl;
        return 1;
    }
    //Check if the file extension is of the correct type (.TSL)
    if(fileName.substr(extensionStart + 1) != "TSL"){
        std::cout<<"Invalid file type. Must be .TSL"<<std::endl;
        return 1;
    }
    std::ifstream file(fileName);

    if(!file.is_open()){
        std::cout<<"Error: Failed to open file"<<std::endl;
        return 1;
    }

    //Used for the tokenization process. Lines are read in one by one and then converted into tokens in the tokens vector
    std::string line;
    std::vector<std::string> tokens;
    //Used for error messages
    int lineCounter = 1;

    std::vector<std::string> states;
    //Manual is the default state. In this state, code is executed line by line
    //If the program is in any other state, it will instead enter TRANSITION mode and will simulate the operations of the
    //machine using only the defined state transitions
    //MANUAL is treated like any other state, so you can define a transition function that transitions to manual. However, you can not define a transition function on MANUAL itself
    states.push_back("MANUAL");

    //Used to ensure duplicate state symbol transition pairs are not defined
    std::vector<std::vector<std::string>> transitions;

    //Step 1: Read in all lines and tokenize. This step also checks that the syntax and logic make sense
    while(std::getline(file, line)){
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        if(line.size() == 0){
            lineCounter++;
            continue;
        }

        if((line[0] == 'L' || line[0] == 'l' || (line[0] == 'R' && line.size() > 1 && line[1] != 'E') || (line[0] == 'R' && line.size() == 1) || line[0] == 'r')){

            for(int i = 0; i < line.size(); i++){
                if(!(line[i] == 'L' || line[i] == 'l' || line[i] == 'R' || line[i] == 'r')){
                    std::cout<<"Error at line "<<lineCounter<<": Tape translations may only contain L, l, R, r, or blank space"<<std::endl;
                    return 1;
                }
                tokens.push_back(std::string(1, line[i]));
            }

        } else if(line[0] == 'R' && line.size() > 2 && line[2] != 'J'){
            if(line.size() < 6){
                std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean REPEAT #?"<<std::endl;
                return 1;
            }

            std::string repeatString = "";
            for(int i = 0; i < 6; i++){
                repeatString += line[i];
            }
            if(repeatString != "REPEAT"){
                std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean REPEAT #?"<<std::endl;
                return 1;
            }
            tokens.push_back("REPEAT");

            if(line.size() > 6){
                std::string numberString = "";
                for(int i = 6; i < line.size(); i++){
                    if(line[i] == '{'){
                        if(i + 1 != line.size()){
                            std::cout<<"Error at line "<<lineCounter<<": Nothing should follow a { on the same line"<<std::endl;
                            return 1;
                        }
                        try{
                            size_t pos;
                            std::stoi(numberString, &pos);
                            if(pos != numberString.size()){
                                std::cout<<"Error at line "<<lineCounter<<": Invalid integer following REPEAT"<<std::endl;
                                return -1;
                            }
                        } catch(const std::runtime_error& e){
                            std::cout<<"Error at line "<<lineCounter<<": Invalid integer following REPEAT"<<std::endl;
                            return 1;
                        }
                        tokens.push_back(numberString);
                        tokens.push_back("{");
                        break;
                    }
                    numberString += line[i];
                }
                if(line[line.size() - 1] != '{'){
                    try{
                        size_t pos;
                        std::stoi(numberString);
                        if(pos != numberString.size()){
                            std::cout<<"Error at line "<<lineCounter<<": Invalid integer following REPEAT"<<std::endl;
                        }
                    } catch(const std::runtime_error& e){
                        std::cout<<"Error at line "<<lineCounter<<": Invalid integer following REPEAT"<<std::endl;
                        return 1;
                    }
                    tokens.push_back(numberString);
                }
            } else {
                tokens.push_back("FOREVER");
            }
        } else if(line[0] == '{' || line[0] == '}'){
            if(line[0] == '{' && line.size() != 1){
                std::cout<<"Error at line "<<lineCounter<<": Nothing should follow a { on the same line"<<std::endl;
                return 1;
            }
            tokens.push_back(std::string(1, line[0]));
            if(line[0] == '}'){
                if(line.size() == 1){
                    continue;
                } else if(line.size() == 5 || line.size() == 6){
                    if(line.substr(1, 4) != "ELSE"){
                        std::cout<<"Error at line "<<lineCounter<<": The only thing that may follow a } is 'ELSE' or 'ELSE {'"<<std::endl;
                        return 1;
                    }
                    tokens.push_back("ELSE");
                    if(line.size() == 6){
                        if(line.substr(5, 1) != "{"){
                            std::cout<<"Error at line "<<lineCounter<<": The only thing that may follow '} ELSE' is '{'"<<std::endl;
                            return 1;
                        }
                        tokens.push_back("{");
                    }
                }
            }
        } else if(line[0] == 'I'){
            if(!(line.size() == 3 || line.size() == 4) || line[1] != 'F'){
                std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean IF <SYMBOL>?"<<std::endl;
                return 1;
            }
            tokens.push_back("IF");
            tokens.push_back(std::string(1, line[2]));
            if(line.size() == 4){
                if(line[3] == '{'){
                    tokens.push_back("{");
                } else {
                    std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean IF <SYMBOL>?"<<std::endl;
                    return 1;
                }
            }
        } else if(line[0] == 'A'){
            if(line.size() != 6 || line != "ACCEPT"){
                std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean ACCEPT?"<<std::endl;
                return 1;
            }
            tokens.push_back("ACCEPT");
        } else if(line[0] == 'R'){
            if(line.size() != 6 || line != "REJECT"){
                std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean REJECT?"<<std::endl;
                return 1;
            }
            tokens.push_back("REJECT");
        } else if(line[0] == 'P'){
            if(line.size() != 5 || line != "PRINT"){
                std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean PRINT?"<<std::endl;
                return 1;
            }
            tokens.push_back("PRINT");
        } else if(line[0] == 'W'){
            if(line.size() < 2){
                std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean WRITE <SYMBOL> or WHILE <SYMBOL>?"<<std::endl;
                return 1;
            }

            if(line[1] == 'R'){
                if(line.size() != 6 || line.substr(0, 5) != "WRITE"){
                    std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean WRITE <SYMBOL>?"<<std::endl;
                    return 1;
                }
                tokens.push_back("WRITE");
                tokens.push_back(std::string(1, line[5]));
            } else if(line[1] == 'H'){
                if(!(line.size() == 6 || line.size() == 7) || line.substr(0, 5) != "WHILE"){
                    std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean WHILE <SYMBOL>?"<<std::endl;
                    return 1;
                }
                tokens.push_back("WHILE");
                tokens.push_back(std::string(1, line[5]));
                if(line.size() == 7){
                    if(line[6] != '{'){
                        std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean WHILE <SYMBOL>?"<<std::endl;
                        return 1;
                    }
                    tokens.push_back("{");
                }
            }
        } else if(line[0] == 'S'){
            if(line.size() < 6 || line.substr(0, 5) != "STATE"){
                std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean STATE <STATE>?"<<std::endl;
                return 1;
            }
            tokens.push_back("STATE");
            std::string state = line.substr(5);
            if(state == "MANUAL"){
                std::cout<<"Error at line "<<lineCounter<<": Can not transition to manual state via STATE because that implies the machine is already in MANUAL state"<<std::endl;
                return 1;
            }
            tokens.push_back(state);

            bool stateExists = false;
            for(int i = 0; i < states.size(); i++){
                if(states[i] == state){
                    stateExists = true;
                    break;
                }
            }
            if(!stateExists){
                std::cout<<"Error at line "<<lineCounter<<": Attempted to transition to a state that does not exist"<<std::endl;
                return 1;
            }

            if(line.substr(5).find(":") != -1){
                std::cout<<"Error at line "<<lineCounter<<": State names may not contain a colon. Did you mean DEF STATE <NAME>: <TYPE>?"<<std::endl;
                return 1;
            }
        } else if(line[0] == 'U'){
            if(!(line.size() == 6 || line.size() == 7) || line.substr(0, 5) != "UNTIL"){
                std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean UNTIL <SYMBOL>?"<<std::endl;
                return 1;
            }
            tokens.push_back("UNTIL");
            tokens.push_back(std::string(1, line[5]));
            if(line.size() == 7){
                if(line[6] != '{'){
                    std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean UNTIL <SYMBOL>?"<<std::endl;
                    return 1;
                }
                tokens.push_back("{");
            }
        } else if(line[0] == 'D'){
            if(line.size() < 4 || line.substr(0, 3) != "DEF"){
                std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean DEF STATE/TRANSITION?"<<std::endl;
                return 1;
            }
            tokens.push_back("DEF");

            if(line[3] == 'S'){
                if(line.size() <  9 || line.substr(3, 5) != "STATE"){
                    std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean DEF STATE <NAME>: <TYPE>?"<<std::endl;
                    return 1;
                }
                tokens.push_back("STATE");

                std::string nameString = "";
                bool endFound = false;
                int colonIndex = 0;
                for(int i = 8; i < line.size(); i++){
                    if(line[i] == ':'){
                        endFound = true;
                        colonIndex = i;
                        break;
                    }
                    nameString += line[i];
                }

                if(!endFound){
                    std::cout<<"Error at line "<<lineCounter<<": Terminating character : not found after name declaration"<<std::endl;
                    return 1;
                }
                if(nameString.size() == 0){
                    std::cout<<"Error at line "<<lineCounter<<": State name can not be blank"<<std::endl;
                    return 1;
                }
                for(int i = 0; i < states.size(); i++){
                    if(states[i] == nameString){
                        std::cout<<"Error at line "<<lineCounter<<": State "<<nameString<<" already exists"<<std::endl;
                        return 1;
                    }
                }
                states.push_back(nameString);
                tokens.push_back(nameString);

                if(line.size() != colonIndex + 2){
                    std::cout<<"Error at line "<<lineCounter<<": State type must follow : Valid types include F, N, R, A"<<std::endl;
                    return 1;
                }
                char stateType = line[colonIndex + 1];

                if(stateType != 'F' && stateType != 'N' && stateType != 'R' && stateType != 'A'){
                    std::cout<<"Error at line "<<lineCounter<<": Invalid state type. Valid types include F, N, R, A"<<std::endl;
                    return 1;
                }
                tokens.push_back(std::string(1, stateType));

            } else if(line[3] == 'T'){
                if(line.size() < 15 || line.substr(3, 11) != "TRANSITION:"){
                    std::cout<<"Error at line "<<lineCounter<<": Invalid command. Did you mean DEF TRANSITION: <CURRENT STATE>, <TAPE SYMBOL>, <PLACE SYMBOL>, <MOVE>, <NEXT STATE>?"<<std::endl;
                    return 1;
                }
                tokens.push_back("TRANSITION");

                int counter = 0;
                std::string parameter = "";
                for(int i = 14; i < line.size(); i++){
                    if(line[i] == ','){
                        if(parameter == ""){
                            std::cout<<"Error at line "<<lineCounter<<": State transitions can not contain an empty parameter"<<std::endl;
                            return 1;
                        }
                        tokens.push_back(parameter);
                        parameter = "";
                        counter++;
                    } else {
                        parameter += line[i];
                    }
                }
                tokens.push_back(parameter);
                counter++;

                if(counter != 5){
                    std::cout<<"Error at line "<<lineCounter<<": Invalid number of parameters for a state transition. Must contain 5: <CURRENT STATE>, <TAPE SYMBOL>, <PLACE SYMBOL>, <MOVE>, <NEXT STATE>"<<std::endl;
                    return 1;
                }

                int parameterStartIndex = tokens.size() - 5;
                bool stateExists = false;
                for(int i = 0; i < states.size(); i++){
                    if(states[i] == tokens[parameterStartIndex]){
                        stateExists = true;
                        break;
                    }
                }
                if(!stateExists){
                    std::cout<<"Error at line "<<lineCounter<<": First parameter, current state, is not defined"<<std::endl;
                    return 1;
                }
                for(int i = 0; i < transitions.size(); i++){
                    if(transitions[i][0] == tokens[parameterStartIndex] && transitions[i][1] == tokens[parameterStartIndex + 1]){
                        std::cout<<"Error at line "<<lineCounter<<": State-Symbol pair already exists as a transition"<<std::endl;
                        return 1;
                    }
                }
                std::vector<std::string> transition;
                transition.push_back(tokens[parameterStartIndex]);
                transition.push_back(tokens[parameterStartIndex + 1]);

                std::string moveSymbol = tokens[parameterStartIndex + 3];
                if(moveSymbol != "R" && moveSymbol != "r" && moveSymbol != "L" && moveSymbol != "l" && moveSymbol != "S" && moveSymbol != "s"){
                    std::cout<<"Error at line "<<lineCounter<<": Invalid move symbol "<<moveSymbol<<" for state transition. Valid move symbols include L, l, R, r, S, and s"<<std::endl;
                    return 1;
                }

                std::string nextState = tokens[parameterStartIndex + 4];
                bool nextStateExists = false;
                for(int i = 0; i < states.size(); i++){
                    if(states[i] == nextState){
                        nextStateExists = true;
                        break;
                    }
                }
                if(!nextStateExists){
                    std::cout<<"Error at line "<<lineCounter<<": Fifth parameter, next state, is not defined"<<std::endl;
                    return 1;
                }
            } else {
                std::cout<<"Error at line "<<lineCounter<<": Invalid defintion. You may define a STATE or a TRANSITION"<<std::endl;
                return 1;
            }
        } else if(line[0] == '#'){
            //This signifies a comment
            continue;
        } else if(line[0] == 'E'){
            if(!(line.size() == 4 || line.size() == 5) || line.substr(0, 4) != "ELSE"){
                std::cout<<"Error at line "<<lineCounter<<": Invalid command, did you mean ELSE? (Note that IF ELSE statements are not supported)"<<std::endl;
                return 1;
            }
            if(tokens[tokens.size() - 1] != '}'){
                std::cout<<"Error at line "<<lineCounter<<": An ELSE statement may only follow the closing curly bracket of an IF statement"<<std::endl;
                return 1;
            }
            tokens.push_back("ELSE");
            if(line.size() == 5){
                if(line[4] != '{'){
                    std::cout<<"Error at line "<<lineCounter<<": The only symbol that can follow an ELSE is {"<<std::endl;
                    return 1;
                }
                tokens.push_back("{");
            }
        } else {
            std::cout<<"Error at line "<<lineCounter<<": Invalid command"<<std::endl;
            return 1;
        }

        lineCounter++;
    }
    file.close();

    for(int i = 0; i < tokens.size(); i++){
        std::cout<<tokens[i]<<std::endl;
    }

    std::stack<char> bracketStack;
    for(int i = 0; i < tokens.size(); i++){
        if(tokens[i] == "{"){
            bracketStack.push('{');
        } else if(tokens[i] == "}"){
            if(bracketStack.empty()){
                std::cout<<"Error: Closing bracket without an opening bracket"<<std::endl;
                return 1;
            }
            bracketStack.pop();
        } else if(tokens[i] == "REPEAT"){
            if(i + 1 == tokens.size()){
                std::cout<<"Error: REPEAT or IF at end of file without a block statement"<<std::endl;
                return 1;
            }
            if(tokens[i + 1] != "{"){
                if((i + 2 == tokens.size()) || tokens[i + 2] != "{"){
                    std::cout<<"Error: REPEAT not followed my a block statement {}"<<std::endl;
                    return 1;
                }
            }
        } else if(tokens[i] == "WHILE"){
            if(i + 1 == tokens.size()){
                std::cout<<"Error: WHILE at end of file without a block statement"<<std::endl;
                return 1;
            }
            if((i + 2 == tokens.size()) || tokens[i + 2] != "{"){
                std::cout<<"Error: WHILE not followed my a block statement {}"<<std::endl;
                return 1;
            }
        } else if(tokens[i] == "UNTIL"){
            if(i + 1 == tokens.size()){
                std::cout<<"Error: UNTIL at end of file without a block statement"<<std::endl;
                return 1;
            }
            if((i + 2 == tokens.size()) || tokens[i + 2] != "{"){
                std::cout<<"Error: WHILE not followed my a block statement {}"<<std::endl;
                return 1;
            }
        } else if(tokens[i] == "ELSE"){
            std::stack<std::string> elseStack;
            elseStack.push_back("}");

            for(int i = )
        }
    }
    if(!bracketStack.empty()){
        std::cout<<bracketStack.size()<<std::endl;
        std::cout<<"Error: Every opening bracket does not have a corresponding closing bracket"<<std::endl;
        return 1;
    }

    //Step 2: Transpile the tokens into a C++ program
    std::ofstream outputFile(fileName.substr(0, fileName.find(".")) + ".cpp");

    outputFile<<"#include <vector>\n#include <string>\n#include <iostream>\n#include <unordered_map>\n";
    outputFile<<"std::string currentState = \"MANUAL\";\nstd::vector<char> tape(10000, '#');\nint head = 5000;\nstd::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> transitions;\nstd::unordered_map<std::string, std::string> states;\nint farthestLeft = 5000;\nint farthestRight = 5000;\nstd::string input = \"\";\nchar status;\n";
    outputFile<<"void Right();\nvoid Left();\nvoid Write(char symbol);\nvoid Accept();\nvoid Reject();\nvoid Print();\n";
    outputFile<<"void AddTransition(std::string currentState, std::string tapeSymbol, std::string placeSymbol, std::string moveSymbol, std::string nextState);\n";
    outputFile<<"void AddState(std::string stateName, std::string stateType);\nstd::vector<std::string> GetTransition(std::string state, std::string tapeSymbol);\nchar GetStateChar(std::string state);\nchar ExecuteTransitionMode();\n";
    outputFile<<"int main(int argc, char* argv[]){\n";
    outputFile<<"if(argc == 2){\ninput = argv[1];\nfor(int i = 0; i < input.size(); i++){\n";
    outputFile<<"tape[head + i] = input[i];\n}\n}\n";

    for(int i = 0; i < tokens.size(); i++){
        if(tokens[i] == "L" || tokens[i] == "l"){
            outputFile<<"Left();\n";
        } else if(tokens[i] == "R" || tokens[i] == "r"){
            outputFile<<"Right();\n";
        } else if(tokens[i] == "WRITE"){
            outputFile<<"Write('"<<tokens[i + 1]<<"');\n";
            i++;
        } else if(tokens[i] == "REPEAT"){
            if(tokens[i + 1] == "FOREVER"){
                outputFile<<"while(true)\n";
            } else{
                outputFile<<"for(int i = 0; i < "<<tokens[i + 1]<<"; i++)\n";
            }
            i++;
        } else if(tokens[i] == "{" || tokens[i] == "}"){
            outputFile<<tokens[i]<<"\n";
        } else if(tokens[i] == "IF"){
            outputFile<<"if(tape[head] == '"<<tokens[i + 1]<<"')\n";
            i++;
        } else if(tokens[i] == "ACCEPT"){
            outputFile<<"Accept();\nreturn 0;\n";
        } else if(tokens[i] == "REJECT"){
            outputFile<<"Reject();\nreturn 0;\n";
        } else if(tokens[i] == "PRINT"){
            outputFile<<"Print();\nreturn 0;\n";
        } else if(tokens[i] == "WHILE"){
            outputFile<<"while(tape[head] == '"<<tokens[i + 1]<<"')\n";
            i++;
        } else if(tokens[i] == "UNTIL"){
            outputFile<<"while(tape[head] != '"<<tokens[i + 1]<<"')\n";
            i++;
        } else if(tokens[i] == "DEF"){
            if(tokens[i + 1] == "STATE"){
                outputFile<<"AddState(\""<<tokens[i + 2]<<"\", \""<<tokens[i + 3]<<"\");\n";
                i += 3;
            } else if(tokens[i + 1] == "TRANSITION"){
                outputFile<<"AddTransition(\""<<tokens[i + 2]<<"\", \""<<tokens[i + 3]<<"\", \""<<tokens[i + 4]<<"\", \""<<tokens[i + 5]<<"\", \""<<tokens[i + 6]<<"\");\n";
                i += 6;
            }
        } else if(tokens[i] == "STATE"){
            outputFile<<"currentState = \""<<tokens[i + 1]<<"\";\n";
            outputFile<<"status = ExecuteTransitionMode();\n";
            outputFile<<"if(status != 'M'){\nreturn 0;\n}\n";

            i++;
        } else if(tokens[i] == "ELSE"){
            outputFile<<"else\n";
        }
    }

    outputFile<<"return 0;\n}\n";
    outputFile<<"void Right(){\nif(head + 1 == transitions.size()){\ntape.push_back('#');\n}\nhead++;\nif(head > farthestRight){\nfarthestRight = head;\n}\n}\n";
    outputFile<<"void Left(){\nif(head == 0){\ntape.insert(tape.begin(), '#');\n} else {\nhead--;\nfarthestRight++;\n}\nif(head < farthestLeft){\nfarthestLeft = head;\n}\n}\n";
    outputFile<<"void Write(char symbol){\ntape[head] = symbol;\n}\n";
    outputFile<<"void Accept(){\nstd::cout<<\"Input \"<<input<<\" accepted\"<<std::endl;\n}\n";
    outputFile<<"void Reject(){\nstd::cout<<\"Input \"<<input<<\" rejected\"<<std::endl;\n}\n";
    outputFile<<"void Print(){\nstd::cout<<\"Tape output:\"<<std::endl<<\"#\";\nfor(int i = farthestLeft; i <= farthestRight; i++){\nstd::cout<<tape[i];\n}\nstd::cout<<\"#\"<<std::endl;\n}\n";
    outputFile<<"void AddTransition(std::string currentState, std::string tapeSymbol, std::string placeSymbol, std::string moveSymbol, std::string nextState){\nstd::vector<std::string> newTransition;\nnewTransition.push_back(currentState);\n";
    outputFile<<"newTransition.push_back(tapeSymbol);\nnewTransition.push_back(placeSymbol);\nnewTransition.push_back(moveSymbol);\nnewTransition.push_back(nextState);\ntransitions[currentState][tapeSymbol] = newTransition;\n}\n";
    outputFile<<"void AddState(std::string stateName, std::string stateType){\nstates[stateName] = stateType;\n}\n";
    outputFile<<"std::vector<std::string> GetTransition(std::string state, std::string tapeSymbol){\nreturn transitions[state][tapeSymbol];\n}\n";
    outputFile<<"char GetStateChar(std::string state){\nreturn states[state][0];\n}\n";
    outputFile<<"char ExecuteTransitionMode(){\nchar statusChar;\nwhile(true){\nstd::vector<std::string> transition = GetTransition(currentState, std::string(1, tape[head]));\n";
    outputFile<<"if(transition.size() == 0){\nif(GetStateChar(currentState) == 'F' || GetStateChar(currentState) == 'A'){\nAccept();\nstatusChar = 'A';\nbreak;\n}\nelse{\nReject();\nstatusChar = 'R';\nbreak;\n}\n}\ncurrentState = transition[4];\nWrite(transition[2][0]);\nif(transition[3] == \"R\"){\nRight();\n} else if(transition[3] == \"L\"){\nLeft();\n}\n";
    outputFile<<"if(currentState == \"MANUAL\"){\nstatusChar = 'M';\nbreak;\n}\nchar stateChar = GetStateChar(currentState);\n";
    outputFile<<"if(stateChar == 'A'){\nAccept();\nstatusChar = 'A';\nbreak;\n} else if(stateChar == 'R'){\nReject();\nstatusChar = 'R';\nbreak;\n}\n";
    outputFile<<"if(tape[head] == '@'){\nif(stateChar == 'A'){\nAccept();\nstatusChar = 'A';\nbreak;\n}\n}\n}\nreturn statusChar;\n}\n";

    outputFile.close();

    std::system(("g++ " + fileName.substr(0, fileName.find(".")) + ".cpp -o " + fileName.substr(0, fileName.find("."))).c_str());
    std::system(("rm -rf " + fileName.substr(0, fileName.find(".")) + ".cpp").c_str());

    return 0;
}