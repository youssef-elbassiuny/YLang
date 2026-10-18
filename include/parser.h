#pragma once
#include <variant>
#include <optional>
#include<vector>
#include"tokenization.h"
#include "Arena.h"
using namespace std;
struct node_ident
{
    token ident;
};
struct node_number
{
    token number;
};
struct node_arithmetic_expr;
struct node_grouped_expr
{
    node_arithmetic_expr* expr;
};
struct node_bin_expr_add
{
    node_arithmetic_expr* lhs;
    node_arithmetic_expr* rhs;
};
struct node_bin_expr_mult
{
    node_arithmetic_expr* lhs;
    node_arithmetic_expr* rhs;
};
struct node_bin_expr_div
{
    node_arithmetic_expr* lhs;
    node_arithmetic_expr* rhs;
};
struct node_bin_expr_sub
{
    node_arithmetic_expr* lhs;
    node_arithmetic_expr* rhs;
};
struct node_bin
{
    variant<node_bin_expr_add* , node_bin_expr_div* , node_bin_expr_mult* , node_bin_expr_sub*>var;
};
struct node_term
{
    variant<node_ident* , node_number* , node_grouped_expr*>var;
};
struct node_arithmetic_expr
{
    variant<node_bin* , node_term*>var;
};
struct node_bool_expr
{
    optional<token> op;
    node_arithmetic_expr* lhs;
    optional<node_arithmetic_expr*> rhs;
};
struct node_return
{
    node_arithmetic_expr* expr;
};
struct node_let
{
    node_arithmetic_expr* expr;
    token ident;
};
struct node_st;
struct node_scope
{
    vector<node_st*>stmt;
};
struct node_if_pred;
struct node_if_pred_else_if
{
    node_bool_expr* expr;
    node_scope* scope;
    optional<node_if_pred*>pred;
};
struct node_if_pred_else
{
    node_scope* scope;
};
struct node_if_pred
{
    variant<node_if_pred_else* , node_if_pred_else_if*>var;
};
struct node_if
{
    node_bool_expr* expr;
    node_scope* scope;
    optional<node_if_pred*>pred;
};
struct node_assignment
{
    token ident;
    node_arithmetic_expr* expr;
};
struct node_while
{
    node_bool_expr* expr;
    node_scope* scope;
};
struct node_st
{
    variant<node_let* , node_assignment* ,  node_if* , node_scope* , node_return* , node_while*>var;
};
struct node_prog
{
    vector<node_st*>stmt;
};
class Parser
{
public:
    Parser(vector<token>tokens)
        :m_tokens(std::move(tokens)),  m_allocator(1024*1024*4)
    {

    }
    optional<node_term*>parse_term()
    {
        if (peek().has_value() && peek().value().type==token_type::number)
        {
            node_number* n_n=m_allocator.alloc<node_number>();
            n_n->number=consume();

            node_term* term=m_allocator.alloc<node_term>();
            term->var=n_n;
            return term;
        }
        else if (peek().has_value() && peek().value().type==token_type::ident)
        {
            node_ident* n_i=m_allocator.alloc<node_ident>();
            n_i->ident=consume();

            node_term* term=m_allocator.alloc<node_term>();
            term->var=n_i;
            return term;
        }
        else if (peek().has_value() && peek().value().type==token_type::open)
        {
            consume();
            auto expr=parse_arithmetic_expr();
            if (!expr.has_value())
            {
                cerr<<"expected expression"<<endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type==token_type::close)
            {
                consume();
            }
            else
            {
                cerr<<"expected ')'"<<endl;
                exit(EXIT_FAILURE);
            }
            node_grouped_expr* n_g_e =m_allocator.alloc<node_grouped_expr>();
            n_g_e->expr=expr.value();

            node_term* term=m_allocator.alloc<node_term>();
            term->var=n_g_e;
            return term;
        }
        else
        {
            return{};
        }
    }

    optional<node_bool_expr*>parse_bool_expr()
    {
        node_bool_expr* exp=m_allocator.alloc<node_bool_expr>();
        auto lhs_arithmetic_exp=parse_arithmetic_expr();
        if (!lhs_arithmetic_exp)
        {
            return{};
        }
        exp->lhs=lhs_arithmetic_exp.value();
        if (peek().has_value() && (peek().value().type==token_type::bigger
            || peek().value().type==token_type::smaller
            || peek().value().type==token_type::e_bigger
            || peek().value().type==token_type::e_smaller
            ||peek().value().type==token_type::d_equal))
        {
            exp->op=consume();
            auto rhs_arithmetic_exp=parse_arithmetic_expr();
            if (!rhs_arithmetic_exp)
            {
                return{};
            }
            exp->rhs=rhs_arithmetic_exp.value();
        }

        return exp;

    }


    optional<node_arithmetic_expr*>parse_arithmetic_expr(int min_precedence =0)
    {
        auto lhs_term=parse_term();
        if (!lhs_term)
        {
            return {};
        }
        node_arithmetic_expr* lhs=m_allocator.alloc<node_arithmetic_expr>();
        lhs->var=lhs_term.value();

        while (true)
        {
            auto current=peek();
            if (!current)
            {
                break;
            }
            auto precedence_value=precedence(current->type);
            if (!precedence_value || precedence_value<min_precedence)
            {
                break;
            }
            token op=consume();
            auto rhs=parse_arithmetic_expr(precedence_value.value()+1);
            if (!rhs)
            {
                cerr<<"expected expression aftert operator"<<endl;
                exit(EXIT_FAILURE);
            }
            node_bin* bin=m_allocator.alloc<node_bin>();
            node_arithmetic_expr* new_expr=m_allocator.alloc<node_arithmetic_expr>();

            switch (op.type)
            {
                case token_type::plus:
                    {
                        node_bin_expr_add* n =m_allocator.alloc<node_bin_expr_add>();
                        n->lhs=lhs;
                        n->rhs=rhs.value();
                        bin->var=n;
                        break;
                    }
                case token_type::sub:
                    {
                        node_bin_expr_sub* n =m_allocator.alloc<node_bin_expr_sub>();
                        n->lhs=lhs;
                        n->rhs=rhs.value();
                        bin->var=n;
                        break;
                    }
                case token_type::div:
                    {
                        node_bin_expr_div* n=m_allocator.alloc<node_bin_expr_div>();
                        n->lhs=lhs;
                        n->rhs=rhs.value();
                        bin->var=n;
                        break;
                    }
                case token_type::star:
                    {
                        node_bin_expr_mult* n=m_allocator.alloc<node_bin_expr_mult>();
                        n->lhs=lhs;
                        n->rhs=rhs.value();
                        bin->var=n;
                        break;
                    }
                default:
                    cerr<<"unexpected operator"<<endl;
                    exit(EXIT_FAILURE);

            }
            new_expr->var=bin;
            lhs=new_expr;
        }
        return lhs;
    }
    optional<node_scope*>parse_scope()
    {
        if (!peek().has_value() || peek().value().type!=token_type::open_curly)
        {
            return {};
        }
        consume();
        node_scope* scope=m_allocator.alloc<node_scope>();
        while (auto st=parse_st())
        {
            scope->stmt.push_back(st.value());
        }
        if (peek().has_value() && peek().value().type==token_type::close_curly)
        {
            consume();
        }
        else
        {
            cerr<<"expected '}'"<<endl;
            exit(EXIT_FAILURE);
        }
        return scope;
    }
    optional<node_if_pred*>parse_if_pred()
    {
        if (peek().has_value() && peek().value().type==token_type::elif)
        {
            consume();
            if (peek().has_value() && peek().value().type==token_type::open)
            {
                consume();
            }
            else
            {
                cerr<<"expected '('"<<endl;
                exit(EXIT_FAILURE);
            }
            node_if_pred_else_if* n_i_f=m_allocator.alloc<node_if_pred_else_if>();
            if (auto exp =parse_bool_expr())
            {
                n_i_f->expr=exp.value();
            }
            else
            {
                cerr<<"expected expression"<<endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type==token_type::close)
            {
                consume();
            }
            else
            {
                cerr<<"expected ')'"<<endl;
                exit(EXIT_FAILURE);
            }
            if (auto scope =parse_scope())
            {
                n_i_f->scope=scope.value();
            }
            else
            {
                cerr<<"expected scope"<<endl;
                exit(EXIT_FAILURE);
            }
            n_i_f->pred=parse_if_pred();
            node_if_pred* pred=m_allocator.alloc<node_if_pred>();
            pred->var=n_i_f;
            return pred;
        }
        else if (peek().has_value() && peek().value().type==token_type::_else)
        {
            consume();
            node_if_pred_else* _else=m_allocator.alloc<node_if_pred_else>();
            if (auto scope=parse_scope())
            {
                _else->scope=scope.value();
            }
            else
            {
                cerr<<"expected scope"<<endl;
                exit(EXIT_FAILURE);
            }
            node_if_pred* pred=m_allocator.alloc<node_if_pred>();
            pred->var=_else;
            return pred;

        }
        else
        {
            return{};
        }
    }
    optional<node_st*>parse_st()
    {
        if (peek().has_value() && peek().value().type==token_type::_return)
        {
            consume();
            node_return* _return =m_allocator.alloc<node_return>();
            if (auto exp =parse_arithmetic_expr())
            {
                _return->expr=exp.value();
            }
            else
            {
                cerr<<"expected expression"<<endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type==token_type::semi)
            {
                consume();
            }
            else
            {
                cerr<<"expected ';'"<<endl;
                exit(EXIT_FAILURE);
            }
            node_st* st=m_allocator.alloc<node_st>();
            st->var=_return;
            return st;
        }
        else if(peek().has_value() && peek().value().type==token_type::let
            && peek(1).has_value() && peek(1).value().type==token_type::ident
            && peek(2).has_value() && peek(2).value().type==token_type::eq)
        {
            consume();
            node_let* let=m_allocator.alloc<node_let>();
            let->ident=consume();
            consume();
            if (auto exp =parse_arithmetic_expr())
            {
                let->expr=exp.value();
            }
            else
            {
                cerr<<"expected expression"<<endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type==token_type::semi)
            {
                consume();
            }
            else
            {
                cerr<<"expected ';'";
                exit(EXIT_FAILURE);
            }
            node_st* st=m_allocator.alloc<node_st>();
            st->var=let;
            return st;
        }
        else if (peek().has_value() && peek().value().type==token_type::ident
            && peek(1).has_value() && peek(1).value().type==token_type::eq)
        {
            node_assignment* assignment=m_allocator.alloc<node_assignment>();
            assignment->ident=consume();
            consume();
            if (auto exp =parse_arithmetic_expr())
            {
                assignment->expr=exp.value();
            }
            else
            {
                cerr<<"expected expression"<<endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type==token_type::semi)
            {
                consume();
            }
            else
            {
                cerr<<"expected ';'"<<endl;
                exit(EXIT_FAILURE);
            }
            node_st* st =m_allocator.alloc<node_st>();
            st->var=assignment;
            return st;
        }
        else if (peek().has_value() && peek().value().type==token_type::open_curly)
        {
            node_st* st=m_allocator.alloc<node_st>();
            auto scope=parse_scope();
            if (scope)
            {
                st->var=scope.value();
            }
            else
            {
                cerr<<"invalid scope"<<endl;
                exit(EXIT_FAILURE);
            }
            return st;

        }
        else if (peek().has_value() && peek().value().type==token_type::_if)
        {
            consume();
            if (peek().has_value() && peek().value().type==token_type::open)
            {
                consume();
            }
            else
            {
                cerr<<"expected '('"<<endl;
                exit(EXIT_FAILURE);
            }
            node_if* _if=m_allocator.alloc<node_if>();
            if (auto exp=parse_bool_expr())
            {
                _if->expr=exp.value();
            }
            else
            {
                cerr<<"expected exoression"<<endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type==token_type::close)
            {
                consume();
            }
            else
            {
                cerr<<"expected ')'"<<endl;
                exit (EXIT_FAILURE);
            }
            auto scope =parse_scope();
            if (scope)
            {
                _if->scope=scope.value();
            }
            else
            {
                cerr<<"invalid scope"<<endl;
                exit(EXIT_FAILURE);
            }
            _if->pred=parse_if_pred();
            node_st* st=m_allocator.alloc<node_st>();
            st->var=_if;
            return st;
        }
        else if (peek().has_value() && peek().value().type==token_type::_while)
        {
            consume();
            if (peek().has_value() && peek().value().type==token_type::open)
            {
                consume();
            }
            else
            {
                cerr<<"expected '('"<<endl;
                exit(EXIT_FAILURE);
            }
            node_while* _while=m_allocator.alloc<node_while>();
            if (auto expr =parse_bool_expr())
            {
                _while->expr=expr.value();
            }
            else
            {
                cerr<<"invalid expr"<<endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type==token_type::close)
            {
                consume();
            }
            else
            {
                cerr<<"expexted ')'"<<endl;
                exit(EXIT_FAILURE);
            }
            if (auto scope =parse_scope())
            {
                _while->scope=scope.value();
            }
            else
            {
                cerr<<"expected scope"<<endl;
                exit(EXIT_FAILURE);
            }
            node_st* st =m_allocator.alloc<node_st>();
            st->var=_while;
            return st;

        }
        else
        {
            return {};
        }
    }
    optional<node_prog*>parse_prog()
    {
        node_prog* prog=m_allocator.alloc<node_prog>();
        while (peek().has_value())
        {
            auto st = parse_st();
            if (st)
            {
                prog->stmt.push_back(st.value());
            }
            else
            {
                cerr<<"invalid statment"<<endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }
private:

    optional<token>peek(size_t head=0)
    {
        if (m_index+head>= m_tokens.size())
        {
            return{};
        }
        else
        {
            return m_tokens[m_index+head];
        }
    }

    token consume()
    {
        return m_tokens[m_index++];
    }

    vector<token>m_tokens;
    Arena_allocator m_allocator;
    size_t m_index=0;

};


