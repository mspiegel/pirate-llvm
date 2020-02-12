//===- PropagateEnclaves.cpp ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Propagate enclave annotations
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/LexDiagnostic.h"
#include "llvm/Support/raw_ostream.h"
#include "Gr.h"
#include <unordered_set>
#include <unordered_map>

using namespace clang;

namespace {

class PropagateAnnotations
 : public RecursiveASTVisitor<PropagateAnnotations> {

private:
  ASTContext& Context;
  std::unordered_set<Decl*> currentSet;

public:
  std::unordered_map<Decl*, std::unordered_set<Decl*>> declRefs;

  PropagateAnnotations(ASTContext &Context)
    : Context(Context)
    , currentSet()
    , declRefs() {}

  // We visit imlicit constructors because they might call
  // restricted constructors.
  bool shouldVisitImplicitCode () const { return true; }

  bool TraverseDecl(Decl *D) {
    auto previous = std::move(currentSet);
    currentSet.clear();
    auto result = RecursiveASTVisitor<PropagateAnnotations>::TraverseDecl(D);

    auto & s = declRefs[D->getCanonicalDecl()];
    for (auto p : currentSet) {
      s.insert(p);
    }

    currentSet = previous;
    return result;
  }

  bool VisitDeclRefExpr(DeclRefExpr* D) {
    currentSet.insert(D->getDecl()->getCanonicalDecl());
    return true;
  }
};

class PropagateAnnotationsConsumer : public ASTConsumer {
public:
  void HandleTranslationUnit(ASTContext &Context) override {
    PropagateAnnotations Visitor(Context);
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());

    for (auto elt : Visitor.declRefs) {
      auto decl = elt.first;
      auto links = elt.second;
      if (auto *funDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
        llvm::errs() << " " << funDecl << ":" << funDecl->getName();
      } else {
        llvm::errs() << " " << decl;
      }
      llvm::errs() << "\n  ";
      for (auto decl : links) {
        if (auto *funDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
          llvm::errs() << " " << funDecl << ":" << funDecl->getName();
        } else {
          llvm::errs() << " " << decl;
        }
      }
      llvm::errs() << "\n";
    }

      llvm::errs() << "\n\n\n";


    Gr<Decl*> g;
    for (auto const& x : Visitor.declRefs) {
      g.set_node(x.first, std::vector<Decl*>(x.second.begin(), x.second.end()));
    }

    auto components = g.scc_enumeration();
    auto scc_gr = scc_graph(g, components);

    for (size_t i = 0; i < components.size(); i++) {
      llvm::errs() << "Component " << i << "\n";

      for (auto elt : components[i]) {
        if (auto *funDecl = llvm::dyn_cast<FunctionDecl>(elt)) {
          llvm::errs() << " " << funDecl << ":" << funDecl->getName();
        } else {
          llvm::errs() << " " << elt;
        }
      }
      llvm::errs() << "\n  ->";
      for (auto const& x : scc_gr.edges[i]) {
        llvm::errs() << " " << x;
      }
      llvm::errs() << "\n";
    }
  }
};

class PropagateAnnotationsAction : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef) override {
    return std::make_unique<PropagateAnnotationsConsumer>();
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    return true;
  }

  PluginASTAction::ActionType getActionType() override {
    return AddBeforeMainAction;
  }
};

/*
class PragmaAnnotateHandler : public PragmaHandler {
public:
  PragmaAnnotateHandler() : PragmaHandler("enable_annotate") { }

  void HandlePragma(Preprocessor &PP, PragmaIntroducer Introducer,
                    Token &PragmaTok) override {

    Token Tok;
    PP.LexUnexpandedToken(Tok);
    if (Tok.isNot(tok::eod))
      PP.Diag(Tok, diag::ext_pp_extra_tokens_at_eol) << "pragma";

    if (HandledDecl) {
      DiagnosticsEngine &D = PP.getDiagnostics();
      unsigned ID = D.getCustomDiagID(
        DiagnosticsEngine::Error,
        "#pragma enable_annotate not allowed after declarations");
      D.Report(PragmaTok.getLocation(), ID);
    }

    EnableAnnotate = true;
  }
};
*/

}

static FrontendPluginRegistry::Add<PropagateAnnotationsAction>
X("propagate-enclaves", "propagate enclave annotations");

/*
static PragmaHandlerRegistry::Add<PragmaAnnotateHandler>
Y("enable_annotate","enable annotation");
*/
