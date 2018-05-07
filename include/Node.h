//
// Created by Paul on 3/1/2018.
//

#ifndef RW_NODE_H
#define RW_NODE_H


#include <vector>
#include <set>
#include <string>

class Node {
private:
public:
    void computeLabel();
    std::vector<Node *> prev;
    std::vector<Node *> next;
    int delay; //ASSUME INTEGER DELAY
    uint32_t id;
    bool visited;
    int label = 0;
    int label_v = 0;

    bool isPI = false;
    bool isPO = false;
    std::string strID = "INVALID";  //TODO: RIP OUT LATER (DEBUG)
    std::string procStr = ""; //used for second run through during BLIF parsing
    Node *addr = nullptr;

    Node(){
        delay = 1;
        visited = false;
    }
    Node(int d){
        delay = d;
        visited = false;
    }
};

bool compare_lv (const Node* lhs, const Node* rhs);

#endif //RW_NODE_H
