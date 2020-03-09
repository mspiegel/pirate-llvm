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

#include "clang/AST/Attr.h"
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

#if 0
namespace std {
  template<> struct hash<StringRef> {
    size_t operator()(StringRef const& s) const noexcept {
      return hash<string>{}(s.str());
    }
  };
}
#endif

namespace {

class PropagateAnnotations
 : public RecursiveASTVisitor<PropagateAnnotations> {

private:
  std::unordered_set<Decl*> currentSet;

public:
  std::unordered_map<Decl*, std::unordered_set<Decl*>> declRefs;

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
    PropagateAnnotations Visitor;
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());

    Gr<Decl*> g;
    for (auto const& x : Visitor.declRefs) {
      g.set_node(x.first, std::vector<Decl*>(x.second.begin(), x.second.end()));
    }

    auto components = g.scc_enumeration();
    auto scc_gr = scc_graph(g, components);

    std::unordered_map<unsigned long, std::unordered_set<std::string>> decl_caps;
    std::unordered_map<unsigned long, std::unordered_set<std::string>> decl_enclaves;

    // Determine pirate_capability and pirate_enclave_only used by each component
    for (size_t i = 0; i < components.size(); i++) {
      for (auto const p : components[i]) {
        if (auto *const funDecl = dyn_cast<FunctionDecl>(p)) {
          for (auto const attr : funDecl->specific_attrs<PirateCapabilityAttr>()) {
            decl_caps[i].insert(attr->getCapability());
          }
          for (auto const attr : funDecl->specific_attrs<PirateEnclaveOnlyAttr>()) {
            decl_enclaves[i].insert(attr->getEnclaveName());
          }
        }
      }
    }

    // Inherit requirements from all parents
    for (auto subtree : scc_gr.dfs_enumeration()) {
      for (auto i : subtree) {
        for (auto e : scc_gr.edges[i]) {
          // inherit capabilities
          auto const& caps = decl_caps[e];
          decl_caps[i].insert(caps.begin(), caps.end());
          // inherit enclave restrictions
          auto const& encs = decl_enclaves[e];
          decl_enclaves[i].insert(encs.begin(), encs.end());
        }
      }
    }

    // Distribute capabilities back to component members
    for (size_t i = 0; i < components.size(); i++) {
      for (auto p : components[i]) {
        if (auto *funDecl = dyn_cast<FunctionDecl>(p)) {
          // The attributes need to be on the definition to be visible
          funDecl = funDecl->getDefinition();
          {
            auto caps_needed = decl_caps[i];

            // remove attributes that are already explicitly added
            for (auto const &attr :
                 funDecl->specific_attrs<PirateCapabilityAttr>()) {
              caps_needed.erase(attr->getCapability());
            }

            // add implicit capability requirements
            for (auto const &cap : caps_needed) {
              auto a = PirateCapabilityAttr::Create(
                  Context, cap, AttributeCommonInfo(SourceRange()));
              funDecl->addAttr(a);
            }
          }

          {
            auto enclaves_needed = decl_enclaves[i];

            // remove attributes that are already explicitly added
            for (auto const &attr :
                 funDecl->specific_attrs<PirateEnclaveOnlyAttr>()) {
              enclaves_needed.erase(attr->getEnclaveName());
            }

            // add implicit capability requirements
            for (auto const &enc : enclaves_needed) {
              auto a = PirateEnclaveOnlyAttr::Create(
                  Context, enc, AttributeCommonInfo(SourceRange()));
              funDecl->addAttr(a);
            }
          }
        }
      }
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
