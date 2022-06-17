#include <vector>
#include <string>
#include <math.h>  
#include <queue>
#include <functional>
#include <iostream>
#include <algorithm>
#include <queue>
#include <map>
#include <cmath>
#include <stdio.h>

using namespace std;


struct MinHeapNode {

	string data;

	int freq;

	MinHeapNode *left, *right;

	MinHeapNode(string data, unsigned freq)

	{

		left = right = NULL;
		this->data = data;
		this->freq = freq;
	}
};


struct QMDD_Node {

int var;
int p1, p2, p3, p4;
bool p1flag, p2flag;
std::multimap<int, int> parent;

QMDD_Node(int a, int b, int c, int d, int e, bool a1, bool a2)

	{

		var	= a;
		
		p1	= b;
		p2	= c;
		p3 	= d;
		p4 	= e;

		p1flag	=	a1;
		p2flag	= 	a2;



	}

QMDD_Node(const QMDD_Node& a)

	{

		var	= a.var;
		
		this->p1	= a.p1;
		this->p2	= a.p2;
		this->p3	= a.p3;
		this->p4	= a.p4;
		
		this->p1flag 	=	a.p1flag;
		this->p2flag 	=	a.p2flag;

		this->parent	=	a.parent;
	

	}

QMDD_Node(int a)

	{
		
		var	= a ;
		
		p1	= -1;
	
		p2	= -1;

		p3	= -1;
	
		p4	= -1;
		

		p1flag	=	false;
		p2flag	=	false;

	}

};

std::map<QMDD_Node, int> hash_map;

vector<QMDD_Node> QMDD_array;

bool operator<(const QMDD_Node& fk, const QMDD_Node& lk) {

	if (fk.var < lk.var) { return true;} 
	if (fk.var > lk.var) { return false;} 
	
	if (fk.p1 <lk.p1) {return true; }
	if (fk.p1 >lk.p1) {return false; }
	
	if (fk.p2 < lk.p2) {return true; }
	if (fk.p2 > lk.p2) {return false; }
	
	if (fk.p3 <lk.p3) {return true; }
	if (fk.p3 >lk.p3) {return false; }

	if (fk.p4 <lk.p4) {return true; }
	if (fk.p4 >lk.p4) {return false; }

	if (fk.p1flag <lk.p1flag) {return true; }
	if (fk.p1flag >lk.p1flag) {return false; }


	return (fk.p2flag < lk.p2flag);



}


bool operator==(QMDD_Node& fk, const QMDD_Node& lk) { 
	if (((fk.var) == (lk.var)) && ((fk.p1) == (lk.p1)) && ((fk.p2) == (lk.p2)) && ((fk.p3) == (lk.p3)) && ((fk.p4) == (lk.p4)) && ((fk.p1flag) == (lk.p1flag)) && ((fk.p2flag) == (lk.p2flag)))
	{
	return true;
    }
	else{
    return false;
    }
	
	}

struct compare {

	bool operator()(MinHeapNode* l, MinHeapNode* r)

	{
		return (l->freq > r->freq);
	}
};


void hufCodes(struct MinHeapNode* root, string str, std::map<string, string>& check_map)
{

	if (!root)
		return;

	if (root->data != "$"){
        check_map.insert({root->data , str});
    }

    
	hufCodes(root->left, str + "0", check_map);
	hufCodes(root->right, str + "1", check_map);
}


std::map<string, string> HuffmanCodes(vector<string> output)
{

    std::map<std::string, int> output_freq;

    std::map<string, string> output_enc;


    for (auto & elem : output)
    {
        auto result = output_freq.insert(std::pair<std::string, int>(elem, 1));
        if (result.second == false)
            result.first->second++;
    }


    for (auto & elem : output_freq)
    {
        output_freq[elem.first]    =   ceil(log2(elem.second));
    }


	struct MinHeapNode *left, *right, *top;

	
	priority_queue<MinHeapNode*, vector<MinHeapNode*>, compare> minHeap;

	for (auto & elem : output_freq)
		minHeap.push(new MinHeapNode(elem.first, elem.second));


	while (minHeap.size() != 1) {

		
		left = minHeap.top();
		minHeap.pop();

		right = minHeap.top();
		minHeap.pop();

		top = new MinHeapNode("$", std::max(left->freq, right->freq) + 1);

		top->left = left;
		top->right = right;

		minHeap.push(top);
	}

	hufCodes(minHeap.top(), "", output_enc);

    return output_enc;
}

int find_qmdd_array_index(vector<QMDD_Node>& QMDD_array, QMDD_Node& node){

		auto it = find(QMDD_array.begin(), QMDD_array.end(), node);
		
        int index = it - QMDD_array.begin();
	
		return index;
	}


int findOrCreateNode (int v, int p1, int p2, int p3, int p4, bool p1flag, bool p2flag)
{

    QMDD_Node new_node(v,p1,p2,p3,p4,p1flag,p2flag);

		auto search = hash_map.find(new_node);

		if (search!=hash_map.end()){

			return search->second;
		}

		else{

		QMDD_array.push_back(new_node);
		
		hash_map.insert(std::make_pair(new_node,find_qmdd_array_index(QMDD_array, new_node)));
	
		return find_qmdd_array_index(QMDD_array, new_node);

		}
		

}

int buildQMDD(vector<string>& dd_combination, int var){

	if(dd_combination.empty())
	{
		return 0;
	}

	vector<string> p1_vector;

	vector<string> p2_vector;

	vector<string> p3_vector;

	vector<string> p4_vector;


	string safe;

	int zero = 0;

	int one = 1;

	int p1,p2,p3,p4;

	bool p1flag = false; 
	bool p2flag = false;

	int f1 = 0; 
	int f2 = 0;
	int f3 = 0;
	int f4 = 0;

	bool f1flag = false; 
	bool f2flag = false;
    
    int label = 100 + var;

	if (dd_combination[0].size() == 2){

		for (int i =0; i < dd_combination.size(); i++) {

		if (dd_combination[i][0] == '0' && dd_combination[i][1] == '0')  {

		    f1 = 1;

		}

		else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '0')  {

			f2 = 1;
		}

		else if (dd_combination[i][0] == '0' && dd_combination[i][1] == '1')  {

			f3 = 1;

		}

		else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '1')  {

			f4 = 1;
		}

		else if (dd_combination[i][0] == '0' && dd_combination[i][1] == '_')  {

			f1 = 1;

			f1flag = true;

		}

		else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '_')  {

			f2 = 1;

			f2flag = true;
		}

		

	}

	return findOrCreateNode(label, f1, f2, f3, f4, f1flag, f2flag);

	}

	else{
	for (int i = 0; i < dd_combination.size(); i++) {

		if (dd_combination[i][0] == '0' && dd_combination[i][1] == '0'){

			safe = (dd_combination[i]).erase(0,2);

			p1_vector.push_back(safe);
			
			}

		else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '0'){

			safe = (dd_combination[i]).erase(0,2);

			p2_vector.push_back(safe);
			
			}

		else if (dd_combination[i][0] == '0' && dd_combination[i][1] == '1'){

			safe = (dd_combination[i]).erase(0,2);

			p3_vector.push_back(safe);
			
			}

		else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '1'){

			safe = (dd_combination[i]).erase(0,2);

			p4_vector.push_back(safe);
			
			}

		else if (dd_combination[i][0] == '0' && dd_combination[i][1] == '_'){

			safe = (dd_combination[i]).erase(0,2);

			p1_vector.push_back(safe);

			p1flag = true;
			
			}

		else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '_'){

			safe = (dd_combination[i]).erase(0,2);

			p2_vector.push_back(safe);

			p2flag = true;
			
			}
		
	}
	
	p1 = buildQMDD(p1_vector,var+1);

	p2 = buildQMDD(p2_vector,var+1);

	p3 = buildQMDD(p3_vector,var+1);

	p4 = buildQMDD(p4_vector,var+1);

	return findOrCreateNode(label, p1, p2, p3, p4, p1flag, p2flag);
	}


}

vector<string> ioCombination (vector<string>& input_combination, vector<string>& output_combination, std::map<string, string>& enc_output){

		vector<string> dd_combination;

		int bitwidth =   input_combination[0].size();

    	for (auto & elem : enc_output)
    	{
		int dont_care	=	bitwidth -  elem.second.size();
        
        if (dont_care){

			for (int i = 0; i < dont_care; i++){
				elem.second.push_back('_');
			}

    	}

		}

		for (int i = 0; i < output_combination.size(); i++){

		output_combination[i]	=	enc_output[output_combination[i]];

		}

		for (int i = 0; i < output_combination.size(); i++){

		string io;


		for (int j = 0; j < input_combination[0].size(); j++){

			io += input_combination[i][j];
			io += output_combination[i][j];

		}

		dd_combination.push_back(io);

		}

		return dd_combination;


	}

int main()
{

vector<string> output_combination { "010", "010", "100", "100", "011","010", "010", "001"}; 

vector<string> input_combination { "000", "001", "010", "011", "100","101", "110", "111"}; 

vector<string> dd_combination; 

cout<<"-------------------------------------"<<endl;

for (int i = 0; i < output_combination.size(); i++){

    cout<<input_combination[i]<<":"<<output_combination[i]<<endl;

}
cout<<"-------------------------------------"<<endl;
    
std::map<string, string> enc_output;

enc_output  =   HuffmanCodes(output_combination);

for (auto &x : enc_output)
    cout<<x.first<<":"<<x.second<<endl;



dd_combination = ioCombination(input_combination, output_combination, enc_output);
	

for (int i = 0; i < output_combination.size(); i++){

    cout<<input_combination[i]<<":"<<output_combination[i]<<endl;

}
	
	
cout<<"-------------------------------------"<<endl;

for (int i = 0; i < dd_combination.size(); i++){

cout<<dd_combination[i]<<endl;}

cout<<"-------------------------------------"<<endl;

vector<string> string_vector;

// tt representing CNOT opertion

string_vector.push_back("0000");
string_vector.push_back("0011");
string_vector.push_back("1101");
string_vector.push_back("1110");


QMDD_Node zero_node(0);

QMDD_Node one_node(1);

hash_map.insert(std::make_pair(zero_node, 0));

hash_map.insert(std::make_pair(one_node, 1));

QMDD_array.push_back(zero_node);
QMDD_array.push_back(one_node);

int zero = 0;

int root_index = buildQMDD(string_vector, zero);   //QMDD for CNOT TT.

// int root_index = buildQMDD(dd_combination, zero);   //QMDD for the input/output combination specified above.   

//QMDD_array (the root node is present in the last index)

auto s = QMDD_array.begin();

cout<<"-------------------------------------"<<endl;

for(QMDD_Node& a : QMDD_array){
	cout<<a.var<<endl;
	cout<<a.p1<<endl;
	cout<<a.p2<<endl;
	cout<<a.p3<<endl;
	cout<<a.p4<<endl;
	cout<<a.p1flag<<endl;
	cout<<a.p2flag<<endl;
	
}

cout<<"-------------------------------------"<<endl;

return 0;

}
