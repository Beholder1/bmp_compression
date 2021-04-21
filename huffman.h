#include <SDL2/SDL.h>

/*Klasy_drzewa_Huffmana______________________________________________*/

class Node
{
    Uint8 value;
    double probability;
    bool emptyNode;
    Node* left;
    Node* right;
    Node* next;
public:
    Node(double p, Uint8 v = 0)
        : value(v), probability(p), emptyNode(false)
        , next(nullptr), left(nullptr), right(nullptr) {}

    ~Node()
    {
        delete left;
        delete right;
        delete next;
    }

    void setLeft(Node* l)
    {
        left = l;
    }
    void setRight(Node* r)
    {
        right = r;
    }
    void setNext(Node* n)
    {
        next = n;
    }
    void setProbability(double p)
    {
        probability = p;
    }
    void setValue(Uint8 v)
    {
        value = v;
    }
    void setEmptyNode(bool e)
    {
        emptyNode = e;
    }
    Node* getLeft()
    {
        return left;
    }
    Node* getRight()
    {
        return right;
    }
    Node* getNext()
    {
        return next;
    }
    double getProbability()
    {
        return probability;
    }
    Uint8 getValue()
    {
        return value;
    }
    bool getEmptyNode()
    {
        return emptyNode;
    }
    bool leaf()
    {
        if(left == nullptr && right == nullptr)
            return true;

        return false;
    }
};

class H_tree
{
    Node* root;
    Node* tail;
public:
    H_tree(double p, Uint8 v)
        : root(new Node(p, v))
    {
        root->setNext(nullptr);
    }

    ~H_tree()
    {
        delete root;
    }

    Node* getRoot()
    {
        return root;
    }

    void insert(double p, Uint8 v)
    {
        if(completeList())
            return;

        if(root->getNext() == nullptr && p > root->getProbability())
        {
            root->setNext(new Node(p, v));
            return;
        }
        else if(root->getNext() == nullptr && p < root->getProbability())
        {
            Node* tmp = root;
            root = new Node(p, v);
            root->setNext(tmp);
            return;
        }

        Node* tmp = root;
        Node* newNode = new Node(p, v);

        while(tmp->getNext() != nullptr)
        {
            if(tmp->getNext()->getProbability() < p)
            {
                tmp = tmp->getNext();
            }
            else break;
        }

        if(tmp == root && root->getProbability() > p)
        {
            root = newNode;
            root->setNext(tmp);
            return;
        }

        newNode->setNext(tmp->getNext());
        tmp->setNext(newNode);

        if(completeList())
        {
            std::cout << "\n\nComplet list\n\n";
            treeGrowth();
        }

    }

    Node* insert2(double p)
    {
        if(root->getProbability() > p)
        {
            Node* tmp = root;
            root = new Node(p);
            root->setNext(tmp);
            return root;
        }

        if(root == tail)
        {
            root = new Node(p);
            root->setNext(tail);
            return root;
        }

        Node* tmp = root;
        Node* newNode = new Node(p);

        while(tmp->getNext() != tail)
        {
            if(tmp->getNext()->getProbability() < p)
                tmp = tmp->getNext();
            else break;
        }

        newNode->setNext(tmp->getNext());
        tmp->setNext(newNode);
        return newNode;
    }

    bool completeList()
    {
        int count = 0;
        Node* tmp = root;

        while(tmp->getNext() != nullptr)
        {
            count ++;
            tmp = tmp->getNext();
        }

        count ++;
        tail = tmp;

        if(count == 32)
            return true;

        return false;
    }

    void treeGrowth()
    {
        while(root->getNext() != tail)
        {
            Node* tmp = root;
            root = root->getNext()->getNext();
            Node* tmpRoot = insert2(tmp->getProbability() + tmp->getNext()->getProbability());
            tmpRoot->setLeft(tmp);
            tmpRoot->setRight(tmp->getNext());
            tmpRoot->setEmptyNode(true);
            tmp->setNext(nullptr);
            tmpRoot->getRight()->setNext(nullptr);
        }
        Node* tmp = new Node(root->getProbability() + tail->getProbability());
        tmp->setLeft(tail);
        tmp->setRight(root);
        tmp->setEmptyNode(true);
        root->setNext(nullptr);
        tail->setNext(nullptr);
        root = tmp;
        tail = root;
    }

    void HuffmanDictionary(Uint32* bitWay)
    {
        Node* tmp = root;
        Uint32 tmpBit = 0;

        while(!root->leaf())
        {
            if(tmp->getRight() != nullptr && tmp->getRight()->leaf())
            {
                if(tmp->getRight()->getEmptyNode() == false)
                {
                    tmpBit <<= 1;
                    tmpBit += 1;
                    bitWay[tmp->getRight()->getValue()] = tmpBit;
                }
                tmpBit = 0;
                tmp->setRight(nullptr);
                tmp = root;
            }
            else if(tmp->getRight() != nullptr)
            {
                tmp = tmp->getRight();
                tmpBit <<= 1;
                tmpBit += 1;
            }
            else if(tmp->getLeft() != nullptr && tmp->getLeft()->leaf())
            {
                if(tmp->getLeft()->getEmptyNode() == false)
                {
                    tmpBit <<= 1;
                    bitWay[tmp->getLeft()->getValue()] = tmpBit;
                }
                tmpBit = 0;
                tmp->setLeft(nullptr);
                tmp = root;
            }
            else if(tmp->getLeft() != nullptr)
            {
                tmp = tmp->getLeft();
                tmpBit <<= 1;
            }
        }
    }
};

/*Funkcje do tworzenia slownika______________________________________*/

Uint32* Dictionary(palette_5bit* palette, double count)
{
    H_tree huffman(palette[0].count/count, palette[0].bit5);

    for(int i = 1; i < 32; i++)
        huffman.insert(palette[i].count/count, palette[i].bit5);

    Uint32* huffDict = new Uint32[32];

    huffman.HuffmanDictionary(huffDict);

    return huffDict;
}

int* dictCodeLength(Uint32* dictionary)
{
    int count;
    int* countTable = new int[32];
    Uint32 tmp;

    for(int i = 0; i < 32; i++)
    {
        tmp = dictionary[i];
        count  = 0;

        while(tmp > 0)
        {
            tmp = tmp/2;
            count++;
        }

        countTable[i] = count;
        if(dictionary[i] == 0)
            countTable[i] = 1;
    }

    return countTable;
}

int huffDictSearch(Uint32 word, Uint32* dict)
{
    for(int i = 0; i < 32; i++)
    {
        if(dict[i] == word)
            return i;
    }

    return -1;
}

void huffmanCompresion(bufor& link_variable, Uint32 huffWord, int codeLength, std::ofstream& output)
{
    Uint32 tmp = huffWord;

    if(link_variable.free >= codeLength)
    {
        link_variable.free -= codeLength;
        link_variable.variable <<= codeLength;
        link_variable.variable |= tmp;

        if(link_variable.free == 0)
        {
            output.write((char*)&link_variable.variable, sizeof(Uint32));
            link_variable.variable = 0;
            link_variable.free = 32;
        }
    }
    else if(link_variable.free < codeLength)
    {
        link_variable.dif = codeLength - link_variable.free;
        tmp >>= link_variable.dif;
        link_variable.variable <<= link_variable.free;
        link_variable.variable |= tmp;
        output.write((char*)&link_variable.variable, sizeof(Uint32));

        tmp = huffWord;
        tmp <<= 32 - link_variable.dif;
        tmp >>= 32 - link_variable.dif;
        link_variable.free = 32 - link_variable.dif;
        link_variable.variable = tmp;
    }
}

void huffmanDecompres(bufor& link_variable, Uint32 currBufor, Uint32* huffDict, int* codeLength, std::vector<Uint8>& indexTab)
{
    link_variable.dif = 32;
    Uint32 localBuffor = currBufor;
    Uint32 tmp;

    while(link_variable.dif > 0)
    {
        if(link_variable.free == 32)
        {
            link_variable.variable = localBuffor;
            link_variable.free -= link_variable.dif;
            link_variable.dif = 0;
        }
        else
        {
            if(link_variable.free >= link_variable.dif)
            {
                link_variable.variable <<= link_variable.dif;
                link_variable.variable |= localBuffor;
                link_variable.free -= link_variable.dif;
                link_variable.dif = 0;
            }
            else
            {
                link_variable.variable <<= link_variable.free;
                tmp = localBuffor;
                tmp >>= link_variable.dif - link_variable.free;
                link_variable.variable |= tmp;
                localBuffor <<= (32 - link_variable.dif + link_variable.free);
                localBuffor >>= (32 - link_variable.dif + link_variable.free);
                link_variable.dif -= link_variable.free;
                link_variable.free = 0;
            }
        }

        for(int i = 31 - link_variable.free; i >= 0; i--)
        {
            tmp = link_variable.variable;
            tmp >>= i;
            int index = huffDictSearch(tmp, huffDict);

            if(index != -1)
            {
                indexTab.push_back(index);
                link_variable.free += codeLength[index];
                link_variable.variable <<= link_variable.free;
                link_variable.variable >>= link_variable.free;
            }
        }
    }
}
