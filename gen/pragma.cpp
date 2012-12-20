//===-- pragma.cpp --------------------------------------------------------===//
//
//                         LDC – the LLVM D compiler
//
// This file is distributed under the BSD-style LDC license. See the LICENSE
// file for details.
//
//===----------------------------------------------------------------------===//

#include "pragma.h"
#include "attrib.h"
#include "id.h"
#include "expression.h"
#include "scope.h"
#include "module.h"
#include "declaration.h"
#include "template.h"
#include "llvm/Support/CommandLine.h"

static bool parseStringExp(Expression* e, std::string& res)
{
    StringExp *s = NULL;

    e = e->optimize(WANTvalue);
    if (e->op == TOKstring && (s = static_cast<StringExp *>(e)))
    {
        char* str = static_cast<char*>(s->string);
        res = str;
        return true;
    }
    return false;
}

static void pragmaDeprecated(Identifier* oldIdent, Identifier* newIdent)
{
#ifndef DMDV1
    // Do not print a deprecation warning for D1 – we do not want to
    // introduce needless breakage at this stage.
    if (!global.params.useDeprecated)
        error("non-vendor-prefixed pragma '%s' is deprecated; use '%s' instead");
#endif
}

static bool matchPragma(Identifier* needle, Identifier* ident, Identifier* oldIdent)
{
    if (needle == ident) return true;
    if (needle == oldIdent)
    {
        pragmaDeprecated(oldIdent, ident);
        return true;
    }
    return false;
}

Pragma DtoGetPragma(Scope *sc, PragmaDeclaration *decl, std::string &arg1str)
{
    Identifier *ident = decl->ident;
    Expressions *args = decl->args;
    Expression *expr = (args && args->dim > 0) ? (*args)[0]->semantic(sc) : 0;

    // pragma(intrinsic, "string") { funcdecl(s) }
    if (matchPragma(ident, Id::LDC_intrinsic, Id::intrinsic))
    {
        if (!args || args->dim != 1 || !parseStringExp(expr, arg1str))
        {
             error("requires exactly 1 string literal parameter");
             fatal();
        }

        // Recognize LDC-specific pragmas.
        struct LdcIntrinsic
        {
            std::string name;
            Pragma pragma;
        };
        static LdcIntrinsic ldcIntrinsic[] = {
            { "bitop.bt",  LLVMbitop_bt },
            { "bitop.btc", LLVMbitop_btc },
            { "bitop.btr", LLVMbitop_btr },
            { "bitop.bts", LLVMbitop_bts },
        };

        static std::string prefix = "ldc.";
        if (arg1str.length() > prefix.length() &&
            std::equal(prefix.begin(), prefix.end(), arg1str.begin()))
        {
            // Got ldc prefix, binary search through ldcIntrinsic.
            std::string name(arg1str.begin() + prefix.length(), arg1str.end());
            size_t i = 0, j = sizeof(ldcIntrinsic) / sizeof(ldcIntrinsic[0]);
            do
            {
                size_t k = (i + j) / 2;
                int cmp = name.compare(ldcIntrinsic[k].name);
                if (!cmp)
                    return ldcIntrinsic[k].pragma;
                else if (cmp < 0)
                    j = k;
                else
                    i = k + 1;
            }
            while (i != j);
        }

        return LLVMintrinsic;
    }

    // pragma(notypeinfo) { typedecl(s) }
    else if (matchPragma(ident, Id::LDC_no_typeinfo, Id::no_typeinfo))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMno_typeinfo;
    }

    // pragma(nomoduleinfo) ;
    else if (matchPragma(ident, Id::LDC_no_moduleinfo, Id::no_moduleinfo))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMno_moduleinfo;
    }

    // pragma(alloca) { funcdecl(s) }
    else if (matchPragma(ident, Id::LDC_alloca, Id::Alloca))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMalloca;
    }

    // pragma(va_start) { templdecl(s) }
    else if (matchPragma(ident, Id::LDC_va_start, Id::vastart))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMva_start;
    }

    // pragma(va_copy) { funcdecl(s) }
    else if (matchPragma(ident, Id::LDC_va_copy, Id::vacopy))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMva_copy;
    }

    // pragma(va_end) { funcdecl(s) }
    else if (matchPragma(ident, Id::LDC_va_end, Id::vaend))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMva_end;
    }

    // pragma(va_arg) { templdecl(s) }
    else if (matchPragma(ident, Id::LDC_va_arg, Id::vaarg))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMva_arg;
    }

    // pragma(fence) { funcdecl(s) }
    else if (matchPragma(ident, Id::LDC_fence, Id::fence))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMfence;
    }

    // pragma(atomic_load) { templdecl(s) }
    else if (matchPragma(ident, Id::LDC_atomic_load, Id::atomic_load))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMatomic_load;
    }

    // pragma(atomic_store) { templdecl(s) }
    else if (matchPragma(ident, Id::LDC_atomic_store, Id::atomic_store))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMatomic_store;
    }

    // pragma(atomic_cmp_xchg) { templdecl(s) }
    else if (matchPragma(ident, Id::LDC_atomic_cmp_xchg, Id::atomic_cmp_xchg))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMatomic_cmp_xchg;
    }

    // pragma(atomic_rmw, "string") { templdecl(s) }
    else if (matchPragma(ident, Id::LDC_atomic_rmw, Id::atomic_rmw))
    {
        if (!args || args->dim != 1 || !parseStringExp(expr, arg1str))
        {
             error("requires exactly 1 string literal parameter");
             fatal();
        }
        return LLVMatomic_rmw;
    }

    // pragma(ldc, "string") { templdecl(s) }
    else if (ident == Id::ldc)
    {
        pragmaDeprecated(Id::ldc, Id::LDC_verbose);

        if (!args || args->dim != 1 || !parseStringExp(expr, arg1str))
        {
             error("requires exactly 1 string literal parameter");
             fatal();
        }
        else if (arg1str == "verbose")
        {
            sc->module->llvmForceLogging = true;
        }
        else
        {
            error("command '%s' invalid", expr->toChars());
            fatal();
        }

        return LLVMignore;
    }

    // pragma(LDC_verbose);
    else if (ident == Id::LDC_verbose)
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        sc->module->llvmForceLogging = true;
        return LLVMignore;
    }

    // pragma(llvm_inline_asm) { templdecl(s) }
    else if (matchPragma(ident, Id::LDC_inline_asm, Id::llvm_inline_asm))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMinline_asm;
    }

    // pragma(llvm_inline_ir) { templdecl(s) }
    else if (matchPragma(ident, Id::LDC_inline_ir, Id::llvm_inline_ir))
    {
        if (args && args->dim > 0)
        {
             error("takes no parameters");
             fatal();
        }
        return LLVMinline_ir;
    }

    return LLVMnone;
}

void DtoCheckPragma(PragmaDeclaration *decl, Dsymbol *s,
                    Pragma llvm_internal, const std::string &arg1str)
{
    if (llvm_internal == LLVMnone || llvm_internal == LLVMignore)
        return;

    if (s->llvmInternal)
    {
        error("multiple LDC specific pragmas not allowed not affect the same "
            "declaration ('%s' at '%s')", s->toChars(), s->loc.toChars());
        fatal();
    }

    Identifier *ident = decl->ident;

    switch(llvm_internal)
    {
    case LLVMintrinsic:
        if (FuncDeclaration* fd = s->isFuncDeclaration())
        {
            fd->llvmInternal = llvm_internal;
            fd->intrinsicName = arg1str;
            fd->linkage = LINKintrinsic;
            static_cast<TypeFunction*>(fd->type)->linkage = LINKintrinsic;
        }
        else if (TemplateDeclaration* td = s->isTemplateDeclaration())
        {
            td->llvmInternal = llvm_internal;
            td->intrinsicName = arg1str;
        }
        else
        {
            error("only allowed on function declarations");
            fatal();
        }
        break;

    case LLVMatomic_rmw:
        if (TemplateDeclaration* td = s->isTemplateDeclaration())
        {
            td->llvmInternal = llvm_internal;
            td->intrinsicName = arg1str;
        }
        else
        {
            error("the '%s' pragma is only allowed on template declarations",
                ident->toChars());
            fatal();
        }
        break;

    case LLVMva_start:
    case LLVMva_arg:
    case LLVMatomic_load:
    case LLVMatomic_store:
    case LLVMatomic_cmp_xchg:
        if (TemplateDeclaration* td = s->isTemplateDeclaration())
        {
            if (td->parameters->dim != 1)
            {
                error("the '%s' pragma template must have exactly one template parameter",
                    ident->toChars());
                fatal();
            }
            else if (!td->onemember)
            {
                error("the '%s' pragma template must have exactly one member",
                    ident->toChars());
                fatal();
            }
            else if (td->overnext || td->overroot)
            {
                error("the '%s' pragma template must not be overloaded",
                    ident->toChars());
                fatal();
            }
            td->llvmInternal = llvm_internal;
        }
        else
        {
            error("the '%s' pragma is only allowed on template declarations",
                ident->toChars());
            fatal();
        }
        break;

    case LLVMva_copy:
    case LLVMva_end:
    case LLVMfence:
    case LLVMbitop_bt:
    case LLVMbitop_btc:
    case LLVMbitop_btr:
    case LLVMbitop_bts:
        if (FuncDeclaration* fd = s->isFuncDeclaration())
        {
            fd->llvmInternal = llvm_internal;
        }
        else
        {
            error("the '%s' pragma is only allowed on function declarations",
                ident->toChars());
            fatal();
        }
        break;

    case LLVMno_typeinfo:
        s->llvmInternal = llvm_internal;
        break;

    case LLVMalloca:
        if (FuncDeclaration* fd = s->isFuncDeclaration())
        {
            fd->llvmInternal = llvm_internal;
        }
        else
        {
            error("the '%s' pragma must only be used on function declarations "
                "of type 'void* function(uint nbytes)'", ident->toChars());
            fatal();
        }
        break;

    case LLVMinline_asm:
        if (TemplateDeclaration* td = s->isTemplateDeclaration())
        {
            if (td->parameters->dim > 1)
            {
                error("the '%s' pragma template must have exactly zero or one "
                    "template parameters", ident->toChars());
                fatal();
            }
            else if (!td->onemember)
            {
                error("the '%s' pragma template must have exactly one member",
                    ident->toChars());
                fatal();
            }
            td->llvmInternal = llvm_internal;
        }
        else
        {
            error("the '%s' pragma is only allowed on template declarations",
                ident->toChars());
            fatal();
        }
        break;

    case LLVMinline_ir:
        if (TemplateDeclaration* td = s->isTemplateDeclaration())
        {
            Dsymbol* member = td->onemember;
            if (!member)
            {
                error("the '%s' pragma template must have exactly one member",
                    ident->toChars());
                fatal();
            }
            FuncDeclaration* fun = member->isFuncDeclaration();
            if (!fun)
            {
                error("the '%s' pragma template's member must be a function declaration",
                    ident->toChars());
                fatal();
            }

            TemplateParameters& params = *td->parameters;
            bool valid_params =
                params.dim == 3 && params[1]->isTemplateTypeParameter() &&
                params[2]->isTemplateTupleParameter();

            if(valid_params)
            {
                TemplateValueParameter* p0 = params[0]->isTemplateValueParameter();
                valid_params = valid_params && p0 && p0->valType == Type::tstring;
            }

            if(!valid_params)
            {
                error("the '%s' pragma template must have exactly three parameters: "
                    "a string, a type and a type tuple", ident->toChars());
                fatal();
            }

            td->llvmInternal = llvm_internal;
        }
        else
        {
            error("the '%s' pragma is only allowed on template declarations",
                ident->toChars());
            fatal();
        }
        break;

    default:
        warning(Loc(),
            "the LDC specific pragma '%s' is not yet implemented, ignoring",
            ident->toChars());
    }
}