#include<iostream>
#include<fstream>
#include<sstream>
#include "tokenization.h"
#include "parser.h"
#include "generator.h"
using namespace std;

int main(int argc , char *argv[])
{
    if (argc!=2)
    {
        cerr<<"incorrect usege";
        return EXIT_FAILURE;
    }
    cout<<argv[1]<<endl;
    string content;
    {
        stringstream content_stream;
        fstream input(argv[1] , ios::in);
        content_stream <<input.rdbuf();
        content = content_stream.str();
    }
    Tokenizer tokenizer(content);
    pair<vector<token> , size_t>v= tokenizer.tokenize();
    // for (const auto& i: v.first)
    // {
    //     cout<<int(i.type)<<endl;
    // }
    Parser parser(v.first , v.second);
    optional<node_prog*>AST=parser.parse_prog();
    Generator generator(AST.value());
    {
        generator.gen();
    }
}
