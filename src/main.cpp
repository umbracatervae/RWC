/*
PROJECT NAME: RWClustering Application
AUTHORS:      Akshay Nagendra <akshaynag@gatech.edu>
              Paul Yates      <paul.maxyat@gatech.edu>

DESCRIPTION: C++ STL Implementation of the Rajaraman-Wong clustering algorithm designed to simplify
             the complexity of input circuit netlist while minimizing the critical IO path delay
FEATURES:   -Supports pure Rajaraman-Wong clustering with various runtime options such as sparse matrices and on-the fly delay calculations
            -Utilizes a Python GUI (RWGUI) that highlights various stages of the clustering algorithm if the circuit is small enough
            -Allows users to also run the Lawler labelling and clustering algorithm to compare the two clustering algorithms
            -Supports experimental method to minimized area increase due to overlap in gate clusters (--exp)
*/



#include <set>
#include "Node.h"
#include "Cluster.h"
#include "common.h"
#include <algorithm>
#include <getopt.h>
#include "SparseMatrix.h"

namespace sc = std::chrono;

int MAX_CLUSTER_SIZE = 8; //default value = 8
int INTER_CLUSTER_DELAY = 3;
int PRIMARY_INPUT_DELAY = 0;
int PRIMARY_OUTPUT_DELAY = 1;
int NODE_DELAY = 1;
int USE_DELAY_MATRIX = true;
int USE_SPARSE = true;
std::string FILENAME = "example_lecture.blif";
int USE_LAWLER_LABELING = false;
#if (defined(LINUX) || defined(__linux__))
    bool UNIX_RUN = true;
#else
    bool UNIX_RUN = false;
#endif
int USE_GUI = false;
int USE_EXP = false;
int USE_EXP2 = false; //RECOMMENDED AGAINST USING

std::string BLIFFile;

void addPredecessors(std::vector<Node *>&, Node*);
int max_delay(Node*, Node*, std::vector<Node*>);
void lawler_cluster(std::vector<Node *>&, Node*, std::vector<Cluster>&);
int main(int argc, char **argv) {

    //parse arguments
    int HELP_FLAG = 0;
    const struct option longopts[] =
    {
        {"lawler", no_argument,     &USE_LAWLER_LABELING, 1},
        {"no_matrix", no_argument, &USE_DELAY_MATRIX, 0},
        {"no_sparse", no_argument, &USE_SPARSE, 0},
        {"help", no_argument, nullptr, 'h'},
        {"max_cluster_size", required_argument, nullptr, 's'},
        {"pi_delay", required_argument, nullptr, 'i'},
        {"po_delay", required_argument, nullptr, 'o'},
        {"node_delay", required_argument, nullptr, 'n'},
        {"intercluster_delay", required_argument, nullptr, 'c'},
        {"gui",no_argument,&USE_GUI,1},
        {"exp",no_argument,&USE_EXP,1},
        {0,0,0,0}
    };
    int flag;
    int option_index;
    while(true){
        flag = getopt_long(argc, argv, "s:i:o:n:c:", longopts, &option_index);
        if(flag == -1) break;
        switch(flag){
            case 0:
                break;
            case 's':
                MAX_CLUSTER_SIZE = std::atoi(optarg);
                break;
            case 'i':
                PRIMARY_INPUT_DELAY = std::atoi(optarg);
                break;
            case 'o':
                PRIMARY_OUTPUT_DELAY = std::atoi(optarg);
                break;
            case 'n':
                NODE_DELAY = std::atoi(optarg);
                break;
            case 'c':
                INTER_CLUSTER_DELAY = std::atoi(optarg);
                break;
            case 'h':
                HELP_FLAG = 1;
            case '?':
                HELP_FLAG = 1;
                break;
            default:
                std::cout << "Encountered error with command line arguments" << std::endl;
                abort();
        }
    }
    if (optind < argc) {
        FILENAME = argv[optind];
    }
    if (HELP_FLAG) {
        std::cout << "\nUsage: rw [arguments] [inputFile.blif]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "-h, --help\t\tShow this help message and exit" << std::endl;
        std::cout << "--lawler\t\tUse Lawler labeling algorithm instead of RW" << std::endl;
        std::cout << "--no_matrix\t\tAvoid using a delay matrix, (pays a large runtime penalty at a large memory benefit)" << std::endl;
        std::cout << "--no_sparse\t\tAvoid using a sparse matrix, (pays a large memory penalty at a small runtime benefit)" << std::endl;
        std::cout << "--gui\t\tEnable interactive GUI (pays a runtime penalty for GUI file creation)" << std::endl;
        std::cout << "--exp\t\tEnable non-overlap for clusters that are subsets of other clusters (pays runtime penalty)" << std::endl;
        std::cout << "-s, --max_cluster_size\tSet max cluster size (default 8)" << std::endl;
        std::cout << "-i, --pi_delay\t\tSet delay for all primary input nodes (default 0)" << std::endl;
        std::cout << "-o, --po_delay\t\tSet delay for all primary output nodes (default 1)" << std::endl;
        std::cout << "-n, --node_delay\t\tSet delay for all non-pi and non-po nodes (default 1)" << std::endl;
        std::cout << "-c, --intercluster_delay\tSet intercluster delay (default 3)" << std::endl;

        return 0;
    }

    if (UNIX_RUN){
        BLIFFile = FILENAME;
    }
    else {
        BLIFFile = "../" + FILENAME;
    }

    if(USE_LAWLER_LABELING){
        USE_DELAY_MATRIX = false;//delay matrix should not be calculated for lawler labeling
    }


    /*
    std::cout << "RWClustering Application" << "\nAuthors: Akshay Nagendra <akshaynag@gatech.edu>, Paul Yates <pyates6@gatech.edu>" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Input File: " << FILENAME.c_str() << std::endl;
    std::cout << "Max Cluster Size = " << MAX_CLUSTER_SIZE << std::endl;
    std::cout << "PI Node Delay = " << PRIMARY_INPUT_DELAY << std::endl;
    std::cout << "PO Node Delay = " << PRIMARY_OUTPUT_DELAY << std::endl;
    std::cout << "Normal Node Node Delay = " << NODE_DELAY << std::endl;
    std::cout << "Inter Cluster Delay = " << INTER_CLUSTER_DELAY << std::endl;
    if(USE_LAWLER_LABELING){
        std::cout << "Using Lawler Labeling" << std::endl;
    }
    if(USE_DELAY_MATRIX) {
        std::cout << "Using Delay Matrix" << std::endl;
    }
    if (USE_GUI){
        std::cout << "Interctive GUI: Enabled" << std::endl;
    }
    else {
        std::cout << "Interctive GUI: Disabled" << std::endl;
    }
    */
    std::vector<Node> rawNodeList;

    auto parsestart = sc::high_resolution_clock::now();
    parseBLIF(BLIFFile,PRIMARY_INPUT_DELAY,PRIMARY_OUTPUT_DELAY,NODE_DELAY,rawNodeList);


    //DEBUG
    /*
    std::cout << "NODE ORDER: [";
    int idN = 0;
    for (std::vector<Node>::iterator iN = rawNodeList.begin(); iN < rawNodeList.end(); ++iN){
        std::cout << iN->strID << "(" << idN << "),";
        idN += 1;
    }
    std::cout << "]" << std::endl;
     */

    // REQUIREMENT: All arrays containing node objects MUST point to rawNodeList. No copies of Nodes may be made at any time.
    std::vector<Node *> PIs = obtainPINodes(rawNodeList);
    std::vector<Node *> POs = obtainPONodes(rawNodeList);
    auto parseEnd = sc::high_resolution_clock::now();

    std::cout << "Parsing and Population (PI & PO) Complete" << std::endl;

    int N = rawNodeList.size(); //the number of total nodes


    std::vector<Node *> master;
    auto topoStart = sc::high_resolution_clock::now();
    for(auto out : POs){ //recursively add all nodes to master in topological order
        addPredecessors(master, out);
    }
    auto topoEnd = sc::high_resolution_clock::now();

    //DEBUG (SHOULD BE DELETED); JUST FOR CHECKING WITH MY HANDWRITTEN SOLUTION
    /*master.clear();
    int indices[12] = {0,1,2,5,6,7,8,9,10,11,3,4};
    for (int i=0; i < 12; ++i){
        master.push_back(&rawNodeList.at(indices[i]));
    }
    */

    //number the nodes in order for use in indexing the delay_matrix array

    auto labelInitialStart = sc::high_resolution_clock::now();
    uint32_t id = 0;
    for(auto node : master){
        node->id = id++;
        //apply initial labeling
        if(!USE_LAWLER_LABELING){
            if(node->isPI) {
                node->label = node->delay; //For RW, PIs start at node delay
            }
        }
    }
    auto labelInitialEnd = sc::high_resolution_clock::now();

    //Abort GUI if too large for GUI to handle or if using non-pure Rajaraman-Clustering
    if (master.size() > GUI_NODE_CLUSTERSIZE_LIMIT || MAX_CLUSTER_SIZE > GUI_NODE_CLUSTERSIZE_LIMIT || USE_EXP2) {
        USE_GUI = 0;
    }

    //DEBUG
    /*
    std::cout << "TOPOLOGICAL ORDER: [";
    for (std::vector<Node*>::iterator iM = master.begin(); iM < master.end(); ++iM){
        std::cout << (*iM)->strID << ",";
    }
    std::cout << "]" << std::endl;
    */

    auto delayMStart = sc::high_resolution_clock::now();
    int* delay_matrix;
    SparseMatrix sparse_delay_matrix(N,N);
    if(USE_DELAY_MATRIX) {
        //////     COMPUTE DELAY MATRIX //////
        // delay_matrix[x][y] = max delay from output x to output y (node delay only)
        if(!USE_SPARSE) {
            delay_matrix = new int[N * N]; // Delay matrix is NxN square matrix.
        }

        //delay_matrix[N*r+c] (aka delay_matrix[r][c]) represents max delay from node r to node c
        //the matrix entry = 0 if c precedes r in topological order
        std::vector<Node *>::const_iterator r; //the node corresponding to the current matrix row
        std::vector<Node *>::const_iterator c; // the node corresponding to the  current matrix column

        for (r = master.begin(); r != master.end(); ++r) { // iterate across every row
            //delay between a node and any previous node (and itself) is 0
            if(!USE_SPARSE) {
                for (c = master.begin(); c != r + 1; ++c) delay_matrix[N * (*r)->id + (*c)->id] = 0;
            }
            for (c = r + 1; c != master.end(); ++c) {
                //max_delay(r,c) = max( max_delay(r, c->prev) )
                int max = 0;
                int prev_delay;
                for (Node *p : (*c)->prev) {
                    if(USE_SPARSE){
                        prev_delay = sparse_delay_matrix.get((*r)->id, p->id);
                    }
                    else {
                        prev_delay = delay_matrix[N * (*r)->id + p->id];
                    }
                    if (prev_delay > max) {
                        max = prev_delay;
                        //std::cout << "Found that " << (*c)->strID << "'s predecessor, " << (*p)->strID << ", has delay " << prev_delay << " to " << (*r)->strID << std::endl; //debug
                    }
                }
                //if no predecessors of c have a delay to r, then either r is a direct predecessor, or there is no link
                if (max == 0) {
                    if(!USE_SPARSE) {
                        delay_matrix[N * (*r)->id + (*c)->id] = 0;
                    }
                    for (Node *p2 : (*c)->prev) {
                        if (p2->id == (*r)->id) {
                            if(USE_SPARSE){
                                sparse_delay_matrix.set((*r)->id,(*c)->id,(*c)->delay);
                            }
                            else {
                                delay_matrix[N * (*r)->id + (*c)->id] = (*c)->delay;
                            }
                        }
                    }
                } else {
                    if(USE_SPARSE){
                        sparse_delay_matrix.set((*r)->id, (*c)->id, (*c)->delay + max);
                    }
                    else {
                        delay_matrix[N * (*r)->id + (*c)->id] = (*c)->delay + max;
                    }
                }
                //std::cout << "Delay from " << (*r)->strID << " to " << (*c)->strID << " is " << delay_matrix[N*(*r)->id+(*c)->id]<< std::endl; //debug
            }

        }
        std::cout << "Delay Matrix Calculation Complete" << std::endl;
    }
    auto delayMEnd = sc::high_resolution_clock::now();

    /*
    // DEBUG
    if(USE_DELAY_MATRIX) {
        std::cout << "DELAY MATRIX:" << std::endl;
        for (auto m : master) {
            std::cout << "\t" << m->strID;
        }
        std::cout << std::endl;
        for (uint32_t i = 0; i < N; ++i) {
            std::cout << master.at(i)->strID;
            for (int j = 0; j < N; ++j) {
                if(USE_SPARSE){
                    std::cout << "\t" << sparse_delay_matrix.get(i,j);
                }
                else {
                    std::cout << "\t" << delay_matrix[N * i + j];
                }
            }
            std::cout << std::endl;
        }
    }
    /*
    // check delay matrix against max_delay calculation (DEBUG)
    bool max_delay_consistent = true;
    std::cout << "MAX DELAY CALC RESULTS:" << std::endl;
    for (auto m : master){
        std::cout << "\t" << m->strID;
    }
    std::cout << std::endl;
    for(uint32_t i = 0; i < N; ++i){
        std::cout << master.at(i)->strID;
        for(uint32_t j=0; j < N; ++j){
            int m_d = max_delay(master.at(i),master.at(j),master);
            std::cout << "\t" << m_d;
            if(USE_DELAY_MATRIX) {
                if (m_d != delay_matrix[N * i + j]) max_delay_consistent = false;
            }

        }
        std::cout << std::endl;
    }
    ///*
    if(USE_DELAY_MATRIX) {
        if (max_delay_consistent) {
            std::cout << "max_delay function PASSED consistency check" << std::endl;
        } else {
            std::cout << "max_delay function FAILED consistency check" << std::endl;
        }
    }
     */



   //////      CALCULATE LABELS    ///////
    auto labelClusterStart = sc::high_resolution_clock::now();
    std::vector<Cluster> clusters;
    int maxIODelay = 0;
    int maxLabel = 0;
    if(!USE_LAWLER_LABELING) {

    // Let Gv be the subgraph containing v and all its predecessors
        // label_v(x) for all x in Gv\{v}
            // label_v(x) = label(x) + delay_matrix[x][v]
    // Let S be the set of nodes in Gv\{v} sorted by decreasing label_v values
        // remove node one-by-one in sorted order  from S and add it to cluster(v) until size constraint is met
        // let l1 = max(label_v) of any PI node in cluster(v)
        // let l2 = max(label_v+delay) of any node remaining in S
        // label(v) = max(l1,l2)


        //todo: consider optimizing this code by changing how and when the ordered set container is used

        for (auto v : master) {

            std::vector<Node *> S;

            for (auto n = master.begin(); n != master.begin()+v->id; ++n){ //set all predecessors' visited flag so we can get predecessors
                (*n)->visited = false;
            }

            //skip PIs (label(PI) = delay(pi) already implemented)
            if (!v->prev.empty()) {
                for (auto n : v->prev) {
                    addPredecessors(S, n);
                }
            }

            // calculate label_v(x)
            for (auto x : S) {
                if (USE_DELAY_MATRIX) {
                    if(USE_SPARSE){
                        x->label_v = x->label + sparse_delay_matrix.get(x->id,v->id);
                    }
                    else {
                        x->label_v = x->label + delay_matrix[N * x->id + v->id];
                    }
                } else {
                    x->label_v = x->label + max_delay(x, v, master);
                }
            }

            // sort S
            std::sort(S.begin(), S.end(), compare_lv);

            //DEBUG
            /*
            std::cout << "S for Node " << v->strID << " has." << std::endl;
            for(auto s : S){
                std::cout << s->strID << " ,";
            }
            std::cout << std::endl;
            */



            Cluster cl(v->id);
            cl.members.push_back(v);

            // pop first element from S and add to c until max cluster size reached or S is empty
            for (int i = 1; i < MAX_CLUSTER_SIZE; ++i) { //i starts at 1 to include initial element already in cluster
                if (S.empty()) break;
                cl.members.push_back(*S.begin());
                S.erase(S.begin());
            }

            if (!v->prev.empty()) {
                int L2 = 0;
                if (!S.empty()) {

                    L2 = (*S.begin())->label_v + INTER_CLUSTER_DELAY;
                }
                int L1 = cl.calcL1Value();
                //DEBUG
                //std::cout << v->strID << "'s L1 value: " << L1 << std::endl;
                //std::cout << v->strID << "'s L2 value: " << L2 << std::endl;


                v->label = (L1 > L2) ? L1 : L2;
            }
            maxLabel = (v->label > maxLabel) ? v->label : maxLabel;
            generateInputSet(cl);
            clusters.push_back(cl);
        }
    }
    else{
        // LAWLER LABELING ALGORITHM
        // For each PI node, label = 0 (already implemented above)
        // For each non PI node v
            // p = maximum label of predecessors
            // Xp = set of predecessors with label p
            // if |Xp| < MAX_CLUSTER_SIZE
                // L(v) = p
            // else
                // L(v) = p+1
        // nodes with the same label go in the same cluster

        for(auto v : master){ //traversing in topological order guarantees all predecessors of v will be labeled
            if(!v->isPI){
                int max = 0;
                int count = 0;
                for (auto n = master.begin(); n != master.begin()+v->id; ++n){ //set all predecessors' visited flag so we can get predecessors
                    (*n)->visited = false;
                }
                std::vector<Node *> pre;
                for(auto n : v->prev){
                    addPredecessors(pre, n);
                }
                for(auto p : pre){
                    if(p->label == max){ //keep a count of the number of predecessors with max label
                        ++count;
                    }
                    else if(p->label > max){
                        max = p->label;
                        count = 1;
                    }
                    /*if (USE_DELAY_MATRIX){
                        if (USE_SPARSE){
                            maxIODelay = (sparse_delay_matrix.get(p->id,v->id) > maxIODelay) ? sparse_delay_matrix.get(p->id,v->id) : maxIODelay;
                        }
                        else {
                            maxIODelay = (delay_matrix[N * p->id + v->id] > maxIODelay) ? delay_matrix[N * p->id + v->id] : maxIODelay;
                        }
                    }
                    else {
                      maxIODelay = (max_delay(p, v, master) > maxIODelay) ? max_delay(p, v, master) : maxIODelay;
                    }*/
                    maxIODelay = (max_delay(p, v, master) > maxIODelay) ? max_delay(p, v, master) : maxIODelay;
                }
                if(count < MAX_CLUSTER_SIZE){
                    v->label = max;
                }
                else{
                    v->label = max+1;
                }
            }
            maxLabel = (v->label > maxLabel) ? v->label : maxLabel;
        }
        for(auto n : master){ //prepare for recursive clustering, set visited node to false so we only add each node once
            n->visited = false;
        }
        for(auto PO : POs){
            lawler_cluster(master, PO, clusters);
        }


    }
    auto labelClusterEnd = sc::high_resolution_clock::now();

    std::cout << "Calculation of Labels and Clusters Complete" << std::endl;

    //DEBUG
    /*
    for (auto m : master){
        std::cout << "LABEL FOR NODE " << m->strID << ": " << m->label << std::endl;
    }
    */
    /*
    for(auto cluster : clusters){
        std::cout << "\nCluster " << cluster.id << "(" << master.at(cluster.id)->strID << ") has: " << std::endl;
        for(auto member : cluster.members){
            std::cout << member->id << " (" << member->strID << ")" << std::endl;
        }

        std::cout << "\nCluster " << cluster.id << "(" << master.at(cluster.id)->strID << ") has inputs: " << std::endl;
        for(auto in : cluster.inputSet){
            std::cout << in->id << " (" << in->strID << ")" << std::endl;
        }

    }
    */
    std::vector<Cluster *> finalClusterList;
    std::vector<std::vector<Node*>> L_HISTORY;
    auto clusterPhaseStart = sc::high_resolution_clock::now();
    if(!USE_LAWLER_LABELING) { //for RW
        //CLUSTERING PHASE
        std::vector<Node *> L;

        std::copy(POs.begin(), POs.end(), std::back_inserter(L)); //Generate L as the set of all POs in the circuit
        if (USE_GUI){
            L_HISTORY.push_back(L);
        }
        //using visited flag to tell if a node has been placed into the final cluster list
        for(Node* n : master){
            n->visited = false;
        }

        if (USE_EXP2){
            //experimental code to remove redundant clusters
            //Iterate through node list in reverse topological order
            //If a node is unvisited, add the cluster for which this node is the head
            //For each node in the added cluster, set visited
            //DOES NOT SUPPORT GUI
            for(auto n : master){
                n->visited = false;
            }
            for(auto it = master.rbegin(); it != master.rend(); ++it){
                if(!(*it)->visited) {
                    Cluster *cl = &(clusters.at((*it)->id));
                    finalClusterList.push_back(cl);
                    for(auto member : cl->members){
                        member->visited = true;
                    }
                }
            }
        }
        else {
            while (!L.empty()) {
                //retrieve first element of L and pop from L
                Node *lNode = *L.begin();
                L.erase(L.begin());

                //add cluster to finalClusterList
                Cluster *cl = &(clusters.at(lNode->id));
                finalClusterList.push_back(cl);

                if (USE_EXP) {
                    //Experiment Method
                    for (auto iNode : cl->inputSet) {
                        //std::cout << "Checking if " << iNode->strID << " can be excluded" << std::endl;
                        bool alreadyAdded = true;
                        for (auto n : clusters.at(iNode->id).members) {
                            if(!n->visited){
                                //std::cout << "Member " << n->strID << " has NOT been clustered" << std::endl;
                                alreadyAdded = false;
                            }
                            // DEBUG
                            /*
                            else{
                                std::cout << "Member " << n->strID << " HAS been clustered" << std::endl;
                            }
                            */
                        }
                        // DEBUG
                        if(alreadyAdded){
                            //std::cout << "Refusing to add node " << iNode->strID << " to L set" << std::endl;
                        }
                        if (!alreadyAdded && retrieveNodeByStr_ptr(iNode->strID, L) == nullptr) {
                            L.push_back(iNode);
                            for(Node* n : clusters.at(iNode->id).members){
                                n->visited = true;
                                //std::cout << n->strID << " was just clustered" << std::endl;
                            }
                        }
                    }
                } else {
                    //add any node in input(lNode's cluster) whose cluster is not in the finalClusterList
                    for (auto iNode : cl->inputSet) {
                        if (!Cluster::isClusterInList(iNode->id, finalClusterList) &&
                            retrieveNodeByStr_ptr(iNode->strID, L) == nullptr) {
                            L.push_back(iNode);
                        }
                    }
                }


                if (USE_GUI) {
                    L_HISTORY.push_back(L);
                }
            }
        }
    }
    else{ //for lawler labeling, just insert clusters into final cluster list as they are
        for(auto it = clusters.begin(); it != clusters.end(); ++it){
            finalClusterList.push_back(&(*it));
        }
    }
    auto clusterPhaseEnd = sc::high_resolution_clock::now();

    //DEBUG
    /*
    std::cout << "FINAL CLUSTER LIST: " << std::endl;
    for (auto c : finalClusterList){
        std::cout << "CLUSTER " << master.at(c->id)->strID << ": [";
        for (auto mem : c->members){
            std::cout << mem->strID << ",";
        }
        std::cout << "]" << std::endl;
    }
    */

    int maxIODelay_LAWLERMODIFIED = maxIODelay;
    if (!USE_LAWLER_LABELING){
        //RW Clustering has max label = max PI-PO delay
        maxIODelay = maxLabel;
    }
    else {
        maxIODelay = maxIODelay + maxLabel;
        maxIODelay_LAWLERMODIFIED = maxIODelay + maxLabel*INTER_CLUSTER_DELAY;
    }


    std::cout << "PROGRAM COMPLETE" << std::endl;

    //STATISTICS
    long int CLUSTER_AREA_COST = 0;
    float AREA_COST = 0.0f;
    //print to files
    writeOutputFiles(BLIFFile.substr(0,BLIFFile.length()-5),
                     master,clusters,finalClusterList,
                     MAX_CLUSTER_SIZE,INTER_CLUSTER_DELAY,PRIMARY_INPUT_DELAY,PRIMARY_OUTPUT_DELAY,NODE_DELAY,
                     USE_LAWLER_LABELING,USE_GUI,USE_EXP);
    if (USE_GUI && !USE_LAWLER_LABELING) {
        writeGUIFile(master, clusters, finalClusterList, L_HISTORY, maxIODelay, UNIX_RUN);
    }

    std::ofstream verboseFile;

    verboseFile.open("output_" + BLIFFile.substr(0, BLIFFile.length() - 5) + "_verbose.txt", std::fstream::app);


    std::vector<std::pair<long long int,std::string>> execTimes;
    std::vector<std::string> execStrs = {"PARSING AND POPULATION OF ALL NODES",
                                         "TOPOLOGICAL SORTING",
                                         "DELAY MATRIX CALCULATION",
                                         "LABEL INITIALIZATION",
                                         "LABEL AND INITIAL CLUSTERING PHASE",
                                         "CLUSTERING PHASE"};

    execTimes.push_back(measureExecTime(parsestart,parseEnd));
    execTimes.push_back(measureExecTime(topoStart,topoEnd));
    execTimes.push_back(measureExecTime(delayMStart,delayMEnd));
    execTimes.push_back(measureExecTime(labelInitialStart,labelInitialEnd));
    execTimes.push_back(measureExecTime(labelClusterStart,labelClusterEnd));
    execTimes.push_back(measureExecTime(clusterPhaseStart,clusterPhaseEnd));

    //print out execution times for each code section
    std::cout << "\n------------------------------------\nSTATISTICS:" << std::endl;
    verboseFile << "\n----------STATISTICS----------\n" << std::endl;
    std::cout << "TOTAL NUMBER OF NODES:\t" << N << std::endl;
    verboseFile << "TOTAL NUMBER OF NODES:\t" << N << std::endl;
    std::cout << "NUMBER OF CLUSTERS:\t" << finalClusterList.size() << std::endl;
    verboseFile << "NUMBER OF CLUSTERS:\t" << finalClusterList.size() << std::endl;
    if (USE_LAWLER_LABELING) {
        std::cout << "MAX LABEL:\t" << maxLabel << std::endl;
        verboseFile << "MAX LABEL:\t" << maxLabel << std::endl;
        std::cout << "MAX IO PATH DELAY (UNIT DELAY MODEL):\t" << maxIODelay << std::endl;
        verboseFile << "MAX IO PATH DELAY (UNIT DELAY MODEL):\t" << maxIODelay << std::endl;
        std::cout << "MAX IO PATH DELAY (GENERAL DELAY MODEL):\t" << maxIODelay_LAWLERMODIFIED << std::endl;
        verboseFile << "MAX IO PATH DELAY (GENERAL DELAY MODEL):\t" << maxIODelay_LAWLERMODIFIED << std::endl;
    }
    else {
        std::cout << "MAX IO PATH DELAY:\t" << maxIODelay << std::endl;
        verboseFile << "MAX IO PATH DELAY:\t" << maxIODelay << std::endl;
    }
    for (auto cl : finalClusterList){
        for (auto mem : cl->members){
            CLUSTER_AREA_COST += 1;
        }
    }
    std::cout << "----------EXECUTION TIMES----------" << std::endl;
    verboseFile << "\n----------EXECUTION TIMES----------\n" << std::endl;
    for (uint32_t i=0; i < execStrs.size(); ++i){
        std::cout << execStrs.at(i) << ":\t" << execTimes.at(i).first << execTimes.at(i).second << std::endl;
        verboseFile << execStrs.at(i) << ":\t" << execTimes.at(i).first << execTimes.at(i).second << std::endl;
    }

    verboseFile.close();
    if(UNIX_RUN){
        reportMemUsage(BLIFFile.substr(0, BLIFFile.length() - 5));
    }
    verboseFile.open("output_" + BLIFFile.substr(0, BLIFFile.length() - 5) + "_verbose.txt", std::fstream::app);

    std::cout << "----------AREA ANALYSIS----------" << std::endl;
    verboseFile << "\n----------AREA ANALYSIS----------\n" << std::endl;
    std::cout << "NOTE: Assuming unit area per node" << std::endl;
    verboseFile << "NOTE: Assuming unit area per node" << std::endl;
    std::cout << "ORIGINAL AREA:\t" << N << std::endl;
    verboseFile << "ORIGINAL AREA:\t" << N << std::endl;
    std::cout << "CLUSTERED AREA:\t" << CLUSTER_AREA_COST << std::endl;
    verboseFile << "CLUSTERED AREA:\t" << CLUSTER_AREA_COST << std::endl;
    std::cout << "AREA INCREASED BY FACTOR OF " << ((float) CLUSTER_AREA_COST)/((float)N) << std::endl;
    verboseFile << "AREA INCREASED BY FACTOR OF " << ((float) CLUSTER_AREA_COST)/((float)N) << std::endl;

    verboseFile.close();

    if(USE_DELAY_MATRIX && !USE_SPARSE) {
        delete[] delay_matrix;
    }

    return 0;
}

//adds a node and its predecessors to vector in topological order
void addPredecessors(std::vector<Node *> &m, Node *n){
    if(n->visited) return;
    for(auto node : n->prev){
        if (!node->visited) addPredecessors(m, node);
    }
    n->visited = true; //todo: remove the need for a visited flag by using a more efficient method
    m.push_back(n);
    //std::cout << "Adding " << n->strID  << " to master." << std::endl; //debug
}

//find longest path in the DAG using topological ordering properties
//requirement: nodes must be topologically sorted with sequential IDs (starting at 0)
int max_delay(Node* src, Node* dst, std::vector<Node *> nodes){
    if(src->id >= dst->id) return 0; //no path between these nodes if src does not come before dst
    int offset = src->id; //use offset to avoid making the delays array longer than necessary
    int delays[dst->id - offset + 1]; //a delay value for all topological nodes between src and dst (inclusive)
    for(int i=0; i<=dst->id-offset; ++i){ //initialize delays vector to -1 for each node
        delays[i] = -1;
    }
    //set delay of source to 0
    delays[0] =  0;
    for(int i=0; i<dst->id-offset; ++i){
        if(delays[i] != -1){
            for(auto it = nodes[i+offset]->next.begin(); it != nodes[i+offset]->next.end(); ++it){
                if((*it)->id <= dst->id) { //don't operate on nodes which come topologically after dst
                    if(delays[(*it)->id - offset] < delays[i]+(*it)->delay){
                        delays[(*it)->id - offset] = delays[i]+(*it)->delay;
                    }
                }
            }
        }
    }
    if(delays[dst->id - offset] == -1) return 0; //if there was no path from src to dst, return 0
    return delays[dst->id - offset];
}
//similar to addPredecessors, but only adds the node if it has the specified label
void get_lawler_cluster(std::vector<Node *> &nodes, Node* n, int p, bool* visited){
    if (visited[n->id]) return;
    if(n->label == p){
        for(auto prev : n->prev){
            get_lawler_cluster(nodes, prev, p,visited);
        }
        visited[n->id] = true;
        nodes.push_back(n);
    }
}
void lawler_cluster(std::vector<Node*> &m, Node* n, std::vector<Cluster> &clusters){
    if(!n->visited) {
        bool cluster = true;
        for (auto suc : n->next) { //for each of n's successors
            if (n->label == suc->label) {
                cluster = false;
            }
        }
        if (cluster) {
            std::vector<Node *> clust;
            bool visited[n->id+1];
            for(int i=0; i<=n->id; ++i){
                visited[i] = false;
            }
            get_lawler_cluster(clust, n, n->label,visited);
            Cluster newCluster(n->id);
            for (auto c : clust) {
                newCluster.members.push_back(c);
            }
            clusters.push_back(newCluster);
        }
    }
    n->visited = true;
    for(auto p : n->prev){
        lawler_cluster(m,p, clusters);
    }
}
