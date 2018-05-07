//
// Created by Paul on 3/1/2018.
//

#include "../include/Node.h"

// for ordering nodes in S set, nodes are ordered first by label, then by ID
bool compare_lv (const Node* lhs, const Node* rhs) {
    if(lhs->label_v == rhs->label_v){
        return lhs->id > rhs->id;
    }
    return lhs->label_v > rhs->label_v; //sorting should be in DECREASING order
}


