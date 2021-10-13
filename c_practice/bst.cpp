#include <iostream>
#include <stack>

class Node
{
public:
    Node(int value)
    {
        this->value = value;
        left = 0;
        right = 0;
    };

    int value;
    Node* left = nullptr;
    Node* right = nullptr;
};


void inOrder(Node* root)
{
    std::stack<Node* > theStack;
    Node* current = root;

    while (current != NULL || theStack.empty() == false)
    {
        while (current != NULL)
        {
            theStack.push(current);
            current = current->left;
        }
        // Once we reach the bottom, pop and print, then push right nodes
        current = theStack.top();
        theStack.pop();
        std::cout << current->value << std::endl;
        current = current->right;
    }
};


int main(void)
{
    Node* root = new Node(1);
    root->left = new Node(2);
    root->right = new Node(3);
    root->left->left = new Node(4);
    root->left->right = new Node(5);
    inOrder(root);
    return 0;
}
