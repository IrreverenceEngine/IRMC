#include <IRMC_Octree.hpp>

namespace IRMC {

    void Octree::Split(const glm::vec3& pos)
    {
        UInt32 idx = 0;
        AABB tmpbounds = bounds;

        while (true) {
            Node* node = &nodes[idx];

            if (node->depth >= maxDepth) {
                return;
            }

            if (node->firstChild == UINT32_MAX) {
                UInt32 firstChild = nodes.size();
                nodes.resize(nodes.size() + 8);

                node = &nodes[idx];
                node->firstChild = firstChild;

                for (UInt32 i = 0; i < 8; i++) {
                    Node& child = nodes[firstChild + i];

                    child.firstChild = UINT32_MAX;
                    child.parent = idx;
                    child.depth = node->depth + 1;
                    child.data = INVALID_DATA;
                }
            }

            UInt8 octant = OctantFromPos(tmpbounds, pos);
            idx = node->firstChild + octant;

            Octree::SliceBoundsByOctant(octant, &tmpbounds);
        }
    }

    void Octree::Split(UInt32 idx)
    {
        Node* node = &nodes[idx];
        if (node->firstChild == UINT32_MAX) {
            UInt32 firstChild = nodes.size();
            nodes.resize(nodes.size() + 8);

            node = &nodes[idx];
            node->firstChild = firstChild;

            for (UInt32 i = 0; i < 8; i++) {
                Node& child = nodes[firstChild + i];

                child.firstChild = UINT32_MAX;
                child.parent = idx;
                child.depth = node->depth + 1;
                child.data = INVALID_DATA;
            }
        }
    }

    void Octree::Traverse(const glm::vec3& pos, bool split, std::function<bool(UInt32, UInt8, const AABB&, UInt64)> func)
    {
        UInt32 idx = 0;
        AABB tmpbounds = bounds;

        while (true) {
            Node* node = &nodes[idx];

            if (!func(idx, node->depth, tmpbounds, node->data)) {
                return;
            }

            if (node->depth >= maxDepth) {
                return;
            }

            if (split) {
                Split(idx);
            } else {
                if (node->firstChild == UINT32_MAX) {
                    return;
                }
            }

            UInt8 octant = OctantFromPos(tmpbounds, pos);
            idx = node->firstChild + octant;

            Octree::SliceBoundsByOctant(octant, &tmpbounds);
        }
    }

    UInt32 Octree::GetNode(const glm::vec3& pos)
    {
        UInt32 idx = 0;
        AABB tmpbounds = bounds;

        while (true) {
            Node* node = &nodes[idx];

            if (node->depth >= maxDepth || node->firstChild == UINT32_MAX) {
                return idx;
            }

            UInt8 octant = OctantFromPos(tmpbounds, pos);
            idx = node->firstChild + octant;

            Octree::SliceBoundsByOctant(octant, &tmpbounds);
        }
    }

    void Octree::PushNodeChildren(UInt32 idx, std::vector<UInt32>& vec)
    {
        UInt32 firstChild = nodes[idx].firstChild;
        if (firstChild == INVALID_IDX) { // Node is a leaf, has no children
            return;
        }
        
        for (UInt8 i = 0; i < 8; i++) {
            vec.push_back(firstChild + i);
        }
    }

    void Octree::SetNodeData(UInt32 idx, UInt64 data)
    {
        nodes[idx].data = data;
    }

    UInt8 Octree::OctantFromPos(const AABB& bounds, const glm::vec3& pos)
    {
        const glm::vec3 center = bounds.GetCenter();
        return ((pos.x >= center.x) << 0) | ((pos.y >= center.y) << 1) | ((pos.z >= center.z) << 2);
    }
    
    void Octree::SliceBoundsByOctant(UInt8 octant, AABB* bounds)
    {
        const glm::vec3 center = bounds->GetCenter();

        if (octant & 1) bounds->min.x = center.x;
        else bounds->max.x = center.x;

        if (octant & 2) bounds->min.y = center.y;
        else bounds->max.y = center.y;

        if (octant & 4) bounds->min.z = center.z;
        else bounds->max.z = center.z;
    }
    
}
