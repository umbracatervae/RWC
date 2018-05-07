//
// Created by Paul on 3/1/2018.
//

#include "../include/Cluster.h"


// computes a L1 value from a cluster
// let l1 = max(label_v) of any PI node in cluster(v)
int Cluster::calcL1Value(){
    int currentMax = 0;
    for(auto node: members){
        int m = 0;
        if(node->isPI) { // if *it is not a PI
            m = node->label_v;
        }
        if (m > currentMax){
            currentMax = m;
        }
    }
    return currentMax;
}
Cluster::Cluster(int id){
    this->id = id;
}
bool Cluster::isClusterInList(int cID,std::vector<Cluster*>& cList){
    for (auto c: cList){
        if (c->id == cID){
            return true;
        }
    }
    return false;
}

bool Cluster::isClusterInList_str(std::string sID,std::vector<Cluster*>& cList){
    for (auto c: cList){
        for (auto cMem : c->members){
            if (cMem->strID == sID){
                return true;
            }
        }
    }
    return false;
}