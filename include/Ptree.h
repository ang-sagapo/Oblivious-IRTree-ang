#ifndef PTREE_H
#define PTREE_h

#include "Node.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <fstream>
#include <cstdlib>
#include <cctype>
#include <chrono>
#include <set>

using namespace std;

struct kNNUtil {
    bool isleaf;
    double rele;
    Branch* pBranch;

};
string Com = string("flag");
const int max_datasize = 1e5+5;
class Ptree :public Node{ 
public:
    set<Branch*> temp_stash;//����ר�Ŵ洢δд�صĽڵ����ݡ�
    set<Branch*> stash;
    int branch_num;
    int node_num;
    int leaf_node_num;
    int data_num;
    int all_leaf_num;
    int tree_deep;
    const char *data[max_datasize];
    
    vector<Node*> leaf_position_map;
    vector<Branch*> position_branchs;
    map<int, string> text_position_map;
    vector<Node*> nodes;
    Node* root;

    Ptree()
    {
        tree_deep = 0;
        leaf_node_num = 0;
        branch_num = 0;
        node_num = 0;
        data_num = 0;
        all_leaf_num = 0;
        ifstream dicFile;
        key = new CryptoPP::byte[PathORAM::key_length];
        dicFile.open("C:\\Z.Sagapo\\code\\Linux\\Project1\\test1\\data\\output2.txt");
        if (!dicFile.is_open())
        {
            printf("error!!!");
        }
        string _line;
        int _i = 0;
        while (getline(dicFile,_line))
        {
            istringstream _iss(_line);
            _iss >> dic_str[_i];
            dic_map[_i] = dic_str[_i];
            _i++;
        }
        for (int i = 0; i < dic_str.size(); i++)
        {
            Lower(dic_str[i]);
            Lower(dic_map[i]);
        }
        dicFile.close();
        ifstream inputFile;
        inputFile.open("C:\\Z.Sagapo\\code\\Linux\\Project1\\test1\\data\\dataset2.txt");
        if (!inputFile.is_open())
        {
            printf("error!!!");
        }
        string line;
        int temp_ID = 0;
        while(getline(inputFile,line))
        {
            istringstream iss(line);
            Branch *mBranch = new Branch;
            mBranch->is_virtual = false;
            
            
            string data_line = to_string(branch_num);
            string temp_line;
            PathORAM::aes_encrypt(data_line, key, temp_line);
            mBranch->trueData = temp_line;
            mBranch->pointBranch = mBranch;
            mBranch->isEmpty = false;
            string text;
            double p1 = 0, p2 = 0;
            iss >> text >> p1 >> p2;
            string temp_text;
            PathORAM::aes_encrypt(text, key, temp_text);
            mBranch->text = temp_text;
            mBranch->m_rect.min_Rec[0] = p1;
            mBranch->m_rect.min_Rec[1] = p2;
            mBranch->m_rect.max_Rec[0] = p1;
            mBranch->m_rect.max_Rec[1] = p2;
            mBranch->CalcuKeyWordRele(text);
            
            mBranch->ID = temp_ID;
            mBranch->textID = temp_ID;
            text_position_map.insert(pair<int,string>(temp_ID, temp_text));
            temp_ID++;
            position_branchs.push_back(mBranch);
            
            branch_num++;
            mBranch->level = 0;
        }
        data_num = branch_num;
        int deep = int(ceil(log(maxsize) / log(MAX_SIZE))) + 1;
        branch_num = int(ceil((pow(MAX_SIZE, deep)) / (MAX_SIZE - 1)));
        all_leaf_num = int(pow(MAX_SIZE, deep - 1));
        for (int i = data_num; i < all_leaf_num; i++)
        {
            Branch* virtual_branch = new Branch();
            int virtual_ID = -1;
            string tmp_line = PathORAM::generate_random_block(B - PathORAM::aes_block_size - sizeof(uint32_t));
            string vid_str = string((const char*)(&virtual_ID), sizeof(uint32_t));
            string cipher;
            PathORAM::aes_encrypt(tmp_line + vid_str, key, cipher);
            virtual_branch->trueData = cipher;
            virtual_branch->is_virtual = true;
            virtual_branch->ID = temp_ID;
            virtual_branch->textID = temp_ID;
            string text_cipher;
            PathORAM::aes_encrypt(Com, key, text_cipher);
            virtual_branch->text = text_cipher;
            text_position_map.insert(pair<int, string>(temp_ID, Com));
            temp_ID++;
            position_branchs.push_back(virtual_branch);
        }
        inputFile.close();
        printf("The inition of the Data block is completed......");
        node_num = int(ceil(branch_num / MAX_SIZE));
        sort(position_branchs.begin(), position_branchs.begin() + data_num, [](Branch* brancha, Branch* branchb) {return brancha->m_rect.min_Rec[0] < branchb->m_rect.min_Rec[0]; });
        int i = 0;
        while (i < all_leaf_num)
        {
            if (MAX_SIZE + i < all_leaf_num)
            {
                sort(position_branchs.begin() + i, position_branchs.begin() + i + MAX_SIZE, [](Branch* brancha, Branch* branchb) {return brancha->m_rect.min_Rec[1] < branchb->m_rect.min_Rec[1]; });
            }
            else
            {
                sort(position_branchs.begin() + i, position_branchs.begin() + all_leaf_num, [](Branch* brancha, Branch* branchb) {return brancha->m_rect.min_Rec[1] < branchb->m_rect.min_Rec[1]; });
            }
            double min0 = 0, min1 = 0, max0 = 0, max1 = 0;
            Node* m_node = new Node;
            int index = i;
            for (index = i; index < i + MAX_SIZE && index < all_leaf_num; index++)
            {
                min0 = min(position_branchs[i]->m_rect.min_Rec[0], min0);
                min1 = min(position_branchs[i]->m_rect.min_Rec[1], min1);
                max0 = max(position_branchs[i]->m_rect.max_Rec[0], max0);
                max1 = max(position_branchs[i]->m_rect.max_Rec[1], max1);
                position_branchs[index]->curNode = m_node;
                m_node->mBranch.push_back(position_branchs[index]);
                m_node->InvertedFileUpdate(position_branchs[index]);
            }
            m_node->m_rect.max_Rec[0] = max0;
            m_node->m_rect.max_Rec[1] = max1;
            m_node->m_rect.min_Rec[0] = min0;
            m_node->m_rect.min_Rec[1] = min1;
            m_node->level = 0;
            m_node->setCount();
            m_node->ID = index / MAX_SIZE;
            nodes.push_back(m_node);
            leaf_position_map.push_back(m_node);
            leaf_node_num++;
            i = index;
        }
        int leaf_node_num = int(nodes.size());
        int temp_num = leaf_node_num;
        int cnt = 0;
        int _level = 1;
        while(temp_num > 1)
        {
            int temp_temp_num = int(nodes.size());

            while(cnt < temp_temp_num)
            {
                Node *m_node = new Node();
                m_node->initRectangle();
                m_node->level = _level; 
                
                for(int i = 0; i < MAX_SIZE; i++)
                {
                    Branch *m_branch = new Branch();
                    m_branch->m_rect = nodes[cnt]->m_rect;
                    for(int j = 0; j < nodes[cnt]->mBranch.size(); j++)
                    {
                        if(!(nodes[cnt]->mBranch[j]->is_virtual))
                        {
                            m_branch->keyWeightUpdate(nodes[cnt]->mBranch[j]);
                            
                            m_branch->textUpdate(nodes[cnt]->mBranch[j]);
                        }
                        m_branch->child.push_back(nodes[cnt]->mBranch[j]);
                        nodes[cnt]->mBranch[j]->partent = m_branch;
                    }
                    m_branch->level = _level;
                    m_branch->ID = temp_ID;
                    m_branch->textID = temp_ID;
                    text_position_map.insert(pair<int, string>(temp_ID, m_branch->text));
                    temp_ID++;
                    position_branchs.push_back(m_branch);
                  
                    m_branch->curNode = m_node;
                    m_node->mBranch.push_back(m_branch);
                    m_node->InvertedFileUpdate(m_branch);
                    m_node->rectUpdate(m_branch);
                    nodes[cnt]->parentNode = m_node;
                    cnt++;
                }
                nodes.push_back(m_node);
            }
            temp_num /= MAX_SIZE;
            _level++;
        }
        tree_deep = _level;
        node_num = int(nodes.size());
        for(int index = leaf_node_num; index < node_num; index++)
        {
            int temp_num = int(nodes[index]->mBranch.size());
            if(temp_num < MAX_SIZE)
            {
                nodes[index]->addVirtualBranch(key,text_position_map,temp_ID,Com);
            }
        }
        root = nodes.back();
        printf("The procsessing of constructing a IRtree is Completed......\n");
    }

    void textPathOram(Branch *curBranch)
    {
        Node* newNode = new Node();
        int rand_num = newNode->randNum(branch_num);
        int rand_num2 = newNode->randNum(leaf_node_num);
        Node* temp_node = leaf_position_map[rand_num2];
        vector<Branch*>text_branchs;
        while(temp_node != NULL)
        {
            for (int i = 0; i < MAX_SIZE; i++)
            {
                text_branchs.push_back(temp_node->mBranch[i]);
                string out_text;
                string in_text = temp_node->mBranch[i]->text;
                PathORAM::aes_decrypt(in_text, key, out_text);
                string cipher;
                PathORAM::aes_encrypt(out_text, key, cipher);
            }
            temp_node = temp_node->parentNode;
        }
        Node* curNode = curBranch->curNode;
        while (curNode != NULL)
        {
            for (int i = 0; i < MAX_SIZE; i++)
            {
                text_branchs.push_back(curNode->mBranch[i]);
                string out_text;
                string in_text = curNode->mBranch[i]->text;
                in_text = string("\0");
                PathORAM::aes_decrypt(in_text, key, out_text);
                string cipher;
                PathORAM::aes_encrypt(out_text, key, cipher);
            }
            curNode = curNode->parentNode;
        }
        string text = text_position_map[rand_num];
        if (text.compare(Com) == 0)
        {
            curBranch->textID = rand_num;
            text_position_map[rand_num] = text_position_map[curBranch->textID];
            text_position_map[curBranch->textID] = Com;
        }
        
    }
    vector<Datatype> Search(Rectangle rect, string &text)
    {
        auto start = chrono::high_resolution_clock::now();
        vector<Datatype> results;
        list<kNNUtil> Queue;
        int k = search_K;
        Branch* search_branch = new Branch();
        textPathOram(search_branch);
        search_branch->CalcuKeyWordRele(text);
        search_branch->m_rect = rect;
        for (int i = 0; i < MAX_SIZE; i++)
        {
            kNNUtil knn_object;
            double _rele = root->CalcuTestSPaceRele(root->mBranch[i], search_branch);
            //textPathOram(root->mBranch[i]);
            knn_object.rele = _rele;
            knn_object.pBranch = root->mBranch[i];
            if (root->mBranch[i]->level == 0)
            {
                knn_object.isleaf = true;
            }
            else
            {
                knn_object.isleaf = false;
            }
            Queue.push_back(knn_object);
        }

        int i = 0;
        for (auto pr = Queue.begin(); i < k && pr != Queue.end();)
        {
            if ((pr->isleaf) == true)
            {
                if (pr == Queue.end())
                {
                    break;
                }
                i++;
                pr++;
                continue;
            }
            else
            {
                Branch* inBranch = pr->pBranch;
                Queue.erase(pr);
                knnInsert(inBranch, Queue, search_branch);
                i = 0;
                pr = Queue.begin();
            }

        }

        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        list<kNNUtil>::iterator pKnn = Queue.begin();
        for (pKnn = Queue.begin(), i = 0; pKnn != Queue.end() && i < search_K; i++)
        {
         
            string out_line; 
            PathORAM::aes_decrypt((*pKnn).pBranch->trueData, key, out_line);
            results.push_back(out_line);
            int flag = 1;
            if (!stash.empty())
            {
                for (auto pr = stash.begin(); pr != stash.end(); pr++)
                {
                    string out_line_2;
                    if ((*pr)->level == 0)
                    {
                        
                        PathORAM::aes_decrypt((*pr)->trueData, key, out_line_2);
                        if (out_line.compare(out_line_2) == 0)
                        {
                            flag = 0;
                            stash.erase(pr);
                            break;
                        }
                        if (flag == 0)
                        {
                            break;
                        }
                    }
                }
            }
            if (flag)
            {
                pathEvcition(pKnn->pBranch);
            }
          
            pKnn++;
        }
        return results;
    }

    void displayResults(vector<Datatype> results)
    {
       
        for (Datatype& text : results)
        {
            cout<< text << ' ';
        }
        cout << endl;

    }
    //
    void knnInsert(Branch* inBranch, list<kNNUtil>& Queue, Branch* search_branch)
    {
        int _size = inBranch->child.size();
        for (int i = 0; i < _size; i++)
        {
            Branch* myBranch = inBranch->child[i];
            double _rele = CalcuTestSPaceRele(myBranch,search_branch);
            kNNUtil myKnn{};
            myKnn.rele = _rele;
            myKnn.pBranch = myBranch;
            if (myBranch->level == 0)
            {
                myKnn.isleaf = true;
            }
            else
            {
                myKnn.isleaf = false;
            }
            int _insert = 0;
            list<kNNUtil>::iterator pInsert = Queue.begin();
            if (Queue.size() > 0)
            {
                while (_rele < (*pInsert).rele)
                {
                    pInsert++;
                    if (pInsert == Queue.end())
                    {
                        break;
                    }
                }
            }
            Queue.emplace(pInsert, myKnn);
        }
    }
    

    void pathRecovery(int ID1, int ID2)
    {
        Node* node1 = leaf_position_map[ID1];
        Node* node2 = leaf_position_map[ID2];
        while (node1 != NULL)
        {
            for (auto pr = stash.begin(); pr != stash.end(); pr++)
            {
                if ((*pr)->curNode == node1)
                {
                    Node* temp_node = (*pr)->curNode;
                    for (int i = 0; i < MAX_SIZE; i++)
                    {
                        Branch* temp_branch = temp_node->mBranch[i];
                        if (temp_branch->pointBranch == NULL)
                        {
                            temp_branch = (*pr);
                            stash.erase(*pr);
                            break;
                        }
                    }
                }
                node1 = node1->parentNode;
            }
        }
        while (node2 != NULL)
        {
            for (auto pr = stash.begin(); pr != stash.end(); pr++)
            {
                if ((*pr)->curNode == node2)
                {
                    Node* temp_node = (*pr)->curNode;
                    for (int i = 0; i < MAX_SIZE; i++)
                    {
                        Branch* temp_branch = temp_node->mBranch[i];
                        if (temp_branch->pointBranch == NULL)
                        {
                            temp_branch = (*pr);
                            stash.erase(*pr);
                        }
                    }
                }
                node2 = node2->parentNode;
            }
        }
    }

    void searchPathBranch(Node* mNode)
    {
        Node* temp_node = mNode;
        while (temp_node != NULL)
        {
            for (int i = 0; i < MAX_SIZE; i++)
            {
                stash.insert(temp_node->mBranch[i]);
                temp_node->mBranch[i]->pointBranch = NULL;
            }
            temp_node = temp_node->parentNode;
        }
    }
    void insertOtherPosition(Branch* mBranch)
    {
        Node* m_node = new Node();
        int rand_num = m_node->randNum(leaf_node_num);
        string str = mBranch->pointBranch->trueData;
        Node* other_node = leaf_position_map[rand_num];
        Node* tempcurNode = mBranch->pointBranch->curNode;
        Branch* tempBranch = mBranch->pointBranch;
        int ID1 = mBranch->curNode->ID;
        int ID2 = other_node->ID;
        searchPathBranch(other_node);
        int flag = 0;
        while (other_node->level < tree_deep)
        {
            for (int i = 0; i < MAX_SIZE; i++)
            {
                if (other_node->mBranch[i]->trueData.compare(Com))
                {
                    other_node->mBranch[i]->trueData = str;
                    mBranch->pointBranch = other_node->mBranch[i];
                    mBranch->pointBranch->trueData = Com;
                    flag = 1;
                    stash.erase(mBranch);
                    break;
                }

            }
            if (flag)
            {
                break;
            }
        }
        pathRecovery(ID1, ID2);
    }
    //
    void pathEvcition(Branch* mBranch)
    {
        temp_stash.insert(mBranch);
        search_num++;
        if (search_num == eviction_max)
        {
            for (auto pr = temp_stash.begin(); pr != temp_stash.end(); pr++)
            {
                Node* temp_node = (*pr)->curNode;
                searchPathBranch(temp_node);
                insertOtherPosition(*pr);
                temp_stash.erase(*pr);
            }
        }
    }

    //
    //
    void branchUpdate(Branch* oBranch, Branch* nBranch)
    {
        nBranch->ID = oBranch->ID;
        nBranch->pointBranch = oBranch->pointBranch;
        nBranch->is_virtual = oBranch->is_virtual;
        nBranch->level = oBranch->level;
        nBranch->m_rect = oBranch->m_rect;
        nBranch->partent = oBranch->partent;
        nBranch->weight = oBranch->weight;
    }
    //

    chrono::nanoseconds getRunTime(double x, double y, string &text)
    {
        Rectangle rect = Rectangle();
        rect.min_Rec[0] = x;
        rect.min_Rec[1] = y;
        rect.max_Rec[0] = x;
        rect.max_Rec[1] = y;
        auto _start = chrono::high_resolution_clock::now();
        Search(rect, text);
        auto _end = chrono::high_resolution_clock::now();
        auto runtime = chrono::duration_cast<chrono::nanoseconds>(_end - _start);
        return runtime;

    }

};

#endif