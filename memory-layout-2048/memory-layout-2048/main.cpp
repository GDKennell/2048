//
//  main.cpp
//  memory-layout-2048
//
//  Created by Grant Kennell on 8/25/17.
//  Copyright © 2017 GrantKennell. All rights reserved.
//

#include <iostream>
#include <string>
#include <queue>

using namespace std;

const int MAX_DEPTH = 4;
int mem_num = 0;
int depth = 0;

string *out_names;
struct node
{
    string label;
    node *parent;
    int num_chidren;
    node *children;
};

queue<node *> nodeQueue;

void print_moves(string startStr);

void print_outcomes(int index)
{

}

void print_moves(int index)
{
    char moveChars[4] = {'L','R','U','D'};
    if (index == 0)
    {
        for (int i = 0; i< 4; ++i)
        {
            string thisMoveStr = moveChars[i] + to_string(1);
            out_names[i] = thisMoveStr;
        }
    }
    else
    {

    }
    if (index < MAX_DEPTH)
    {
        print_outcomes(index + 1);
    }
}

int index_for_move(int moveN, int depth)
{
    int multiplier = 1;
    for (int i = depth; i > 1; --i)
    {
        int tempMultiplier = (i % 2 == 0) ? 30 : 4;
        multiplier *= tempMultiplier;
        multiplier += 1;
    }
    return multiplier * moveN;
}
void add_outcome_children_to_move(node *move);

void add_move_children_to_outcome(node *outcome)
{
    if (depth >= MAX_DEPTH) return;
    depth++;

    string moveChars[4] = {"L","R","U","D"};

    outcome->num_chidren = 4;
    outcome->children = new node[4];

    for (int i = 0 ;i<4; ++i)
    {
        outcome->children[i].parent = outcome;
        outcome->children[i].label = outcome->label +" "+ moveChars[i] + to_string(depth);
        add_outcome_children_to_move(&outcome->children[i]);
    }
    depth--;
}

void add_outcome_children_to_move(node *move)
{
    if (depth >= MAX_DEPTH) return;
    depth++;

    move->num_chidren = 30;
    move->children = new node[30];
    for (int i = 0; i < 30; ++i)
    {
        move->children[i].parent = move;
        move->children[i].label = move->label +" "+ "O" + to_string(depth) + "," + to_string(i);
        add_move_children_to_outcome(&move->children[i]);
    }
    depth--;
}

void print_children(node *root)
{
    for (int i = 0; i < root->num_chidren; ++i)
    {
        cout<<root->children[i].label<<endl;
    }

}
void print_tree(node *root)
{
    print_children(root);
}
int main(int argc, const char * argv[]) {
    string moveChars[4] = {"L","R","U","D"};

    node root;
    add_move_children_to_outcome(&root);
    nodeQueue.push(&root);
    while (!nodeQueue.empty())
    {
        node *thisNode = nodeQueue.front();
        nodeQueue.pop();
        cout<<thisNode->label<<endl;
        for (int i = 0; i < thisNode->num_chidren; ++i)
        {
            nodeQueue.push(&thisNode->children[i]);
        }
    }
//    add_move_children_to_outcome(&root);

    return 0;
}
