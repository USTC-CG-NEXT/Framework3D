#pragma once

// #include "GCore/geom_node_global_payload.h"
#include "GUI/ui_event.h"
#include "USTC_CG.h"
#include "nodes/core/node.hpp"
#include "nodes/core/node_tree.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct NodeTreeExecutor;

class NodeSystemExecution {
   public:
    virtual ~NodeSystemExecution() = default;
    virtual float cached_last_frame() const;
    virtual void set_required_time_code(float time_code_to_render)
    {
    }

    NodeSystemExecution();

    virtual Node* create_node_menu();

    std::vector<std::unique_ptr<Node>>& get_nodes();

    std::string Serialize();
    void Deserialize(const std::string& str);

    NodeSocket* FindPin(SocketID id)
    {
        return node_tree->find_pin(id);
    }

    NodeLink* FindLink(LinkId id)
    {
        return node_tree->find_link(id);
    }

    virtual void CreateLink(SocketID startPinId, SocketID endPinId);

    virtual void MarkDirty()
    {
        required_execution = true;
    }

    void RemoveLink(LinkId linkId);

    bool IsPinLinked(SocketID id)
    {
        return node_tree->is_pin_linked(id);
    }

    void delete_node(NodeId id);

    void trigger_refresh_topology();

    void show_debug_info();

    unsigned GetNextId();

    virtual void try_execution();

    std::vector<std::unique_ptr<NodeLink>>& get_links()
    {
        return node_tree->links;
    }

    std::unique_ptr<NodeTree> node_tree;
    std::unique_ptr<NodeTreeExecutor> executor;

    bool CanCreateLink(NodeSocket* a, NodeSocket* b);

   protected:
    bool required_execution = false;
    Node* default_node_menu(
        const std::map<std::string, NodeTypeInfo*>& registry);

    unsigned m_NextId = 1;
};

struct GeoNodeSystemExecution : public NodeSystemExecution {
    GeoNodeSystemExecution();

    void set_edited_prim_path(const pxr::SdfPath& sdf_path);
    void consume_pickevent(PickEvent* pick);

    float cached_last_frame() const override;

    void MarkDirty() override;

    void set_required_time_code(float time_code_to_render) override;

    void try_execution() override;
    Node* create_node_menu() override;

   private:
    GeomNodeGlobalParams params_;
    float cached_last_frame_ = 0;
    float time_code_to_render_ = 0;
    bool just_renewed = true;
};

struct RenderNodeSystemExecution : public NodeSystemExecution {
    RenderNodeSystemExecution();
    Node* create_node_menu() override;
};

struct CompositionNodeSystemExecution : public NodeSystemExecution {
    CompositionNodeSystemExecution();
    void try_execution() override;
    Node* create_node_menu() override;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
