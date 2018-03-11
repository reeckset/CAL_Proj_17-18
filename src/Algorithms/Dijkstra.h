//
// Created by reeckset on 10-03-2018.
//

#ifndef CAL_DIJKSTRA_H
#define CAL_DIJKSTRA_H

#include "../Graph/Graph.h"
#include "../Graph/DNode.h"
#include <set>
#include <unordered_set>

using namespace std;

template<typename N>
struct DNodeHash {
    bool operator()(const DNode<N> &d1, const DNode<N> &d2) const {
        return d1 == d2;
    }

    int operator()(const DNode<N> &d) const {
        return d.getId();
    }
};

template<typename N>
class Dijkstra {

    const Graph<N> &graph;
    set<DNode<N> > pQueue;
    unordered_set<DNode<N>, DNodeHash<N>, DNodeHash<N>> checkedDNodes;

    //Variables for current calculation
    const Node<N> &finishNode;
    const Node<N> &startNode;
    double solutionTotalCost = DBL_MAX;
    DNode<N> topDNode;

    vector<Node<N> > lastSolution;

    //Sets all queue's elements to the nodes in the graph and then puts the first node on top
    void populateQueue() {
        for (u_int i = 0; i < graph.getNumNodes(); i++) {
            if(i == this->startNode.getId()){
                pQueue.emplace(graph.getNodeById(i),0);
            }else {
                pQueue.emplace(graph.getNodeById(i));
            }
        }
    }

    //Gets the node in the queue for a specified id
    DNode<N> getDNodeInQueueById(u_int id) {
        for (DNode<N> d : pQueue) {
            if (d.getId() == id) {
                return d;
            }
        }
    }

    void updateDNodeOnQueue(const DNode<N> &currDNode) {

        //check if the current node has been analised (if it has, it will be in checkedDNodes)
        auto dNodeIt = this->checkedDNodes.find(currDNode);
        if (dNodeIt != this->checkedDNodes.end()) { //Exists in checkedDNodes
            if (currDNode.getTotalWeight() < dNodeIt->getTotalWeight()) {
                this->pQueue.erase(*dNodeIt);
                this->pQueue.insert(currDNode);
                return;
            }
        }

        //if it has not been analised and the current path offers a better way, update it on the priority queue
        for (DNode<N> d : pQueue) {
            if (d.getId() == currDNode.getId()) {
                if (currDNode.getTotalWeight() < d.getTotalWeight()) {
                    pQueue.erase(d);
                    pQueue.insert(currDNode);
                    return;
                }
            }
        }
    }

    //Takes the finish node that should be on top of the queue and creates a path from recurrent previous nodes
    vector<Node<N> > buildPath() {
        vector<Node<N> > result;
        u_int currDNodeId = this->finishNode.getId();
        while (currDNodeId != UINT_MAX) {
            result.insert(result.begin(), getCheckedNode(currDNodeId));
            currDNodeId = getCheckedNode(currDNodeId).getLastNodeId();
        }
        return result;
    }

    //Goes through queue's top node's children and updates them in the queue
    void updateQueue(){
        for (Edge<N> e : this->topDNode.edges) {
            DNode<N> currDNode = getDNodeInQueueById(e.destNode->getId()); //get edge's destination
            currDNode.setTotalWeight(topDNode.getTotalWeight() + e.value); //update node's value
            currDNode.setLastNodeId(topDNode.getId()); //set node's last node id (for path building later)
            updateDNodeOnQueue(currDNode); //
        }
    }

    //Sets values of topDNode based on the current pQueue
    void updateTopDNode(){
        this->topDNode = *(pQueue.begin());
    }

    //Checks if the node on top of the queue is a dead end
    bool isTopDNodeDeadEnd(){
        if (this->topDNode.edges.empty() && this->topDNode != finishNode) {
            this->pQueue.erase(this->pQueue.begin());
            return true;
        }
        return false;
    }

    //checks if the optimal solution has been found (if final node is on top of the queue)
    bool foundOptimalSolution(){
        if (this->topDNode == this->finishNode) {
            this->solutionTotalCost = this->pQueue.begin()->getTotalWeight();
            this->checkedDNodes.insert(topDNode);
            return true;
        }
        return false;
    }

    //retrived a node in checkedNodes by its id
    DNode<N> getCheckedNode(u_int id){
        return *(this->checkedDNodes.find(DNode<N>(id)));
    }

public:

    Dijkstra(const Graph<N> &graph, const Node<N> &startNode, const Node<N> &finishNode): graph(graph), startNode(startNode), finishNode(finishNode){}

    //CALCULATE OPTIMAL PATH
    vector<Node<N> > calcOptimalPath() {
        populateQueue();
        updateTopDNode();
        while (!pQueue.empty()) {
            //Analise the node on top of the priority queue

            //Check if node is a dead end
            if(isTopDNodeDeadEnd()) {
                continue;
            }

            //Check if already reached the optimal solution (finish node will be on top of the queue)
            if(foundOptimalSolution()) {
                this->lastSolution = buildPath();
                return lastSolution;
            }

            //Analise next nodes and updateQueue
            updateQueue();

            //Delete parent from pQueue and put it in the hash table
            pQueue.erase(this->topDNode);
            checkedDNodes.insert(this->topDNode);
            updateTopDNode(); //set this->topDNode
        }
        this->lastSolution = vector<Node<N> >();
        return lastSolution;
    }

    void printSolution(){
        cout << endl;
        if(pQueue.empty()){
            cout << "No solution available. Run calcOptimalSolution before printing" << endl;
            return;
        }
        cout << "Dijkstra optimal path between " << this->startNode.getId() << " and " << this->finishNode.getId() << endl;
        if(lastSolution.empty()){
            cout << "No path could be calculated. Could not print." << endl;
            return;
        }
        cout << "Successfully generated with a total weight of " << getCheckedNode(finishNode.getId()).getTotalWeight() << ":" << endl;
        for(Node<char> n : lastSolution){
            cout << n.getId() << " - " << n.getData() << endl;
        }
    }
};

#endif //CAL_DIJKSTRA_H