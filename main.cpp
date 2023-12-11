#include "bits/stdc++.h"

using namespace std;

//convert binary to decimal
int binaryToDecimal(string n)
{
    // Initializing base value to 1, i.e 2^0
    int sum = 0;

    int p = 0;
    for(int i = 7; i >= 0; i--)
    {
        sum += ((int)n[i] - (int)'0') * pow(2, p);
        p++;
    }

    return sum;
}

//convert decimal to binary
string decimalToBinary(int n)
{
    // Initializing base value to 1, i.e 2^0
    string sum = "";

    for(int i = 0; i < 8; i++)
    {
        sum += to_string(n%2);
        n /= 2;
    }

    return sum;
}

//write binary byte to a file
void write_to_binary(string byte, ofstream &fp) {
    int8_t b = binaryToDecimal(byte);
    fp.write(reinterpret_cast<const char *>(&b), sizeof(b));
}

//get the probability table
unordered_map <char, float> getProb(string path)
{
    unordered_map <char, float> prob;

    ifstream in(path+".txt");
    if (!in.is_open())
    {
        cout<< "Error loading the text file. \n";
    }

    char ch;
    int n=0;
    while (in.get(ch)) 
    {
        prob[ch]++;
        n++;
    }
    for(auto &it:prob)
    {
        it.second /= n;
    }

    in.close();

    return prob;
}

//data structure for a node in huffman tree
struct hNode
{
    string code;
    string data;
    float prob;
    bool leaf;
    hNode *right;
    hNode *left;
    hNode()
    {
        code = "";
        left = right = nullptr;
        data = "";
        prob = 0;
        leaf = false;
    }
    hNode(string d, float p)
    {
        code = "";
        left = right = nullptr;
        data = d;
        prob = p;
        leaf = false;
    }
};

//for comaring two nodes in the priority queue with their probabilities
struct node_cmp
{
   bool operator()( const hNode a, const hNode b ) const 
   {
    return a.prob > b.prob;
   }
};

//get the codes from the tree
void get_codes(hNode* node, char x, string prevCode, unordered_map <char, string> &codes)
{

    if (node == NULL)
        return;

    if (x == 'r')
        node->code = prevCode + '1';
    else if (x=='l')
        node->code = prevCode + '0';


    get_codes(node->left, 'l', node->code, codes);

    if (node->leaf == true)
    {
        codes[node->data[0]] = node->code;
    }

    get_codes(node->right, 'r', node -> code, codes);

}

//encode the input file to binary file
void encoder(unordered_map <char, string> codes, string path)
{
    ifstream in(path+".txt");

    ofstream fp;
    fp.open(path+"Compressed.bin",ios::out | ios :: binary | ios::app);

    if (!in.is_open())
    {
        cout<< "Error loading the text file. \n";
    }
    char ch;
    string s = "";
    while (in.get(ch)) 
    {
        for(int i = 0; i < codes[ch].length(); i++)
        {
            s += codes[ch][i];
            if(s.length() == 8)
            {
                write_to_binary(s, fp);
                s = "";
            }
        }
    }

    if(s.length() > 0)
    {
        int i = 0;
        while(s.length() < 8)
        {
            if(i == codes[' '].size())
            {
                i = 0;
            }
            s += codes[' '][i];
            i++;
        }
        write_to_binary(s, fp);
    }


    in.close();
    fp.close();
}

//decode the binary file into a text file
void decoder(unordered_map<string, char> V, string path, string outpath)
{
    string p = path + "Compressed.bin";
    ifstream in(p);
    ofstream out(outpath + ".txt");
    if (!in.is_open())
    {
        cout<< "Error loading the text file. \n";
    }

    int num;
    char c;
    string code = "";
    while (in.read((char*)&num, 1)) 
    {
        string byte = decimalToBinary(num);
        
        for(int i = 7; i >= 0; i--)
        {
            code += byte[i];
            if(V.find(code) != V.end())
            {
                out << V[code];
                code = "";
            }
        }
    }
    in.close();
    out.close();
}

//caculate efficiency
vector<float> calculate_efficiency(unordered_map <char, float> prob, unordered_map <char, string> codes)
{
    float H = 0;
    float L = 0;
    for(auto&it:prob)
    {
        H += it.second*log2(1/it.second);
        L += it.second*codes[it.first].size();
    }
    float eff = H/L;
    float cmr = L/8;

    return {eff, cmr};
}

//read codes from a file
void read_codes(unordered_map <string, char> &chars, string path)
{
    ifstream in(path+"Table.txt");
    string line;
    while (getline(in, line))
    {
        istringstream iss(line);
        string p;
        int i = 1;
        char data;
        while(getline(iss, p, '\t'))
        {
            switch (i)
            {
            case 1:
                data = p[0];
                i++;
                break;
            case 2:
                chars[p] = data;
                i++;
                break;
            default:
                break;
            }
            
        }
    }
}

//creation of huffman tree & extract the codes (i.e. encode an decode included)
void huffmanCoding(string path, string outpath, char type)
{
    if (type == 'c')
    {
        unordered_map <char, float> prob = getProb(path);
        priority_queue<hNode, vector<hNode>, node_cmp> huffmanTree;

        for(auto &it: prob)
        {
            hNode tmp(string(1, it.first), it.second);
            tmp.leaf = true;
            huffmanTree.push(tmp);
        }

        priority_queue<hNode, vector<hNode>, node_cmp> g = huffmanTree;
        while(g.size() > 1)
        {
            hNode tmp;
            string s = "";
            float cost = 0;
            hNode *left = new hNode;
            *left = g.top();
            tmp.left = left;
            s += g.top().data;
            cost += g.top().prob;
            g.pop();
            hNode *right = new hNode;
            *right = g.top();
            tmp.right = right;
            s += g.top().data;
            cost += g.top().prob;
            g.pop();
            tmp.data = s;
            tmp.prob = cost;
            g.push(tmp);
        }

        hNode root;
        root = g.top();

        unordered_map <char, string> codes;
        get_codes(&root, 'x', "", codes);

        ofstream table(path+"Table.txt");
        for(auto&it:codes)
        {
            table << it.first << "\t" << it.second << endl;
        }
        table.close();

        encoder(codes, path);
        vector<float> results = calculate_efficiency(prob, codes);
        cout << "Efficiency: " << results[0] << endl;
        cout << "Compression ratio: " << results[1] << endl;
    }
    else
    {
        unordered_map <string, char> chars;
        read_codes(chars, path);
        decoder(chars, path, outpath);
    }
}


int main()
{
    int choice;
    cout << "Welcome to Our Huffman Text Compressor: \n";
    main:
    cout << "----------------------------------------\n";
    cout << "1. Compress File\n";
    cout << "2. Decompress File\n";
    cout << "3. Exit\n";
    cout << "----------------------------------------\n";
    cout << "choice: ";
    cin >> choice;

    string name;
    string out;
    switch (choice)
    {
    case 1:
        cout << "----------------------------------------\n";
        cout << "Compression System: \n";
        cout << "----------------------------------------\n";
        cout << "Enter the name of the file: ";
        cin >> name;
        cout << "----------------------------------------\n";
        try
        {
            huffmanCoding(name, "", 'c');
        }
        catch(const std::exception& e)
        {
            cout << "Error: " << e.what() << endl;
        }
        goto main;
        break;
    case 2:
        cout << "----------------------------------------\n";
        cout << "Decompression System: \n";
        cout << "----------------------------------------\n";
        cout << "Enter the name of the file: ";
        cin >> name;
        cout << "Enter the name of the output file: ";
        cin >> out;
        cout << "----------------------------------------\n";
        try
        {
            huffmanCoding(name, out, 'd');
        }
        catch(const std::exception& e)
        {
            cout << "Error: " << e.what() << endl;
        }
        goto main;
        break;
    case 3:
        return 0;
    default:
        break;
    }
}