#pragma once
#include <optional>
#include<vector>
using namespace std;

enum class token_type
{
    _return,
    semi,
    let,
    eq,
    ident,
    _if,
    elif,
    _else,
    number,
    open,
    close,
    open_curly,
    close_curly,
    sub,
    div,
    star,
    plus,
    _while,
    bigger,
    smaller,
    e_bigger,
    e_smaller,
    d_equal,


};

struct token
{
    token_type type;
    optional<string>value;
};

 optional<int>precedence(token_type type)
{
    switch (type)
    {
    case token_type::plus:
    case token_type::sub:
        return 0;
    case token_type::div:
    case token_type::star:
        return 1;
    default:
        return {};
    }
}
class Tokenizer
{
public:
    Tokenizer (const string& scr):
    m_scr(scr)
    {

    }
    vector<token>tokenize()
    {
        vector<token>tokens;
        string buf;
        while (peek().has_value())
        {
            if (isalpha(peek().value()))
            {
                buf.push_back(consume());
                while (peek().has_value() && isalnum(peek().value()))
                {
                    buf.push_back(consume());
                }
                if(buf=="return")
                {
                   tokens.push_back({.type=token_type::_return});
                    buf.clear();
                }
                else if (buf=="if")
                {
                    tokens.push_back({.type=token_type::_if});
                    buf.clear();
                }
                else if (buf=="else")
                {
                    tokens.push_back({.type=token_type::_else});
                    buf.clear();
                }
                else if(buf== "elif")
                {
                    tokens.push_back({.type=token_type::elif});
                    buf.clear();
                }
                else if (buf=="let")
                {
                    tokens.push_back({.type=token_type::let});
                    buf.clear();
                }
                else if (buf=="while")
                {
                    tokens.push_back({.type = token_type::_while});
                    buf.clear();
                }
                else
                {
                    tokens.push_back({.type=token_type::ident , .value=buf});
                    buf.clear();
                }
            }
            else if (isdigit(peek().value()))
            {
                buf.push_back(consume());
                while (peek().has_value() && isdigit(peek().value()))
                {
                    buf.push_back(consume());
                }
                tokens.push_back({.type=token_type::number , .value=buf});
                buf.clear();
            }
            else if (peek().value()=='/' && peek(1).has_value() && peek(1).value()=='/')
            {
                consume();
                consume();
                while (peek().has_value() && peek().value()!='\n')
                {
                    consume();
                }

            }
            else if (peek().value()=='/' && peek(1).has_value() && peek(1).value()=='*')
            {
                consume();
                consume();
                while (!(peek().has_value() && peek().value()=='*' && peek(1).has_value() && peek(1).value()=='/'))
                {
                    consume();
                }
                consume();
                consume();

            }
            else if (peek().value()=='=' && peek(1).has_value() && peek(1).value()=='=')
            {
                consume();
                consume();
                tokens.push_back({.type=token_type::d_equal});
            }
            else if (peek().value()=='<' && peek(1).has_value() && peek(1).value()=='=')
            {
                consume();
                consume();
                tokens.push_back({.type = token_type::e_smaller});
            }
            else if (peek().value()=='>' && peek(1).has_value() && peek(1).value()=='=')
            {
                consume();
                consume();
                tokens.push_back({.type = token_type::e_bigger});
            }
            else if (peek().value()=='=')
            {
                consume();
                tokens.push_back({.type=token_type::eq});
            }
            else if (peek().value()=='-')
            {
                consume();
                tokens.push_back({.type =token_type::sub});
            }
            else if (peek().value()=='*')
            {
                consume();
                tokens.push_back({.type =token_type::star});
            }
            else if (peek().value()=='/')
            {
                consume();
                tokens.push_back({.type =token_type::div});
            }
            else if (peek().value()=='+')
            {
                consume();
                tokens.push_back({.type =token_type::plus});
            }
            else if (peek().value()=='(')
            {
                consume();
                tokens.push_back({.type=token_type::open});
            }
            else if (peek().value()==')')
            {
                consume();
                tokens.push_back({.type=token_type::close});
            }
            else if (peek().value()=='{')
            {
                consume();
                tokens.push_back({.type =token_type::open_curly});
            }
            else if (peek().value()=='}')
            {
                consume();
                tokens.push_back({.type=token_type::close_curly});
            }
            else if (peek().value()==';')
            {
                consume();
                tokens.push_back({.type =token_type::semi});
            }
            else if (peek().value()=='>')
            {
                consume();
                tokens.push_back({.type = token_type::bigger});
            }
            else if (peek().value()=='<')
            {
                consume();
                tokens.push_back({.type = token_type::smaller});
            }

            else if (isspace(peek().value()))
            {
                consume();
            }
            else{
            cerr<<"you missed up"<<endl;
                exit(EXIT_FAILURE);
            }
        }
        return tokens;
    }






private:
    optional<char>peek(int head =0)
    {
        if (m_index+head >=m_scr.length())
        {
            return {};
        }
        else
        {
            return m_scr[m_index+head];
        }
    }

    char consume()
    {
        return m_scr[m_index++];
    }


    string m_scr;
    int m_index=0;

};