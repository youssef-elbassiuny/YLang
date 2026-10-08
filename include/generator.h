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



    void gen_scope(node_scope* scope)
    {

        for (node_st* st :scope->stmt)
        {
            gen_st(st);
        }

    }
    llvm::Value* gen_bin(node_bin* bin)
    {
        if (holds_alternative<node_bin_expr_add*>(bin->var))
        {
            node_bin_expr_add* add=get<node_bin_expr_add*>(bin->var);
            llvm::Value* a=gen_exp(add->lhs);
            llvm::Value* b=gen_exp(add->rhs);
            llvm::Value *result=builder->CreateAdd(a, b, "result");
            return result;
        }
        else if (holds_alternative<node_bin_expr_sub*>(bin->var))
        {
            node_bin_expr_sub* sub=get<node_bin_expr_sub*>(bin->var);
            llvm::Value* a=gen_exp(sub->lhs);
            llvm::Value* b=gen_exp(sub->rhs);
            llvm::Value *result=builder->CreateSub(a, b, "result");
            return result;
        }
        else if (holds_alternative<node_bin_expr_mult*>(bin->var))
        {
            node_bin_expr_mult* mult=get<node_bin_expr_mult*>(bin->var);
            llvm::Value* a=gen_exp(mult->lhs);
            llvm::Value* b=gen_exp(mult->rhs);
            llvm::Value *result=builder->CreateMul(a, b, "result");
            return result;

        }
        else if (holds_alternative<node_bin_expr_div*>(bin->var))
        {
            node_bin_expr_div* div=get<node_bin_expr_div*>(bin->var);
            llvm::Value* a=gen_exp(div->lhs);
            llvm::Value* b=gen_exp(div->rhs);
            llvm::Value *result=builder->CreateSDiv(a, b, "result");
            return result;

        }
        return nullptr;
    }
    llvm::Value* gen_term(node_term* term)
    {
        if (holds_alternative<node_ident*>(term->var))
        {
            node_ident* ident=get<node_ident*>(term->var);
            auto it =m_variable.find(ident->ident.value.value());
            if (it==m_variable.end())
            {
                cerr<<"undeclared identifier"<<ident->ident.value.value()<<endl;
                exit(EXIT_FAILURE);
            }
            llvm::Type *Int=llvm::Type::getInt32Ty(*context);
            return builder->CreateLoad(Int, it->second, "load_value");
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
            return gen_exp(grouped->expr);
        }
        return nullptr;
    }

    llvm::Value* gen_exp(node_expr* exp)
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
    void gen_if_pred(node_if_pred* pred ,llvm::BasicBlock* merge)
    {
        if (holds_alternative<node_if_pred_else_if*>(pred->var))
        {
            node_if_pred_else_if* else_if=get<node_if_pred_else_if*>(pred->var);
            llvm::Value* exp= gen_exp(else_if->expr);
            llvm::Value *condition =
            builder->CreateICmpSGT(
                exp,
                llvm::ConstantInt::get(
                     llvm::Type::getInt32Ty(*context),
                     0
                     ),
                     "condition"
            );
            llvm::BasicBlock *then=llvm::BasicBlock::Create(*context , "then" , main_function);
            llvm::BasicBlock *pred=llvm::BasicBlock::Create(*context ,"pred" ,main_function);


            if (else_if->pred)
            {
                builder->CreateCondBr(
                    condition,
                    then,
                    pred
                );
                builder->SetInsertPoint(then);
                gen_scope(else_if->scope);
                builder->CreateBr(merge);
                builder->SetInsertPoint(pred);

                gen_if_pred(else_if->pred.value() , merge);

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
                builder->CreateBr(merge);


            }



        }
        else if (holds_alternative<node_if_pred_else*>(pred->var))
        {
            node_if_pred_else* _else=get<node_if_pred_else*>(pred->var);
            gen_scope(_else->scope);
            builder->CreateBr(merge);


        }

    }

    void gen_st(node_st* st)
    {
        if (holds_alternative<node_return*>(st->var))
        {
            node_return* _return=get<node_return*>(st->var);
            llvm::Value* exp =gen_exp(_return->expr);
            builder->CreateRet(exp);
        }
        else if (holds_alternative<node_scope*>(st->var))
        {
            node_scope* scope=get<node_scope*>(st->var);
            gen_scope(scope);
        }
        else if (holds_alternative<node_let*>(st->var))
        {
            node_let* let =get<node_let*>(st->var);
            llvm::Value* exp=gen_exp(let->expr);
            std::string name=let->ident.value.value();
            llvm::Type *Int=llvm::Type::getInt32Ty(*context);
            llvm::AllocaInst *variable =
                builder->CreateAlloca(
                    Int,
                    nullptr,
                    name
                );

            builder->CreateStore(exp, variable);
            m_variable[name]=variable;
        }
        else if (holds_alternative<node_assignment*>(st->var))
        {
            node_assignment* assignment=get<node_assignment*>(st->var);
            auto it=m_variable.find(assignment->ident.value.value());

            if (it==m_variable.end())
            {
                cerr<<"undeclared identifier"<<assignment->ident.value.value()<<endl;
                exit(EXIT_FAILURE);
            }
            llvm::Value* value =gen_exp(assignment->expr);
            builder->CreateStore(value, it->second);
        }
        else if (holds_alternative<node_if*>(st->var))
        {
            node_if* _if=get<node_if*>(st->var);
            llvm::Value *exp=gen_exp(_if->expr);
            llvm::Value *condition =
            builder->CreateICmpSGT(
                exp,
                llvm::ConstantInt::get(
                     llvm::Type::getInt32Ty(*context),
                     0
                     ),
                     "condition"
            );
            llvm::BasicBlock *then=llvm::BasicBlock::Create(*context , "then" , main_function);
            llvm::BasicBlock *pred=llvm::BasicBlock::Create(*context ,"pred" ,main_function);
            llvm::BasicBlock *merge=llvm::BasicBlock::Create(*context ,"merge" ,main_function);

            if (_if->pred)
            {
                builder->CreateCondBr(
                    condition,
                    then,
                    pred
                );
                builder->SetInsertPoint(then);
                gen_scope(_if->scope);
                builder->CreateBr(merge);
                builder->SetInsertPoint(pred);

                gen_if_pred(_if->pred.value() , merge);
                builder->SetInsertPoint(merge);

            }
            else
            {
                builder->CreateCondBr(
                   condition,
                   then,
                   merge
               );
                builder->SetInsertPoint(then);
                gen_scope(_if->scope);
                builder->CreateBr(merge);
                builder->SetInsertPoint(merge);

            }
        }
        else if (holds_alternative<node_while*>(st->var))
        {
            node_while* _while=get<node_while*>(st->var);
            //TODO


        }

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
        for (node_st* st: m_root->stmt)
        {
            gen_st(st);
        }
        std::error_code error;
        llvm::raw_fd_ostream output("output.ll", error);

        module->print(output, nullptr);
    }

private:
    node_prog* m_root;
    std::unique_ptr<llvm::LLVMContext>context;
    std::unique_ptr<llvm::Module>module;
    std::unique_ptr<llvm::IRBuilder<>>builder;
    std::map<std::string,llvm::Value* >m_variable;
    llvm::Function *main_function;
    std::map<std::string,llvm::Value* >m_scope;

};
