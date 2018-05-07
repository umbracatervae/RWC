//
// Created by Akshay on 3/24/2018.
//

#ifndef RW_COMMON_H
#define RW_COMMON_H

#include <vector>
#include <fstream>
#include <iostream>
#include <chrono>
#include "string.h"

#define CLUSTER_SIZE_LIMIT 10
#define GUI_NODE_CLUSTERSIZE_LIMIT 20

std::string INPUT_LATCH_PREFIX = "[IL]";
std::string OUTPUT_LATCH_PREFIX = "[OL]";
long long int MAX_PREFIX_LENGTH = (INPUT_LATCH_PREFIX.length() >= OUTPUT_LATCH_PREFIX.length()) ? INPUT_LATCH_PREFIX.length() : OUTPUT_LATCH_PREFIX.length();


Node* retrieveNodeByStr(std::string nodeID, std::vector<Node> &nodeList){
    //DESCRIPTION: Helper function to retrieve a node's pointer
    for (std::vector<Node>::iterator iN = nodeList.begin(); iN < nodeList.end(); ++iN){
        //std::cout << "CHECKING FOR: " << nodeID << "; FOUND " << iN->strID << std::endl;
        if (iN->strID == nodeID){
            //Match found!
            return &(*iN);
        }
    }
    return nullptr;
}
Node* retrieveNodeByStr_ptr(std::string nodeID, std::vector<Node*> &nodeList){
    //DESCRIPTION: Helper function to retrieve a node's pointer
    for (std::vector<Node*>::iterator iN = nodeList.begin(); iN < nodeList.end(); ++iN){
        if ((*iN)->strID == nodeID){
            //Match found!
            return *iN;
        }
    }
    return nullptr;
}

std::string ripBadChars(std::string str){
    std::string result = "";
    for (int i=0; i < str.length(); ++i){
        if (str[i] == '\0' || str[i] == '\r' || str[i] == ' ' || str[i] == '\t' || str[i] == '\n'){
            continue;
        }
        result.push_back(str[i]);
    }
    return result;
}

std::vector<std::string> strSplitter(std::string line){
    std::size_t pos = 0, posSpace =0, posTab = 0;
    std::vector<std::string> result;
    posSpace = line.find_first_of(" ");
    posTab = line.find_first_of("\t");
    if (posSpace != std::string::npos && posTab != std::string::npos){
        pos = (posSpace < posTab ) ? posSpace : posTab;
    }
    else {
        if (posSpace == std::string::npos && posTab != std::string::npos){
            pos = posTab;
        }
        else if (posSpace != std::string::npos && posTab == std::string::npos) {
            pos = posSpace;
        }
        else {
            //both are npos
            result.push_back(line);
            return result;
        }
    }

    while (pos != std::string::npos){
        std::string subStr = line.substr(0,pos);
        //std::cout << subStr << std::endl;
        std::string resStr = ripBadChars(subStr);
        if (subStr != "") {
            result.push_back(resStr);
        }
        line.erase(0,subStr.length() + 1); //" " and "\t" are one char

        posSpace = line.find_first_of(" ");
        posTab = line.find_first_of("\t");
        if (posSpace != std::string::npos && posTab != std::string::npos){
            pos = (posSpace < posTab ) ? posSpace : posTab;
        }
        else {
            if (posSpace == std::string::npos && posTab != std::string::npos){
                pos = posTab;
            }
            else if (posSpace != std::string::npos && posTab == std::string::npos) {
                pos = posSpace;
            }
            else {
                //both are npos
                pos = std::string::npos;
            }
        }

    }
    std::string resStr = ripBadChars(line);
    if (resStr != ""){
        result.push_back(resStr);
    }
    return result;
}

void parseBLIF(std::string filename, int& piDelay, int& poDelay, int& nodeDelay, std::vector<Node>& rawNodeList){
    //preliminary run
    //std::cout << "Filename: " << filename << std::endl;
    std::ifstream blifFile;
    blifFile.open(filename);
    std::string line;

    std::string latchStr = ".latch";
    std::string gateStr = ".names";
    std::string inputStr = ".inputs";
    std::string outputStr = ".outputs";
    std::string prevStr = "";

    if (blifFile.is_open()){
        std::string modeStr = "";
        while(std::getline(blifFile,line)) {
            //std::cout << line << std::endl;
            std::vector<std::string> signals = strSplitter(line);

            if (signals.at(0) == inputStr || modeStr == inputStr){
                std::vector<std::string>::iterator iS = (modeStr == "") ? signals.begin()+1 : signals.begin();
                modeStr = "";
                for (;iS < signals.end();++iS){
                    if ((*iS).c_str()[0] == '\\') {
                        modeStr = inputStr;
                        continue;
                    }
                    Node n(piDelay);
                    n.isPI = true;
                    n.isPO = false;
                    n.strID = *iS;
                    rawNodeList.push_back(n);
                    //std::cout << "PI NODE ADDED: " << n.strID << std::endl;
                }
                continue;
            }
            if (signals.at(0) == outputStr || modeStr == outputStr){
                std::vector<std::string>::iterator iS = (modeStr == "") ? signals.begin()+1 : signals.begin();
                modeStr = "";
                for (;iS < signals.end();++iS){
                    if ((*iS).c_str()[0] == '\\') {
                        modeStr = outputStr;
                        continue;
                    }
                    Node n(poDelay);
                    n.isPI = false;
                    n.isPO = true;
                    n.strID = *iS;
                    rawNodeList.push_back(n);
                    //std::cout << "PO NODE ADDED: " << n.strID << std::endl;
                }
                continue;
            }
            if (signals.at(0) == latchStr){
                modeStr = "";
                int argCount = 0;
                while (argCount < 2){
                    if (!argCount) {
                        //Input of Latch becomes PO
                        Node n(poDelay);
                        n.strID = signals.at(1) + OUTPUT_LATCH_PREFIX;
                        n.isPO = true;
                        n.isPI = false;
                        rawNodeList.push_back(n);
                    }
                    else {
                        //Output of Latch becomes PI
                        Node n(piDelay);
                        n.strID = signals.at(2) + INPUT_LATCH_PREFIX;
                        n.isPI = true;
                        n.isPO = false;
                        rawNodeList.push_back(n);
                    }
                    argCount += 1;
                }
                continue;
            }
            if (signals.at(0) == gateStr){
                modeStr = "";
                Node *gateNode = retrieveNodeByStr(signals.at(signals.size() - 1), rawNodeList);
                if (gateNode != nullptr){
                    //this node has already been initiliazed as a primary output
                    gateNode->procStr = line;
                }
                else {
                    //this node has not already been initialized
                    Node n(nodeDelay);
                    n.strID = signals.at(signals.size()-1);
                    n.procStr = line;
                    n.isPI = false;
                    n.isPO = false;
                    rawNodeList.push_back(n);
                }
            }
        }

    }  //ENDIF BLIF OPEN

    /*
    for (auto node : rawNodeList){
        std::cout << "NODE: " << node.strID << std::endl;
    }
    */

    //fix the rawNodeList structure
    //std::cout << "SECONDARY PARSE RUN" << std::endl;

    for (std::vector<Node>::iterator iN = rawNodeList.begin(); iN < rawNodeList.end(); ++iN){
        iN->addr = &(*iN);
        if (iN->procStr != ""){
            //std::cout << iN->strID << std::endl;
            std::vector<std::string> nStrList = strSplitter(iN->procStr);
            for (std::vector<std::string>::iterator is = nStrList.begin()+1; is < nStrList.end()-1; ++is){
                Node *driver = retrieveNodeByStr(*is, rawNodeList);
                if (driver != nullptr){
                    iN->prev.push_back(driver);
                    driver->next.push_back(&(*iN));
                }
                else {
                    driver = retrieveNodeByStr(*is + INPUT_LATCH_PREFIX,rawNodeList);
                    if (driver != nullptr){
                        iN->prev.push_back(driver);
                        driver->next.push_back(&(*iN));
                    }
                    else {
                        std::cout << "Error: Gate Driver Not Found: " << *is << std::endl;
                        exit(-1);
                    }
                }

            }
        }
        else if (iN->strID.length() > MAX_PREFIX_LENGTH){
            if (iN->strID.substr(iN->strID.length()-OUTPUT_LATCH_PREFIX.length(),OUTPUT_LATCH_PREFIX.length()) == OUTPUT_LATCH_PREFIX) {
                //the node is a PO latch which we need to setup correctly
                Node *driver = retrieveNodeByStr(iN->strID.substr(0,iN->strID.length()-4),rawNodeList);
                if (driver != nullptr){
                    driver->next.push_back(&(*iN));
                    iN->prev.push_back(driver);
                }
                else {
                    driver = retrieveNodeByStr(iN->strID.substr(0,iN->strID.length()-INPUT_LATCH_PREFIX.length()) + INPUT_LATCH_PREFIX,rawNodeList);
                    if (driver != nullptr){
                        driver->next.push_back(&(*iN));
                        iN->prev.push_back(driver);
                    }
                    else {
                        //ERROR
                        std::cout << "Error: Latch Driver Not Found" << std::endl;
                        exit(-2);
                    }
                }
            }
        }
    }

}

std::vector<Node*> obtainPONodes(std::vector<Node>& rawNodeList){
    std::vector<Node*> result;
    for (auto in = rawNodeList.begin(); in < rawNodeList.end(); ++in){
        if (in->isPO){
            result.push_back(&(*in));
        }
    }
    return result;
}

std::vector<Node*> obtainPINodes(std::vector<Node>& rawNodeList){
    std::vector<Node*> result;
    for (auto in = rawNodeList.begin(); in < rawNodeList.end(); ++in){
        if (in->isPI){
            result.push_back(&(*in));
        }
    }
    return result;
}

void generateInputSet(Cluster& c){

    //Description: generates the input() set for a cluster
    std::copy(c.members.begin(),c.members.end(),std::back_inserter(c.inputSet));

    for(auto cNode : c.members){
        for (auto pNode : cNode->prev){
            //check if node isn't already part of input set
            Node *nPtr = retrieveNodeByStr_ptr(pNode->strID,c.inputSet);
            if (nPtr == nullptr){
                c.inputSet.push_back(pNode);
            }
        }
    }

    //delete the cluster elements from the inputSet
    c.inputSet.erase(c.inputSet.begin(), c.inputSet.begin() + c.members.size());

}

std::pair<long long int,std::string> measureExecTime(std::chrono::time_point<std::chrono::high_resolution_clock>& start,std::chrono::time_point<std::chrono::high_resolution_clock>& end){
    std::pair<long long int,std::string> result;
    result.first = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    result.second = "us";
    if (result.first >= 1000){
        result.first = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        result.second = "ms";
        if (result.first >= 1000){
            result.first = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            result.second = "s";
            if (result.first >= 60){
                result.first = std::chrono::duration_cast<std::chrono::minutes>(end - start).count();
                result.second = "min";
                if (result.first >= 60){
                    result.first = std::chrono::duration_cast<std::chrono::hours>(end - start).count();
                    result.second = "hrs";
                }
            }
        }
    }
    return result;
}

void writeOutputFiles(std::string circuitName,
                      std::vector<Node*>& topoNodeList,
                      std::vector<Cluster>& clList,
                      std::vector<Cluster*>& clListFinal,
                      int& cmdMaxClusterSize,
                      int& cmdInterClusterDelay,
                      int& cmdPiDelay,
                      int& cmdPoDelay,
                      int& cmdNodeDelay,
                      int& cmdUseLawlerLabeling,
                      int& cmdUseGui,
                      int& cmdUseExp)
{
    //Description: function for writing to the output files for the application
    bool tooLargeForTable = cmdMaxClusterSize > CLUSTER_SIZE_LIMIT;
    std::ofstream resultTable;
    std::ofstream verboseResult;
    std::ofstream clustrTable;


    resultTable.open("output_" + circuitName + "_table.csv");
    verboseResult.open("output_" + circuitName + "_verbose.txt");
    clustrTable.open("output_" + circuitName + "_cluster.csv");

    if (cmdUseLawlerLabeling) { resultTable << "NODE,PI?,PO?,NODE DELAY,NODE LABEL" << std::endl; }
    else if (tooLargeForTable){ resultTable << "NODE,PI?,PO?,NODE DELAY,NODE LABEL,CLUSTER SIZE" << std::endl; }
    else { resultTable << "NODE,PI?,PO?,NODE DELAY,NODE LABEL,CLUSTER SIZE,CLUSTER CONTENTS" << std::endl; }

    verboseResult << "Rajaraman-Wong/Lawler Clustering Application\nAkshay Nagendra <akshaynag@gatech.edu>, Paul Yates <paul.maxyat@gatech.edu>" << std::endl;
    verboseResult << "\n----------COMMAND LINE PARAMETERS----------\n" << std::endl;
    verboseResult << "Input Netlist: " << circuitName << ".blif" << std::endl;
    verboseResult << "Max Cluster Size: " << cmdMaxClusterSize << std::endl;
    verboseResult << "Inter Cluster Delay: " << cmdInterClusterDelay << std::endl;
    verboseResult << "Primary Input Delay: " << cmdPiDelay << std::endl;
    verboseResult << "Primary Output Delay: " << cmdPoDelay << std::endl;
    verboseResult << "Node Delay: " << cmdNodeDelay << std::endl;
    if (!cmdUseLawlerLabeling){
        verboseResult << "RUN MODE: RW CLUSTERING\n" << std::endl;
    }
    else {
        verboseResult << "RUN MODE: LAWLER\n" << std::endl;
    }
    if (cmdUseGui){
        verboseResult << "GUI MODE: ENABLED" << std::endl;
    }
    else {
        verboseResult << "GUI MODE: DISABLED" << std::endl;
    }
    if (cmdUseExp){
        verboseResult << "NON-OVERLAP MODE: ENABLED" << std::endl;
    }
    else {
        verboseResult << "NON-OVERLAP MODE: DISABLED" << std::endl;
    }
    verboseResult << "----------NODE INFORMATION----------\n" << std::endl;

    for (int i=0; i < topoNodeList.size(); ++i) {
        resultTable << topoNodeList.at(i)->strID << ",";
        verboseResult << "NODE " << topoNodeList.at(i)->strID << ":" << std::endl;
        std::string pi = (topoNodeList.at(i)->isPI) ? "Y" : "N";
        std::string po = (topoNodeList.at(i)->isPO) ? "Y" : "N";
        resultTable << pi << "," << po << ",";
        verboseResult << "\tPI?: " << pi << "\n\tPO?: " << po << std::endl;
        resultTable << topoNodeList.at(i)->delay << ",";
        verboseResult << "\tDELAY: " << topoNodeList.at(i)->delay << std::endl;
        resultTable << topoNodeList.at(i)->label << ",";
        verboseResult << "\tLABEL: " << topoNodeList.at(i)->label << std::endl;
        if (!cmdUseLawlerLabeling) {
            resultTable << clList.at(i).members.size();
            if (!tooLargeForTable) {
                resultTable << ",";
            }
            verboseResult << "\tCLUSTER SIZE: " << clList.at(i).members.size() << std::endl;

            verboseResult << "\tCLUSTER MEMBERS: ";
            int count = 0;
            for (auto clMem : clList.at(i).members) {
                if (!tooLargeForTable) {
                    resultTable << clMem->strID << " ";
                }
                if (count == clList.at(i).members.size() - 1) {
                    verboseResult << clMem->strID;
                } else {
                    verboseResult << clMem->strID << ", ";
                }
                count += 1;
            }
        }
        verboseResult << std::endl;
        resultTable << std::endl;
    }
    resultTable.close();

    verboseResult << "\n----------FORMED CLUSTER INFORMATION----------\n" << std::endl;

    if (tooLargeForTable) { clustrTable << "CLUSTER ROOT NODE,CLUSTER SIZE" << std::endl; }
    else { clustrTable << "CLUSTER ROOT NODE,CLUSTER SIZE,CLUSTER CONTENTS" << std::endl; }
    for (auto cl : clListFinal){
        clustrTable << topoNodeList.at(cl->id)->strID << "," << cl->members.size();
        if (!tooLargeForTable){
            clustrTable << ",";
        }
        verboseResult << "CLUSTER ROOT NODE: " << topoNodeList.at(cl->id)->strID << std::endl;
        verboseResult << "\tCLUSTER SIZE: " << cl->members.size() << std::endl;
        verboseResult << "\tCLUSTER MEMBERS: ";

        int count = 0;
        for (auto clMem : cl->members) {
            if (!tooLargeForTable) {
                clustrTable << clMem->strID << " ";
            }
            if (count == cl->members.size()-1){
                verboseResult << clMem->strID;
            }
            else {
                verboseResult << clMem->strID << ", ";
            }
            count += 1;
        }
        verboseResult << std::endl;
        clustrTable << std::endl;
    }
    clustrTable.close();
}

void writeGUIFile(std::vector<Node*>& mNList,
                  std::vector<Cluster>& cList,
                  std::vector<Cluster*>& fClist,
                  std::vector<std::vector<Node*>>& lsetHistory,
                  int& maxDelay,bool& unixRun){
    std::ofstream guiFile;
    if (!unixRun){
        guiFile.open("../Python/input_graph.dmp");
    }
    else {
        guiFile.open("Python/input_graph.dmp");
    }
    guiFile << "//NODES" << std::endl;
    for (int i=0; i < mNList.size(); ++i){
        auto currentNode = mNList.at(i);
        guiFile << currentNode->id + 1 << ":" << currentNode->strID << ";" << currentNode->delay << ";";
        if (!currentNode->prev.empty()) {
            for (auto iP = currentNode->prev.begin(); iP < currentNode->prev.end()-1; ++iP) {
                guiFile << (*iP)->id + 1 << " ";
            }
            guiFile << (*(currentNode->prev.end()-1))->id + 1;
        }
        guiFile << ";";
        if (!currentNode->next.empty()) {
            for (auto iNx = currentNode->next.begin(); iNx < currentNode->next.end()-1; ++iNx) {
                guiFile << (*iNx)->id + 1 << " ";
            }
            guiFile << (*(currentNode->next.end()-1))->id + 1;
        }
        guiFile << ";";
        guiFile << currentNode->label << ";";
        if (!cList.at(currentNode->id).members.empty()){
            for (auto iC = cList.at(currentNode->id).members.begin(); iC < cList.at(currentNode->id).members.end()-1; ++iC){
                guiFile << (*iC)->id + 1 << " ";
            }
            guiFile << (*(cList.at(currentNode->id).members.end()-1))->id + 1;
        }
        guiFile << std::endl;
    }
    guiFile << "//CLUSTERS" << std::endl;
    if (!lsetHistory.empty()) {
        guiFile << "LSET:";
        if (!lsetHistory.begin()->empty()) {
            for (auto n = lsetHistory.begin()->begin(); n < lsetHistory.begin()->end()-1; ++n) {
                guiFile << (*n)->id + 1 << " ";
            }
            guiFile << (*(lsetHistory.begin()->end()-1))->id + 1;
        }
        guiFile << std::endl;
        for (int i=0; i < fClist.size(); ++i){
            auto lSet = lsetHistory.at(i+1);
            auto currentCluster = fClist.at(i);
            auto inputSet = fClist.at(i)->inputSet;
            guiFile << currentCluster->id + 1 << ":";
            if (!currentCluster->members.empty()){
                for (auto mem = currentCluster->members.begin(); mem < currentCluster->members.end()-1;++mem){
                    guiFile << (*mem)->id + 1 << " ";
                }
                guiFile << (*(currentCluster->members.end()-1))->id + 1;
            }
            guiFile << ";LSET:";
            if (!lSet.empty()) {
                for (auto lNode = lSet.begin(); lNode < lSet.end() - 1; ++lNode) {
                    guiFile << (*lNode)->id + 1 << " ";
                }
                guiFile << (*(lSet.end() - 1))->id + 1;
            }
            guiFile << ";ISET:";
            if (!inputSet.empty()){
                for (auto mem = inputSet.begin(); mem < inputSet.end()-1; ++mem){
                    guiFile << (*mem)->id + 1 << " ";
                }
                guiFile << (*(inputSet.end()-1))->id + 1;
            }
            guiFile << std::endl;
        }

    }
    guiFile << "//MAXDELAY" << std::endl;
    guiFile << maxDelay;
    guiFile.close();
}
int memParseLine(char *line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

void reportMemUsage(std::string circuitName){ //only works on linux

        FILE* file = fopen("/proc/self/status", "r");
        std::ofstream verboseFile;
        verboseFile.open("output_" + circuitName + "_verbose.txt",std::fstream::app);
        int result1 = -1;
        int result2 = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL){
            if (strncmp(line, "VmSize:", 7) == 0){
                result1 = memParseLine(line);
            }
            if (strncmp(line, "VmRSS:", 6) == 0){
                result2 = memParseLine(line);
                break;
            }
        }
        fclose(file);
        std::cout << "-----------MEMORY USAGE------------" << std::endl;
        std::cout << "VIRTUAL MEMORY USED:\t" << result1/1024 << "MB" << std::endl;
        std::cout << "PHYSICAL MEMORY USED:\t" << result2/1024 << "MB" << std::endl;
        verboseFile << "\n-----------MEMORY USAGE------------\n" << std::endl;
        verboseFile << "VIRTUAL MEMORY USED:\t" << result1/1024 << "MB" << std::endl;
        verboseFile << "PHYSICAL MEMORY USED:\t" << result2/1024 << "MB" << std::endl;
        verboseFile.close();
}

#endif //RW_COMMON_H

