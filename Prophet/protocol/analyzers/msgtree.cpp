#include "stdafx.h"
#include "msgtree.h"


bool StackHashComparator::Equals( const MessageAccess *l, const MessageAccess *r )
{
    Assert(l && r);
    return GetProcStackHash(l->CallStack) == GetProcStackHash(r->CallStack);
}


MsgTree::MsgTree(Message *msg)
    : m_root(NULL), m_message(msg)
{

}

MsgTree::~MsgTree()
{
    SAFE_DELETE(m_root);
}

void MsgTree::Construct( const MessageAccessLog *log, MessageAccessComparator &cmp )
{
#define MINIMUM_SEQUENCE_LENGTH 4
    LxDebug("Constructing message tree\n");
    m_root = new TreeNode(0, m_message->Size() - 1);

    const MessageAccess *prev = log->Get(0);
    TreeNode *currNode = new TreeNode(prev->Offset, prev->Offset);
    for (int i = 1; i < log->Count(); i++) {
        const MessageAccess *curr = log->Get(i);

        if (curr->Offset == prev->Offset && cmp.Equals(curr, prev)) {
            // ignore local visits
            continue;
        }

        if (curr->Offset == prev->Offset+1 && cmp.Equals(curr, prev)) {
            currNode->m_r++;
        } else {

            if (currNode->Length() >= MINIMUM_SEQUENCE_LENGTH || currNode->Length() == 1) {
#if 0
                LxInfo("inserting [%d,%d]\n", currNode->m_l, currNode->m_r);
                if (currNode->m_l == 0x46 && currNode->m_r == 0x50) {
                    LxDebug("debug\n");
                }
#endif
                Insert(currNode);
                currNode = new TreeNode(curr->Offset, curr->Offset);
            } else {
                currNode->m_l = currNode->m_r = curr->Offset;
            }
#if 0
            if (!CheckValidity()) {
                LxFatal("MessageTree validity check failed\n");
            }
#endif
        }
        prev = curr;
    }

    if (currNode->Length() >= MINIMUM_SEQUENCE_LENGTH || currNode->Length() == 1) {
        Insert(currNode);
    } else {
        delete currNode;
    }
    

#ifdef _DEBUG
    if (!CheckValidity()) {
        LxFatal("MessageTree validity check failed\n");
    }
#endif

    m_root->FixParent();
#undef MINIMUM_SEQUENCE_LENGTH
}

void MsgTree::Insert( TreeNode *node )
{
    m_root->Insert(node);
}

bool MsgTree::CheckValidity() const
{
    return m_root->CheckValidity();
}

void MsgTree::Dump( File &f ) const
{
    m_root->Dump(f, m_message, 0);
}

void MsgTree::DumpDot( File &f, bool isRoot ) const
{
    if (isRoot) {
        fprintf(f.Ptr(), "digraph %s {\n", m_message->GetName().c_str());
        fprintf(f.Ptr(), "node [shape=box,fontname=\"Consolas\", fontsize=14];\n");
    } else {
        fprintf(f.Ptr(), "subgraph cluster_%s {\n", m_message->GetName().c_str());
    }

    m_root->DumpDot(f, m_message);

    fprintf(f.Ptr(), "}\n");
}

void MsgTree::UpdateHistory( const MessageAccessLog *t )
{
    m_root->UpdateHistory(t);
}

TreeNode * MsgTree::FindOrCreateNode( const MemRegion &r )
{
    return FindOrCreateNode(TaintRegion(r.Addr - m_message->GetRegion().Addr, r.Len));
}

TreeNode * MsgTree::FindOrCreateNode( const TaintRegion &tr )
{
    int left = tr.Offset, right = tr.Offset + tr.Len - 1;
    m_root->Insert(new TreeNode(left, right));
    m_root->FixParent();
    if (!CheckValidity()) {
        LxFatal("Validity failed!\n");
    }

    return DoFindNode(tr);
}

TreeNode * MsgTree::DoFindNode( const MemRegion &r ) const
{
    Assert(m_message->GetRegion().Contains(r));
    return DoFindNode(TaintRegion(r.Addr - m_message->GetRegion().Addr, r.Len));
}

TreeNode * MsgTree::DoFindNode( const TaintRegion &tr ) const
{
    int left = tr.Offset;
    int right = tr.Offset + tr.Len - 1;
    TreeNode *n = m_root;

    while (true) {
        Assert(n->m_l <= left && n->m_r >= right);
        if (n->m_l == left && n->m_r == right) return n;
        if (n->IsLeaf()) {
            //return n;
            TreeNode *newNode = new TreeNode(left, right, n);
            n->Insert(newNode);
            return newNode;
        }

        bool found = false;
        for (TreeNode *ch : n->m_children) {
            if (ch->m_l <= left && ch->m_r >= right) {
                n = ch;
                found =  true;
                break;
            }
        }
        if (!found) { return n; }
    }
}

TreeNode * MsgTree::FindNode( u32 addr ) const
{
    int offset = addr - m_message->GetRegion().Addr;
    TreeNode *n = m_root;
    while (!n->IsLeaf()) {
        Assert(n->m_l <= offset && n->m_r >= offset);
        for (auto &ch : n->m_children) {
            if (ch->Contains(offset, offset)) {
                n = ch;
                break;
            }
        }
    }
    return n;
}

TreeNode * MsgTree::FindNode( const TaintRegion &tr ) const
{
    int l = tr.Offset, r = tr.Offset + tr.Len - 1;
    TreeNode *n = m_root;
    while (true) {
        Assert(n->m_l <= l && n->m_r >= r);
        if (n->m_l == l && n->m_r == r) return n;
        bool found = false;
        for (auto &ch : n->m_children) {
            if (ch->m_l <= l && ch->m_r >= r) {
                n = ch; found = true;
                break;
            }
        }
        if (!found) return NULL;
    }
}

TreeNode::TreeNode( int l, int r, TreeNode *parent )
    : m_l(l), m_r(r)
{
    Assert(m_l <= m_r);
    m_parent = parent;
    m_flag = 0;
}

TreeNode::~TreeNode()
{
    for (auto &node : m_children) {
        SAFE_DELETE(node);
    }
    m_children.clear();
}

bool TreeNode::Contains( int l, int r )
{
    return m_l <= l && m_r >= r;
}

bool TreeNode::Contains( const TreeNode * n )
{
    return Contains(n->m_l, n->m_r);
}

void TreeNode::Insert( TreeNode *node )
{
    Assert(Contains(node));
    Assert(node->m_children.size() == 0);

#if 0
    if (node->m_l == 130 && node->m_r == 133) {
        LxInfo("debug\n");
    }
#endif

    if (m_l == node->m_l && m_r == node->m_r) {
        delete node;
        return;
    }

    if (m_l == node->m_l && m_r == node->m_r + 1) {
        LxWarning("Ignoring -1 tree node %d-%d\n", node->m_l, node->m_r);
        delete node; return;
    }

    if (m_children.size() == 0) {
        // leaf node, extend to non-leaf
        if (m_l < node->m_l) {
            m_children.push_back(new TreeNode(m_l, node->m_l-1, this));
        }
        node->m_parent = this;
        m_children.push_back(node);
        if (m_r > node->m_r) {
            m_children.push_back(new TreeNode(node->m_r+1, m_r, this));
        }
        return;
    }
    
    // delete if node == any child
    for (uint i = 0; i < m_children.size(); i++) {
        TreeNode *c = m_children[i];
        
        if (c->m_l == node->m_l && c->m_r == node->m_r) {
            delete node; return;
        }
        if (c->m_l == node->m_l && c->m_r == node->m_r - 1 && node->Length() > 8) {
            delete node; return;
        }
        if (!c->IsLeaf()) continue;
        if (c->m_l == node->m_l && c->m_r == node->m_r + 1 
            && i < m_children.size() - 1 && node->Length() > 8
            && m_children[i+1]->IsLeaf() && c->m_r > c->m_l) 
        {
            // shrink c
            c->m_r--; m_children[i+1]->m_l--;
            delete node; return;
        }
        if (c->m_r == node->m_l && i < m_children.size() - 1 
            && m_children[i+1]->IsLeaf() && c->m_r > c->m_l) 
        {
            // shrink c; like this: 0-7,8-12 <- 7-10  => 0-6,7-10,11-12
            c->m_r--; m_children[i+1]->m_l--; 
            if (m_children[i+1]->m_r == node->m_r) {
                delete node; return;
            }
            break;  // need further processing
        }
    }

    // check if node can be parent of several children
    for (uint i = 0; i < m_children.size(); i++) {
        if (m_children[i]->m_l > node->m_l) break;
        if (m_children[i]->m_l != node->m_l) { continue; }
        uint left = i;
        for (uint j = i; j < m_children.size(); j++) {
            if (m_children[j]->m_r > node->m_r) break;
            if (m_children[j]->m_r != node->m_r) continue;
            std::vector<TreeNode *> newChildren;
            for (uint k = 0; k < left; k++)
                newChildren.push_back(m_children[k]);
            for (uint k = left; k <= j; k++) {
                m_children[k]->m_parent = node;
                node->m_children.push_back(m_children[k]);
            }
            newChildren.push_back(node);
            for (uint k = j + 1; k < m_children.size(); k++)
                newChildren.push_back(m_children[k]);
            m_children = newChildren;
            return;
        }
        break;
    }

    for (uint i = 0; i < m_children.size(); i++) {
        TreeNode *c = m_children[i];
        if (!c->Contains(node)) continue;
        if (c->m_l == node->m_l && c->m_r == node->m_r) {
            delete node;
            return;
        }
        if (!c->IsLeaf() 
            || (c->m_l < node->m_l && c->m_r > node->m_r)
            || c->m_subMessages.size() != 0
            ) {
            c->Insert(node);
            return;
        }

        std::vector<TreeNode *> newChildren;
        for (uint j = 0; j < i; j++)
            newChildren.push_back(m_children[j]);
        if (c->m_l < node->m_l) {
            newChildren.push_back(new TreeNode(c->m_l, node->m_l - 1, this));
            c->m_l = node->m_l;
        }
        
        newChildren.push_back(c);
        if (c->m_r > node->m_r) {
            newChildren.push_back(new TreeNode(node->m_r + 1, c->m_r, this));
            c->m_r = node->m_r;
        }
        for (uint j = i+1; j < m_children.size(); j++)
            newChildren.push_back(m_children[j]);

        Assert(newChildren.size() > m_children.size());
        delete node;
        m_children = newChildren;
        return;
    }

    // no big child, so insert to this
    node->m_parent = this;
    std::vector<TreeNode *> newChildren;
    for (auto c : m_children) {
        if (c->m_r < node->m_l || c->m_l > node->m_r) {
            newChildren.push_back(c);
            continue;
        }
        if (c->m_l < node->m_l) {
            // left-overlap
            c->Insert(new TreeNode(node->m_l, c->m_r));
            newChildren.push_back(c);
            node->m_l = c->m_r+1;
        } else if (c->m_r > node->m_r) {
            // right-overlap
            c->Insert(new TreeNode(c->m_l, node->m_r));
            newChildren.push_back(c);
            node->m_r = c->m_l-1;
        } else {
            c->m_parent = node;
            node->m_children.push_back(c);
        }
    }
    if (node->m_l <= node->m_r) {
        if (node->m_children.size() == 1) {
            Assert(node->m_l == node->m_children[0]->m_l && 
                node->m_r == node->m_children[0]->m_r);
            TreeNode *ch0 = node->m_children[0];
            node->m_children.clear();
            delete node;
            node = ch0;
        }
        newChildren.push_back(node);
    } else {
        delete node;
    }
    m_children = newChildren;
    std::sort(m_children.begin(), m_children.end(), 
        [](const TreeNode *l, const TreeNode *r) -> bool
    {
        return l->m_l < r->m_l;
    });
}

bool TreeNode::CheckValidity() const
{
    if (m_children.size() == 0)
        return m_l <= m_r;    // leaf node

    if (m_children.size() == 1)
        return false;       // then equals parent

#if 0
    for (auto c : m_children) {
        if (c->m_parent != this) {
            return false;
        }
    }
#endif

    int l = m_l-1;
    for (auto c : m_children) {
        if (c->m_l != l+1) 
            return false;
        l = c->m_r;
        if (!c->CheckValidity()) 
            return false;
    }
    if (l != m_r) 
        return false;
    return true;
}

void TreeNode::Dump( File &f, const Message *msg, int level ) const
{
    for (int i = 0; i < level; i++)
        fprintf(f.Ptr(), " ");
    fprintf(f.Ptr(), "[%d,%d] %s  EH:(", m_l, m_r, GetDotLabel(msg).c_str());
    for (auto val : m_execHistory) {
        fprintf(f.Ptr(), "%08x ", val);
    }
    fprintf(f.Ptr(), "), SEH:(");
    for (auto val : m_execHistoryStrict) {
        fprintf(f.Ptr(), "%08x ", val);
    }
    fprintf(f.Ptr(), ")\n");

    for (auto c : m_children)
        c->Dump(f, msg, level+1);
}

void TreeNode::DumpDot( File &f, const Message *msg ) const
{
    fprintf(f.Ptr(), "%s [label=\"%s\",%s];\n", GetDotName(msg).c_str(), 
        GetDotLabel(msg).c_str(), GetDotStyle(msg).c_str());
    std::string nodeName = GetDotName(msg);

    if (!IsLeaf()) {
        fprintf(f.Ptr(), "{ rank = same; rankdir=LR; ");
        int count = 0;
        for (auto &c : m_children) {
            fprintf(f.Ptr(), count == 0 ? "%s" : "->%s", c->GetDotName(msg).c_str());
            count++;
        }

        fprintf(f.Ptr(), "[color=gray];}\n");
        for (auto &c : m_children) {
            c->DumpDot(f, msg);
            fprintf(f.Ptr(), "%s -> %s;\n", nodeName.c_str(), c->GetDotName(msg).c_str());
        }
    }

    for (Message *msg : m_subMessages) {
        std::string tagName = msg->GetName() + "_tag";
        fprintf(f.Ptr(), "%s [shape=box,color=blueviolet,style=filled,fontcolor=white,label=", tagName.c_str());
        msg->GetTag()->DumpDot(f);
        fprintf(f.Ptr(), "];\n");
        fprintf(f.Ptr(), "%s -> %s;\n", nodeName.c_str(), tagName.c_str());
        msg->GetTree()->DumpDot(f, false);
        fprintf(f.Ptr(), "%s -> %s;\n", tagName.c_str(), 
            msg->GetTree()->GetRoot()->GetDotName(msg).c_str());
    }

    for (auto &l : m_links) {
        fprintf(f.Ptr(), "%s -> %s[label=%s,style=dashed,color=red,"
            "fontcolor=red,dir=%s,penwidth=2.0,decorate=true,constraint=false];\n", 
            nodeName.c_str(), 
            l.Target->GetDotName(l.Msg).c_str(), 
            l.Desc.c_str(), l.Dir.c_str());
    }
}

std::string TreeNode::GetDotName(const Message *msg) const
{
    std::stringstream ss;
    ss << msg->GetName() << "__" << m_l << "_" << m_r;
    return ss.str();
}

std::string TreeNode::GetDotStyle( const Message *msg ) const
{
    std::stringstream ss;
    if (IsLeaf()) {
        //ss << "shape=ellipse";
        if (HasFlag(TREENODE_PARALLEL)) {
            ss << "color=blue";
        } else if (HasFlag(TREENODE_SEPARATOR)) {
            ss << "color=darkgreen";
        }
    } else {
        ss << "shape=box";
        if (HasFlag(TREENODE_PARALLEL)) {
            ss << ",color=blue,style=bold";
        } 
    }
    return ss.str();
}

std::string TreeNode::GetDotLabel( const Message *msg ) const
{
    char buf[64];
    sprintf(buf, "%s[%x:%d]", this == msg->GetTree()->GetRoot() ? 
        ("Root " + msg->GetName()).c_str() : "",
        msg->GetRegion().Addr + m_l, m_r - m_l + 1);
    strcat(buf, "\\n");
    return buf + ByteArrayToDotString(msg->GetRaw() + m_l, m_r - m_l + 1, 
        IsLeaf() ? 36 : 24);
}

void TreeNode::UpdateHistory( const MessageAccessLog *t )
{
    for (int i = 0; i < t->Count(); i++) {
        const MessageAccess *ma = t->Get(i);
        if (ma->Offset == m_l) {
            u32 entry = GetProcStackHash(ma->CallStack);
            m_execHistory.insert(entry);
            m_execHistoryStrict.push_back(entry);
        }
    }

    for (auto &c : m_children) {
        c->UpdateHistory(t);
    }
}

void TreeNode::AppendChild( TreeNode *node )
{
    node->m_parent = this;
    m_children.push_back(node);
}

TreeNode * TreeNode::GetChild( int n ) const
{
    Assert(n >= 0 && n < GetChildrenCount());
    return m_children[n];
}

void TreeNode::ClearChildren()
{
    if (IsLeaf()) return;
    bool childHasSubMsg = false;
    for (auto &ch : m_children) {
        if (ch->HasSubMessage()) {
            childHasSubMsg = true;
            break;
        }
    }
    if (childHasSubMsg) {
        for (auto &ch : m_children) {
            ch->DoClearChildren();
        }
    } else {
        DoClearChildren();
    }

}

bool TreeNode::HasSubMessage() const
{
    if (IsLeaf()) return m_subMessages.size() != 0;
    for (auto &c : m_children) {
        if (c->HasSubMessage()) return true;
    }
    return false;
}

void TreeNode::DoClearChildren()
{
    for (auto &ch : m_children) {
        SAFE_DELETE(ch);
    }
    m_children.clear();
}

void TreeNode::FixParent()
{
    for (auto &c : m_children) {
        c->m_parent = this;
        c->FixParent();
    }
}

MessageTreeRefiner::MessageTreeRefiner()
{

}

MessageTreeRefiner::~MessageTreeRefiner()
{

}

void MessageTreeRefiner::RefineTree( MsgTree &tree )
{
    Refine(tree.GetRoot());
    tree.GetRoot()->FixParent();
    if (!tree.CheckValidity()) {
        LxFatal("Tree validity failed after refining\n");
    }
}

void MessageTreeRefiner::Refine(TreeNode *node)
{
    for (auto &c : node->m_children)
        Refine(c);
    RefineNode(node);
}
