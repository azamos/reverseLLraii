#include <iostream>
#include <memory>

class Node
{
    int value;
    std::unique_ptr<Node> next;
    static int constructionCounter;
    static int destructionCounter;

public:
    Node(int val) : value(val), next(nullptr)
    {
        /*TESTING ONLY: Added to the constructor for memory release validation*/
        constructionCounter++;
    }

    /*################################################################*/
    /*These 3 methods are solely for testing memory release*/

    /*1*/
    ~Node()
    {
        destructionCounter++;
    }

    /*2*/
    static int getConstructionCounter()
    {
        return constructionCounter;
    }

    /*3*/
    static int getDestructorCounter()
    {
        return destructionCounter;
    }
    /*                              END                               */
    /*################################################################*/

    int getValue() const
    {
        return value;
    }

    Node *getNext() const
    {
        return next.get();
    }

    void setNext(std::unique_ptr<Node> movable)
    {
        next = std::move(movable);
    }

    /* Need to be able to release control of next for remove and reverse methods */
    std::unique_ptr<Node> releaseNext()
    {
        return std::move(next);
    }
};

int Node::constructionCounter = 0;
int Node::destructionCounter = 0;

class LinkedList
{
    std::unique_ptr<Node> head;
    size_t linkSize;

public:
    LinkedList() : head(nullptr), linkSize(0) {}

    void insert(int newValue)
    {
        std::unique_ptr<Node> newNode = std::make_unique<Node>(newValue);
        newNode->setNext(std::move(head));
        head = std::move(newNode);
        linkSize++;
    }

    bool remove(int valueToDel)
    {
        if (isEmpty())
            return false;

        if (head->getValue() == valueToDel)
        {
            head = std::move(head->releaseNext());
            linkSize--;
            return true;
        }

        Node *curr = head.get();
        while (curr->getNext() && curr->getNext()->getValue() != valueToDel)
        {
            curr = curr->getNext();
        }
        if (!curr->getNext()) /*Means valueToDel was not found in the list*/
            return false;

        /*Being here means: the next node is the node to be deleted.
        Must bypass it and get to next next*/
        std::unique_ptr<Node> nextNext = std::move(curr->getNext()->releaseNext());
        std::unique_ptr<Node> nodeToDel = std::move(curr->releaseNext()); /*RAII - the ownership of next,
         which is the node needed to be deleted, will be transfered to nodeToDel, and so the resource will
         be released once leaving the scope of this function.*/
        curr->setNext(std::move(nextNext));
        linkSize--;
        return true;
    }

    void reverse()
    {
        std::unique_ptr<Node> prev;
        std::unique_ptr<Node> curr = std::move(head);
        std::unique_ptr<Node> next;
        while (curr)
        {
            next = std::move(curr->releaseNext());
            curr->setNext(std::move(prev));
            prev = std::move(curr);
            curr = std::move(next);
        }
        head = std::move(prev);
    }

    bool isEmpty() const
    {
        return head == nullptr;
    }

    size_t size() const
    {
        return linkSize;
    }

    void print() const
    {
        std::cout << std::endl
                  << "Printing the list...";
        if (isEmpty())
        {
            std::cout << std::endl
                      << "List is empty. Nothing to print.";
            return;
        }
        Node *curr = head.get();
        std::cout << std::endl
                  << "List size = " << size();
        std::cout << std::endl
                  << "########## START OF LIST ##########";
        std::cout << std::endl
                  << "head -> ";
        while (curr->getNext() != nullptr)
        {
            std::cout << curr->getValue() << "->";
            curr = curr->getNext();
        }
        std::cout << curr->getValue() << std::endl
                  << "########## END OF LIST ##########" << std::endl;
    }
};

constexpr int SIZE1 = 10;
constexpr int SIZE2 = 100;
constexpr int SIZE3 = 1000;

bool test1()
{
    std::cout << std::endl
              << "Starting test1 : inserting 1,...,10";
    std::cout << std::endl
              << "list should print 10,9,....,1";
    LinkedList l1;
    for (int i = 1; i <= SIZE1; i++)
    {
        l1.insert(i);
    }
    l1.print();
    std::cout << "\nRemoving all elements. List should be empty";
    for (int i = 1; i <= SIZE1; i++)
    {
        l1.remove(i);
    }
    l1.print();
    std::cout << "\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n";
    std::cout << "TEST1: l1 size = " << l1.size();
    return l1.size() == 0 && Node::getConstructionCounter() == Node::getDestructorCounter();
}

bool test2()
{
    std::cout << std::endl
              << "Starting test2 : inserting 1,...,100";
    std::cout << std::endl
              << "list should print 100,99,....,1";
    LinkedList l2;
    for (int i = 1; i <= SIZE2; i++)
    {
        l2.insert(i);
    }
    l2.print();
    std::cout << std::endl
              << "Removing 100,...,1";
    std::cout << std::endl
              << "list should print: 'List is empty. Nothing to print.' ";
    for (int i = SIZE2; i > 0; i--)
    {
        l2.remove(i);
    }
    l2.print();
    std::cout << "\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n";
    std::cout << "TEST2: l2 size = " << l2.size();
    return l2.size() == 0 && Node::getConstructionCounter() == Node::getDestructorCounter();
}

bool test3()
{
    std::cout << std::endl
              << "Starting test3 : inserting 1,...,1000";
    std::cout << std::endl
              << "list should print 1000,999,....,1";
    LinkedList l3;
    for (int i = 1; i <= SIZE3; i++)
    {
        l3.insert(i);
    }
    l3.print();
    std::cout << std::endl
              << "Reversing List...";
    std::cout << std::endl
              << "list should print: 1,...,999,1000 ";
    l3.reverse();
    l3.print();
    std::cout << std::endl
              << "Removing 1000,...,501";
    std::cout << std::endl
              << "list should print: 1,...,499,500 ";
    for (int i = SIZE3; i > SIZE3 / 2; i--)
    {
        l3.remove(i);
    }
    l3.print();
    std::cout << std::endl
              << "Reversing List...";
    std::cout << std::endl
              << "list should print: 500,499,...,1 ";
    l3.reverse();
    l3.print();
    std::cout << std::endl
              << "Removing 500,...,1";
    std::cout << std::endl
              << "list should print: empty list ";
    for (int i = SIZE3 / 2; i > 0; i--)
    {
        l3.remove(i);
    }
    l3.print();
    l3.reverse();
    l3.print();
    /*Note to self: if I change the number of deletions != insertions in one of the tests,
    the destructor counter won't match the destructor counter*/
    std::cout << "\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n";
    std::cout << "TEST3: l3 size = " << l3.size();
    return l3.isEmpty() && Node::getConstructionCounter() == Node::getDestructorCounter();
}

int main()
{
    int score = SIZE2;
    if (!test1())
    {
        std::cout << std::endl
                  << "Test 1 failed";
        score -= 10;
    }
    if (!test2())
    {
        std::cout << std::endl
                  << "Test 2 failed";
        score -= 20;
    }
    if (!test3())
    {
        std::cout << std::endl
                  << "Test 3 failed";
        score -= 70;
    }
    if (score == SIZE2)
        std::cout << std::endl
                  << "All tests passed. Score is: " << score;
    else
    {
        std::cout << std::endl
                  << "Some tests failed. Score is " << score;
        std::cout << "\nconstructior counter = " << Node::getConstructionCounter() << "\ndestructor counter " << Node::getDestructorCounter();
    }
    return 0;
}