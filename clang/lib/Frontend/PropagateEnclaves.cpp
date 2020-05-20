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

#include "clang/Frontend/PropagateEnclaves.h"
#include "clang/AST/Attr.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/LexDiagnostic.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_set>
#include <unordered_map>

using namespace clang;


template<typename N>
class Gr {

public:
    typedef N Node;

    std::unordered_map<Node, std::vector<Node>> edges;
    std::vector<Node> dfs_enumeration_util(std::unordered_set<Node> *seen, Node start) const;
    std::vector<std::vector<Node>> dfs_enumeration() const;
    std::vector<std::vector<Node>> scc_enumeration() const;
    Gr transpose() const;

    void set_node(Node, std::vector<Node> connections);
};

template<typename N>
void Gr<N>::set_node(Node node, std::vector<Node> targets)
{
    edges.emplace(node, targets);
}

template<typename N>
Gr<N> Gr<N>::transpose() const
{
    Gr<N> output;

    for (auto const& entry : edges) {
        output.edges[entry.first] = {};
    }

    for (auto const& entry : edges) {
        for (auto target : entry.second) {
            output.edges[target].push_back(entry.first);
        }
    }

    return output;
}


template <typename N>
std::vector<typename Gr<N>::Node>
Gr<N>::dfs_enumeration_util(std::unordered_set<Node> *seen, Node start) const {
    std::vector<Node> local;
    std::vector<Node> work { start };

    while (!work.empty()) {
        auto current = work.back();
        work.pop_back();
            
        if (!seen->insert(current).second) {
                continue;
        }

        local.push_back(current);

        auto const& next = edges.find(current)->second;
        work.insert(work.end(), next.begin(), next.end());
    }

    std::reverse(local.begin(), local.end());

    return local;
}

template<typename N>
std::vector<std::vector<typename Gr<N>::Node>>
Gr<N>::dfs_enumeration() const {

    std::unordered_set<Node> seen;
    std::vector<std::vector<Node>> output;

    for (auto const& entry : edges) {
        output.emplace_back(dfs_enumeration_util(&seen, entry.first));
    }

    return output;
}

template <typename N>
std::vector<std::vector<typename Gr<N>::Node>>
Gr<N>::scc_enumeration() const
{
    std::vector<Node> dfs_flat;
    for (auto const& tree : dfs_enumeration()) {
        dfs_flat.insert(dfs_flat.end(), tree.begin(), tree.end());
    }

    auto g_ = transpose();
    std::vector<std::vector<Node>> output;
    std::unordered_set<Node> seen;

    while (!dfs_flat.empty()) {
        auto current = dfs_flat.back();
        dfs_flat.pop_back();

        output.push_back(g_.dfs_enumeration_util(&seen, current));
    }

    return output;
}

template<typename N>
Gr<unsigned long>
scc_graph(
    Gr<N> const& g,
    std::vector<std::vector<typename Gr<N>::Node>> components)
{
    Gr<unsigned long> output;
    auto n = components.size();

    std::unordered_map<typename Gr<N>::Node, unsigned long> component_map;

    for (unsigned long i = 0; i < n; i++) {
        for (auto x : components[i]) {
            component_map[x] = i;
        }
    }

    for (decltype(n) i = 0; i < n; i++) {
        auto& edges = output.edges[i];
        for (auto member : components[i]) {
            for (auto outgoing : g.edges.find(member)->second) {
                auto target = component_map[outgoing];
                if (target != i) edges.push_back(target);                
            }
        }
        std::sort(edges.begin(), edges.end());
        auto newend = std::unique(edges.begin(), edges.end());
        edges.erase(newend, edges.end());
    }

    return output;
}




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

  bool VisitCXXMethodDecl(CXXMethodDecl *D) {
    currentSet.insert(D->getParent()->getCanonicalDecl());
    return true; // continue
  }

  bool VisitDeclRefExpr(DeclRefExpr* D) {
    currentSet.insert(D->getDecl()->getCanonicalDecl());
    return true; // continue
  }
};

class PropagateAnnotationsConsumer : public ASTConsumer {

  void report(
    std::vector<std::vector<Decl *>> const& components,
    Gr<unsigned long> const& g
    )
  {
      for (unsigned long i = 0; i < components.size(); ++i) {
        for (auto decl : components[i]) {
          if (auto *x = dyn_cast<NamedDecl>(decl)) {
            llvm::errs() << i << ": " << x->getNameAsString() << "\n";
            x->dumpColor();
          }
        }
      }

      for (auto x : g.edges) {
        for (auto y : x.second) {
          llvm::errs() << x.first << " -> " << y << "\n";
        }
      }
  }

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
        for (auto const attr : p->specific_attrs<PirateCapabilityAttr>()) {
          decl_caps[i].insert(attr->getCapability());
        }
        for (auto const attr : p->specific_attrs<PirateEnclaveOnlyAttr>()) {
          decl_enclaves[i].insert(attr->getEnclaveName());
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

    //report(components, scc_gr);
  }
};
}

std::unique_ptr<ASTConsumer>
PropagateAnnotationsAction::CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef)
{
  return std::make_unique<PropagateAnnotationsConsumer>();
}

bool
PropagateAnnotationsAction::ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) 
{
  return true;
}

PluginASTAction::ActionType
PropagateAnnotationsAction::getActionType()
{
    return AddBeforeMainAction;
}
