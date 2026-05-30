#include <IRX_Common.hpp>

#include <vector>
#include <functional>

namespace IRMC {
    class Octree {
    public:
        constexpr static const UInt32 INVALID_IDX = UINT32_MAX;
        constexpr static const UInt64 INVALID_DATA = UINT64_MAX;
        
        Octree(const AABB& bounds, UInt8 maxDepth) : bounds(bounds), maxDepth(maxDepth) {
            nodes.push_back({ UINT32_MAX, UINT32_MAX, 1, INVALID_DATA });
        }

        void Split(const glm::vec3& pos);
        void Split(UInt32 idx);

        void Traverse(const glm::vec3& pos, bool split, std::function<bool(UInt32, UInt8, const AABB&, UInt64)> func);

        UInt32 GetNode(const glm::vec3& pos);
        UInt64 GetNodeData(UInt32 idx) const IRX_RETURN(nodes[idx].data)
        UInt32 GetNodeParent(UInt32 idx) const IRX_RETURN(nodes[idx].parent)
        UInt32 GetNodeChild(UInt32 idx, UInt8 child) const IRX_RETURN(nodes[idx].firstChild + child)
        void PushNodeChildren(UInt32 idx, std::vector<UInt32>& vec);
        bool isNodeLeaf(UInt32 idx) const IRX_RETURN(nodes[idx].firstChild == INVALID_IDX)

        void SetNodeData(UInt32 idx, UInt64 data);

        static UInt8 OctantFromPos(const AABB& bounds, const glm::vec3& pos);
        static void SliceBoundsByOctant(UInt8 octant, AABB* bounds);
        
    private:
        struct Node {
            UInt32 parent;
            UInt32 firstChild;
            UInt8 depth;

            UInt64 data;
        };

        AABB bounds;
        UInt8 maxDepth;

        std::vector<Node> nodes;
    };
}
