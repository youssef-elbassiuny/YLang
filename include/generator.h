#pragma once
#include<llvm/IR/LLVMContext.h>
#include<llvm/IR/Module.h>
#include<llvm/IR/Function.h>
#include<llvm/IR/BasicBlock.h>
#include<llvm/IR/Type.h>
#include<llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include<map>
#include "parser.h"
#include <string>
class Generator
{
public:
    Generator(node_prog* root)
        :m_root(root),
    context(std::make_unique<llvm::LLVMContext>()),
    module(std::make_unique<llvm::Module>("CodeGeneration" ,*context)),
    builder(std::make_unique<llvm::IRBuilder<>>(*context))
    {
    }



    bool gen_scope(node_scope* scope)
    {
        scope_begin();
        bool reachable = true;
        for (node_st* st :scope->stmt)
        {
            if (!reachable)
            {
                break;
            }

            reachable = gen_st(st);
        }
        scope_end();
        return reachable;
    }
    llvm::Value* gen_bin(node_bin* bin)
    {
        if (holds_alternative<node_bin_expr_add*>(bin->var))
        {
            node_bin_expr_add* add=get<node_bin_expr_add*>(bin->var);
            llvm::Value* a=gen_arithmetic_exp(add->lhs);
            llvm::Value* b=gen_arithmetic_exp(add->rhs);
            llvm::Value *result=builder->CreateAdd(a, b, "result");
            return result;
        }
        else if (holds_alternative<node_bin_expr_sub*>(bin->var))
        {
            node_bin_expr_sub* sub=get<node_bin_expr_sub*>(bin->var);
            llvm::Value* a=gen_arithmetic_exp(sub->lhs);
            llvm::Value* b=gen_arithmetic_exp(sub->rhs);
            llvm::Value *result=builder->CreateSub(a, b, "result");
            return result;
        }
        else if (holds_alternative<node_bin_expr_mult*>(bin->var))
        {
            node_bin_expr_mult* mult=get<node_bin_expr_mult*>(bin->var);
            llvm::Value* a=gen_arithmetic_exp(mult->lhs);
            llvm::Value* b=gen_arithmetic_exp(mult->rhs);
            llvm::Value *result=builder->CreateMul(a, b, "result");
            return result;

        }
        else if (holds_alternative<node_bin_expr_div*>(bin->var))
        {
            node_bin_expr_div* div=get<node_bin_expr_div*>(bin->var);
            llvm::Value* a=gen_arithmetic_exp(div->lhs);
            llvm::Value* b=gen_arithmetic_exp(div->rhs);
            llvm::Value *result=builder->CreateSDiv(a, b, "result");
            return result;

        }
        return nullptr;
    }
    llvm::Value* gen_term(node_term* term)
    {
        if (holds_alternative<node_call_fun*>(term->var))
        {
            std::vector<llvm::Value*>params;
            node_call_fun* call_fun=get<node_call_fun*>(term->var);
            for (int i=0 ; i<call_fun->param.size() ;++i)
            {
                params.push_back(gen_arithmetic_exp(call_fun->param[i]));
            }
            std::size_t code=0;
            for (int i=0 ; i<m_fun_code.size() ;++i)
            {
                if (m_fun_code[i].first==call_fun->ident.value.value())
                {
                    code=m_fun_code[i].second;
                }
            }
            llvm::Value* call=builder->CreateCall( m_fun[code] , params , "call");
            return call;
        }
        else if (holds_alternative<node_ident*>(term->var))
        {
            node_ident* ident=get<node_ident*>(term->var);
            auto it =m_variable.find(ident->ident.value.value());
            if (it==m_variable.end())
            {
                cerr<<"undeclared identifier"<<" "<<ident->ident.value.value()<<endl;
                exit(EXIT_FAILURE);
            }
            llvm::Type *Int=llvm::Type::getInt32Ty(*context);
            return builder->CreateLoad(Int, it->second.value, "load_value");
        }
        else if(holds_alternative<node_number*>(term->var))
        {
            node_number* number=get<node_number*>(term->var);
            llvm::Type* Int = llvm::Type::getInt32Ty(*context);
            int number_value = std::stoi(number->number.value.value());
            llvm::Value* value =
                llvm::ConstantInt::get(Int, number_value);
            return value;

        }
        else if (holds_alternative<node_grouped_expr*>(term->var))
        {
            node_grouped_expr* grouped =get<node_grouped_expr*>(term->var);
            return gen_arithmetic_exp(grouped->expr);
        }


        return nullptr;
    }

    llvm::Value* gen_bool_exp(node_bool_expr* exp)
    {
        llvm::Value* lhs=gen_arithmetic_exp(exp->lhs);
        if (exp->op.has_value())
        {
            llvm::Value* rhs=gen_arithmetic_exp(exp->rhs.value());
            if (exp->op.value().type==token_type::smaller)
            {
                llvm::Value* result=builder->CreateICmpSLT(lhs , rhs , "cmp");
                return result;
            }
            else if (exp->op.value().type==token_type::bigger)
            {
                llvm::Value* result=builder->CreateICmpSGT(lhs , rhs , "cmp");
                return result;
            }
            else if (exp->op.value().type==token_type::d_equal)
            {
                llvm::Value* result=builder->CreateICmpEQ(lhs , rhs , "cmp");
                return result;
            }
            else if (exp->op.value().type==token_type::e_bigger)
            {
                llvm::Value* result=builder->CreateICmpSGE(lhs, rhs , "cmp");
                return result;
            }
            else if (exp->op.value().type==token_type::e_smaller)
            {
                llvm::Value* result =builder->CreateICmpSLE(lhs , rhs , "cmp");
                return result;
            }
        }
        else
        {
            llvm::Value* result=builder->CreateICmpSGT(
                lhs,
                llvm::ConstantInt::get(lhs->getType(), 0),
                "bool"
            );
            return result;
        }
        return nullptr;
    }

    llvm::Value* gen_arithmetic_exp(node_arithmetic_expr* exp)
    {
        if (holds_alternative<node_bin*>(exp->var))
        {
            node_bin* bin=get<node_bin*>(exp->var);
            return gen_bin(bin);
        }
        else if (holds_alternative<node_term*>(exp->var))
        {
            node_term* term=get<node_term*>(exp->var);
            return gen_term(term);
        }
        return nullptr;
    }
    bool gen_if_pred(node_if_pred* pred ,llvm::BasicBlock* merge)
    {
        if (holds_alternative<node_if_pred_else_if*>(pred->var))
        {
            node_if_pred_else_if* else_if=get<node_if_pred_else_if*>(pred->var);
            llvm::Value* condition= gen_bool_exp(else_if->expr);
            llvm::Function* current_function =builder->GetInsertBlock()->getParent();
            llvm::BasicBlock *then=llvm::BasicBlock::Create(
                *context ,
                "then" ,
                current_function
                );

            if (else_if->pred)
            {
                llvm::BasicBlock *pred_block=llvm::BasicBlock::Create(
                *context ,
                "pred" ,
                current_function
                );
                builder->CreateCondBr(
                    condition,
                    then,
                    pred_block
                );
                builder->SetInsertPoint(then);
                gen_scope(else_if->scope);
                bool then_reachable = !builder->GetInsertBlock()->getTerminator();

                if (then_reachable)
                {
                    builder->CreateBr(merge);
                }

                builder->SetInsertPoint(pred_block);

                bool pred_reachable =
                    gen_if_pred(else_if->pred.value(), merge);

                return then_reachable || pred_reachable;
            }
            else
            {
                builder->CreateCondBr(
                   condition,
                   then,
                   merge
               );
                builder->SetInsertPoint(then);
                gen_scope(else_if->scope);
                bool then_reachable =
                 !builder->GetInsertBlock()->getTerminator();

                if (then_reachable)
                {
                    builder->CreateBr(merge);
                }


                return true;
            }
        }
        else if (holds_alternative<node_if_pred_else*>(pred->var))
        {
            node_if_pred_else* _else=get<node_if_pred_else*>(pred->var);
            gen_scope(_else->scope);
            bool then_reachable = !builder->GetInsertBlock()->getTerminator();

            if (then_reachable)
            {
                builder->CreateBr(merge);
            }
            return then_reachable;

        }
        return false;
    }

    bool gen_st(node_st* st)
    {
        if (holds_alternative<node_return*>(st->var))
        {
            node_return* _return=get<node_return*>(st->var);
            llvm::Value* exp =gen_arithmetic_exp(_return->expr);
            builder->CreateRet(exp);
            return false;
        }
        else if (holds_alternative<node_scope*>(st->var))
        {
            node_scope* scope=get<node_scope*>(st->var);
            return gen_scope(scope);
        }
        else if (holds_alternative<node_let*>(st->var))
        {
            node_let* let =get<node_let*>(st->var);
            llvm::Value* exp=gen_arithmetic_exp(let->expr);
            std::string name=let->ident.value.value();
            llvm::Type *Int=llvm::Type::getInt32Ty(*context);
            llvm::AllocaInst *variable =
                builder->CreateAlloca(
                    Int,
                    nullptr,
                    name
                );

            builder->CreateStore(exp, variable);
            if (!define_variable(name, variable , let->is_const))
            {
                cerr<<"variable already declared: "<<name<<endl;
                exit(EXIT_FAILURE);
            }
            return true;
        }
        else if (holds_alternative<node_assignment*>(st->var))
        {
            node_assignment* assignment=get<node_assignment*>(st->var);

            auto it=m_variable.find(assignment->ident.value.value());

            if (it==m_variable.end())
            {
                cerr<<"undeclared identefir"<<assignment->ident.value.value()<<endl;
                exit(EXIT_FAILURE);
            }
            else
            {
                if (it->second.is_const==false)
                {
                    llvm::Value* value =gen_arithmetic_exp(assignment->expr);
                    builder->CreateStore(value, it->second.value);
                }
                else
                {
                    cerr<<it->first<<"is const"<<endl;
                    exit(EXIT_FAILURE);

                }
            }

            return true;
        }
        else if (holds_alternative<node_if*>(st->var))
        {
            node_if* _if=get<node_if*>(st->var);
            llvm::Value *condition=gen_bool_exp(_if->expr);
            llvm::Function* current_function =builder->GetInsertBlock()->getParent();
            llvm::BasicBlock *then=llvm::BasicBlock::Create(*context , "then" , current_function);
            llvm::BasicBlock *merge=llvm::BasicBlock::Create(*context ,"merge" ,current_function);

            if (_if->pred)
            {
                llvm::BasicBlock *pred=llvm::BasicBlock::Create(*context ,"pred" ,current_function);
                builder->CreateCondBr(
                    condition,
                    then,
                    pred
                );
                builder->SetInsertPoint(then);
                bool then_reachable = gen_scope(_if->scope);

                if (then_reachable)
                {
                    builder->CreateBr(merge);
                }

                builder->SetInsertPoint(pred);

                bool pred_reachable =
                    gen_if_pred(_if->pred.value(), merge);

                if (then_reachable || pred_reachable)
                {
                    builder->SetInsertPoint(merge);
                    return true;
                }

                merge->eraseFromParent();
                return false;

            }
            else
            {
                builder->CreateCondBr(
                   condition,
                   then,
                   merge
               );
                builder->SetInsertPoint(then);
                bool then_reachable = gen_scope(_if->scope);

                if (then_reachable)
                {
                    builder->CreateBr(merge);
                }

                builder->SetInsertPoint(merge);
                return true;

            }
        }
        else if (holds_alternative<node_while*>(st->var))
        {
            node_while* _while=get<node_while*>(st->var);
            llvm::Function* current_function =builder->GetInsertBlock()->getParent();
            llvm::BasicBlock *conditionBlock =
            llvm::BasicBlock::Create(
                *context,
                "loop.cond",
                current_function
                );

            llvm::BasicBlock *bodyBlock =
                llvm::BasicBlock::Create(
                    *context,
                    "loop.body",
                    current_function
                    );

            llvm::BasicBlock *exitBlock =
                llvm::BasicBlock::Create(
                    *context,
                    "loop.exit",
                    current_function
                    );
            builder->CreateBr(conditionBlock);
            builder->SetInsertPoint(conditionBlock);
            llvm::Value* condition=gen_bool_exp(_while->expr);

            builder->CreateCondBr(condition ,
                bodyBlock ,
                exitBlock
                );
            builder->SetInsertPoint(bodyBlock);
            bool body_reachable = gen_scope(_while->scope);

            if (body_reachable)
            {
                builder->CreateBr(conditionBlock);
            }
            builder->SetInsertPoint(exitBlock);
            return true;

        }
        else if (holds_alternative<node_fun*>(st->var))
        {
            node_fun* fun=get<node_fun*>(st->var);
            llvm::Type *Int=llvm::Type::getInt32Ty(*context);
            vector<llvm::Type*>param_types;
            for (auto type:fun->param)
            {
                param_types.push_back(Int);
            }
            llvm::FunctionType* function_type =
            llvm::FunctionType::get(
                Int,
                param_types,
                false
            );
            llvm::Function* function =
                llvm::Function::Create(
                    function_type,
                    llvm::Function::ExternalLinkage,
                    fun->ident.value.value(),
                    *module
                );

            auto old_insert_point = builder->saveIP();
            llvm::BasicBlock* function_entry =
                llvm::BasicBlock::Create(*context, "entry", function);
            builder->SetInsertPoint(function_entry);
            scope_begin();
            auto arg = function->arg_begin();
            for (int i=0; i<fun->param.size() ;++i)
            {
                llvm::Value* argument = &*arg++;
                argument->setName(fun->param[i].first.value.value());
                llvm::AllocaInst* variable =
                builder->CreateAlloca(Int, nullptr, fun->param[i].first.value.value());

                builder->CreateStore(argument, variable);

                if (!define_variable(fun->param[i].first.value.value(), variable ,fun->param[i].second ))
                {
                    cerr << "parameter already declared: " << fun->param[i].first.value.value()<< endl;
                    exit(EXIT_FAILURE);
                }
            }

            bool function_reachable = gen_scope(fun->scope);
            scope_end();
            if (function_reachable)
            {
                cerr << "not all paths return a value in function: "
                     << fun->ident.value.value() << endl;
                exit(EXIT_FAILURE);
            }
            builder->restoreIP(old_insert_point);
            m_fun[m_count_fun]=function;
            m_fun_code.push_back({fun->ident.value.value() , m_count_fun});
            m_count_fun++;
            return true;
        }
        return false;
    }

    void gen()
    {
        llvm::Type *Int=llvm::Type::getInt32Ty(*context);
        llvm::FunctionType *type_function=
            llvm::FunctionType::get(
                Int ,
                false
            );
            main_function=
            llvm::Function::Create(type_function,
                llvm::Function::ExternalLinkage ,
                "main",
                *module
            );
         llvm::BasicBlock* entry_main=
            llvm::BasicBlock::Create(
                *context ,
                "entry_main" ,
                main_function
            );
        builder->SetInsertPoint(entry_main);
        scope_begin();
        for (node_st* st: m_root->stmt)
        {
            gen_st(st);
        }
        scope_end();
        std::error_code error;
        llvm::raw_fd_ostream output("ylang.ll", error);

        module->print(output, nullptr);
    }


    void scope_begin()
    {
        m_scope.emplace_back();
    }
    void scope_end()
    {
        for (auto it=m_scope.back().rbegin() ;it !=m_scope.back().rend(); ++it )
        {
            if (it->exist)
            {
                m_variable[it->name]=it->prev;
            }
            else
            {
                m_variable.erase(it->name);
            }
        }
        m_scope.pop_back();
    }
    bool define_variable(const std::string& name, llvm::Value* value ,bool is_const)
    {
        auto& current_scope=m_scope.back();

        for (const auto& entry:current_scope)
        {
            if (entry.name==name)
                return false;
        }
        auto it = m_variable.find(name);
        bool existed = it != m_variable.end();
        current_scope.push_back({
            name,
            existed?it->second:variable{nullptr, false},
            existed
        });
        m_variable[name]={value,is_const };
        return true;
    }

private:

    struct variable
    {
        llvm::Value* value;
        bool is_const;
    };
    struct scope
    {
        string name;
        variable prev;
        bool exist;
    };

    vector<vector<scope>>m_scope;
    node_prog* m_root;
    std::unique_ptr<llvm::LLVMContext>context;
    std::unique_ptr<llvm::Module>module;
    std::unique_ptr<llvm::IRBuilder<>>builder;
    std::map<std::string,variable>m_variable;
    llvm::Function *main_function;
    vector<std::pair<std::string , std::size_t>>m_fun_code;
    map< std::size_t, llvm::Function*>m_fun;
    std::size_t m_count_fun=0;

};
