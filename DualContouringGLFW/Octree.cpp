#include "Octree.hpp"
#include "SDF.hpp"
#include "Mesh.hpp"

OctreeNode* ConstructLeaf(OctreeNode* leaf);
OctreeNode* ConstructOctreeNodes(OctreeNode* node);

vec3 CalculateSurfaceNormal(const vec3& p);
vec3 ApproximateZeroCrossingPosition(const vec3& p0, const vec3& p1);
OctreeNode* SimplifyOctree(OctreeNode* node, const float threshold);
void GenerateVertexIndices(OctreeNode* node, VertexBuffer& vertexBuffer);
void ContourProcessEdge(OctreeNode* node[4], int dir, IndexBuffer& indexBuffer);
void ContourEdgeProc(OctreeNode* node[4], int dir, IndexBuffer& indexBuffer);
void ContourFaceProc(OctreeNode* node[2], int dir, IndexBuffer& indexBuffer);
void ContourCellProc(OctreeNode* node, IndexBuffer& indexBuffer);

OctreeNode* SimplifyOctree(OctreeNode* node, const float threshold)
{
    if (!node)
    {
        return nullptr;
    }

    if (node->type != OctreeNodeType::Internal)
    {
        // Can't simplify!
        return node;
    }

    svd::QefSolver qef;
    int signs[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    int midsign = -1;
    int edgeCount = 0;
    bool isCollapsible = true;

    for (int i = 0; i < 8; i++)
    {
        node->children[i] = SimplifyOctree(node->children[i], threshold);
        if (node->children[i])
        {
            OctreeNode* child = node->children[i];
            if (child->type == OctreeNodeType::Internal)
            {
                isCollapsible = false;
            }
            else
            {
                qef.add(child->drawInfo->qef);

                midsign = (child->drawInfo->corners >> (7 - i)) & 1;
                signs[i] = (child->drawInfo->corners >> i) & 1;

                edgeCount++;
            }
        }
    }

    if (!isCollapsible)
    {
        // Atleast one child is an internal node, can't collapse
        return node;
    }

    svd::Vec3 qefPosition;
    qef.solve(qefPosition, QefError, QefSweeps, QefError);
    float error = qef.getError();

    // Covert to flm vec3
    vec3 position(qefPosition.x, qefPosition.y, qefPosition.z);

    // At this point the mass point will actually be a sum, so divide to make it the average
    if (error > threshold)
    {
        // this collapse breaches the threshold
        return node;
    }

    if (position.x < node->min.x || position.x > (node->min.x + node->size)
     || position.y < node->min.y || position.y > (node->min.y + node->size)
     || position.z < node->min.z || position.z > (node->min.z + node->size))
    {
        const auto& mp = qef.getMassPoint();
        position = vec3(mp.x, mp.y, mp.z);
    }

    // Change the node from an internal node to a psuedo leaf
    OctreeDrawInfo* drawInfo = new OctreeDrawInfo;

    drawInfo->averageNormal = vec3(0.0f);

    for (int i = 0; i < 8; i++)
    {
        if (signs[i] == -1)
        {
            drawInfo->corners |= (midsign << i);
        }
        else
        {
            drawInfo->corners |= (signs[i] << i);
        }

        if (node->children[i])
        {
            OctreeNode* child = node->children[i];
            if (child->type == OctreeNodeType::Pseudo
             || child->type == OctreeNodeType::Leaf)
            {
                drawInfo->averageNormal += child->drawInfo->averageNormal;
            }
        }
    }

    drawInfo->averageNormal = glm::normalize(drawInfo->averageNormal);
    drawInfo->position = position;
    drawInfo->qef = qef.getData();

    for (int i = 0; i < 8; i++)
    {
        DestroyOctree(node->children[i]);
        node->children[i] = nullptr;
    }

    node->type = OctreeNodeType::Pseudo;
    node->drawInfo = drawInfo;

    return node;
}

OctreeNode * BuildOctree(const ivec3 & min, const int size, float threshold)
{
    OctreeNode* root = new OctreeNode;
    root->min = min;
    root->size = size;
    root->type = OctreeNodeType::Internal;

    ConstructOctreeNodes(root);
    root = SimplifyOctree(root, threshold);

    return root;
}

void DestroyOctree(OctreeNode * node)
{
    if (!node)
    {
        return;
    }

    for (int i = 0; i < 8; i++)
    {
        DestroyOctree(node->children[i]);
    }

    if (node->drawInfo)
    {
        delete node->drawInfo;
    }

    delete node;
}

void GenerateMeshFromOctree(OctreeNode * node, VertexBuffer & vertexBuffer, IndexBuffer & indexBuffer)
{
    if (!node)
    {
        return;
    }

    vertexBuffer.clear();
    indexBuffer.clear();

    GenerateVertexIndices(node, vertexBuffer);
    ContourCellProc(node, indexBuffer);
}

void GenerateVertexIndices(OctreeNode* node, VertexBuffer& vertexBuffer)
{
    if (!node)
    {
        return;
    }

    if (node->type != OctreeNodeType::Leaf)
    {
        for (int i = 0; i < 8; i++)
        {
            GenerateVertexIndices(node->children[i], vertexBuffer);
        }
    }

    if (node->type != OctreeNodeType::Internal)
    {
        OctreeDrawInfo* d = node->drawInfo;
        if (!d)
        {
            printf("Error! Could not add vertex!\n");
            exit(EXIT_FAILURE);
        }
        d->index = static_cast<int>(vertexBuffer.size());
        vertexBuffer.push_back(MeshVertex(d->position, d->averageNormal));
    }
}

void ContourProcessEdge(OctreeNode* node[4], int dir, IndexBuffer& indexBuffer)
{
    int minSize = 1000000;		// arbitrary big number
    int minIndex = 0;
    int indices[4] = { -1, -1, -1, -1 };
    bool flip = false;
    bool signChange[4] = { false, false, false, false };

    for (int i = 0; i < 4; i++)
    {
        const int edge = processEdgeMask[dir][i];
        const int c1 = edgevmap[edge][0];
        const int c2 = edgevmap[edge][1];

        const int m1 = (node[i]->drawInfo->corners >> c1) & 1;
        const int m2 = (node[i]->drawInfo->corners >> c2) & 1;

        if (node[i]->size < minSize)
        {
            minSize = node[i]->size;
            minIndex = i;
            flip = m1 != MaterialAir;
        }

        indices[i] = node[i]->drawInfo->index;

        signChange[i] =
            (m1 == MaterialAir && m2 != MaterialAir) ||
            (m1 != MaterialAir && m2 == MaterialAir);
    }

    if (signChange[minIndex])
    {
        if (!flip)
        {
            indexBuffer.push_back(indices[0]);
            indexBuffer.push_back(indices[1]);
            indexBuffer.push_back(indices[3]);

            indexBuffer.push_back(indices[0]);
            indexBuffer.push_back(indices[3]);
            indexBuffer.push_back(indices[2]);
        }
        else
        {
            indexBuffer.push_back(indices[0]);
            indexBuffer.push_back(indices[3]);
            indexBuffer.push_back(indices[1]);

            indexBuffer.push_back(indices[0]);
            indexBuffer.push_back(indices[2]);
            indexBuffer.push_back(indices[3]);
        }
    }
}

void ContourEdgeProc(OctreeNode* node[4], int dir, IndexBuffer& indexBuffer)
{
    if (!node[0] || !node[1] || !node[2] || !node[3])
    {
        return;
    }

    if (node[0]->type != OctreeNodeType::Internal &&
        node[1]->type != OctreeNodeType::Internal &&
        node[2]->type != OctreeNodeType::Internal &&
        node[3]->type != OctreeNodeType::Internal)
    {
        ContourProcessEdge(node, dir, indexBuffer);
    }
    else
    {
        for (int i = 0; i < 2; i++)
        {
            OctreeNode* edgeNodes[4];
            const int c[4] =
            {
                edgeProcEdgeMask[dir][i][0],
                edgeProcEdgeMask[dir][i][1],
                edgeProcEdgeMask[dir][i][2],
                edgeProcEdgeMask[dir][i][3],
            };

            for (int j = 0; j < 4; j++)
            {
                if (node[j]->type == OctreeNodeType::Leaf || node[j]->type == OctreeNodeType::Pseudo)
                {
                    edgeNodes[j] = node[j];
                }
                else
                {
                    edgeNodes[j] = node[j]->children[c[j]];
                }
            }

            ContourEdgeProc(edgeNodes, edgeProcEdgeMask[dir][i][4], indexBuffer);
        }
    }
}

void ContourFaceProc(OctreeNode* node[2], int dir, IndexBuffer& indexBuffer)
{
    if (!node[0] || !node[1])
    {
        return;
    }

    if (node[0]->type == OctreeNodeType::Internal ||
        node[1]->type == OctreeNodeType::Internal)
    {
        for (int i = 0; i < 4; i++)
        {
            OctreeNode* faceNodes[2];
            const int c[2] =
            {
                faceProcFaceMask[dir][i][0],
                faceProcFaceMask[dir][i][1],
            };

            for (int j = 0; j < 2; j++)
            {
                if (node[j]->type != OctreeNodeType::Internal)
                {
                    faceNodes[j] = node[j];
                }
                else
                {
                    faceNodes[j] = node[j]->children[c[j]];
                }
            }

            ContourFaceProc(faceNodes, faceProcFaceMask[dir][i][2], indexBuffer);
        }

        const int orders[2][4] =
        {
            { 0, 0, 1, 1 },
            { 0, 1, 0, 1 },
        };
        for (int i = 0; i < 4; i++)
        {
            OctreeNode* edgeNodes[4];
            const int c[4] =
            {
                faceProcEdgeMask[dir][i][1],
                faceProcEdgeMask[dir][i][2],
                faceProcEdgeMask[dir][i][3],
                faceProcEdgeMask[dir][i][4],
            };

            const int* order = orders[faceProcEdgeMask[dir][i][0]];
            for (int j = 0; j < 4; j++)
            {
                if (node[order[j]]->type == OctreeNodeType::Leaf ||
                    node[order[j]]->type == OctreeNodeType::Pseudo)
                {
                    edgeNodes[j] = node[order[j]];
                }
                else
                {
                    edgeNodes[j] = node[order[j]]->children[c[j]];
                }
            }

            ContourEdgeProc(edgeNodes, faceProcEdgeMask[dir][i][5], indexBuffer);
        }
    }
}

void ContourCellProc(OctreeNode* node, IndexBuffer& indexBuffer)
{
    if (!node)
    {
        return;
    }

    if (node->type == OctreeNodeType::Internal)
    {
        for (int i = 0; i < 8; i++)
        {
            ContourCellProc(node->children[i], indexBuffer);
        }

        for (int i = 0; i < 12; i++)
        {
            OctreeNode* faceNodes[2];
            const int c[2] = { cellProcFaceMask[i][0], cellProcFaceMask[i][1] };

            faceNodes[0] = node->children[c[0]];
            faceNodes[1] = node->children[c[1]];

            ContourFaceProc(faceNodes, cellProcFaceMask[i][2], indexBuffer);
        }

        for (int i = 0; i < 6; i++)
        {
            OctreeNode* edgeNodes[4];
            const int c[4] =
            {
                cellProcEdgeMask[i][0],
                cellProcEdgeMask[i][1],
                cellProcEdgeMask[i][2],
                cellProcEdgeMask[i][3],
            };

            for (int j = 0; j < 4; j++)
            {
                edgeNodes[j] = node->children[c[j]];
            }

            ContourEdgeProc(edgeNodes, cellProcEdgeMask[i][4], indexBuffer);
        }
    }
}

// Inefficent octree construction (I don't know how to do it efficently, that's for another time)
OctreeNode* ConstructOctreeNodes(OctreeNode* node)
{
    if (!node)
    {
        return nullptr;
    }

    if (node->size == 1)
    {
        return ConstructLeaf(node);
    }

    const int childSize = node->size / 2;
    bool hasChildren = false;

    for (int i = 0; i < 8; i++)
    {
        OctreeNode* child = new OctreeNode;
        child->size = childSize;
        child->min = node->min + (ChildMinOffsets[i] * childSize);
        child->type = OctreeNodeType::Internal;

        node->children[i] = ConstructOctreeNodes(child);
        //hasChildren = hasChildren || (node->children[i] != nullptr);
        hasChildren |= (node->children[i] != nullptr);
    }

    if (!hasChildren)
    {
        delete node;
        return nullptr;
    }

    return node;
}

// The Surface Normal can be found by finding the derivative of the density function, 
// The derivative can be approximated using the Finite Difference method
vec3 CalculateSurfaceNormal(const vec3& p)
{
    const float H = 0.001f;
    const float dx = DensityFunc(p + vec3(H,    0.0f, 0.0f)) - DensityFunc(p - vec3(H,    0.0f, 0.0f));
    const float dy = DensityFunc(p + vec3(0.0f, H,    0.0f)) - DensityFunc(p - vec3(0.0f, H,    0.0f));
    const float dz = DensityFunc(p + vec3(0.0f, 0.0f, H   )) - DensityFunc(p - vec3(0.0f, 0.0f, H   ));

    return glm::normalize(vec3(dx, dy, dz));
}

//OctreeNode* ConstructLeaf(OctreeNode* leaf)
//{
//    if (!leaf || leaf->size != 1)
//    {
//        return nullptr;
//    }
//
//    int corners = 0; // Could use a char or maybe a bitset for this?
//    for (int i = 0; i < 8; i++)
//    {
//        const ivec3 cornerPos = leaf->min + ChildMinOffsets[i];
//        const float density = DensityFunc(vec3(cornerPos));
//        const int material = density < 0.0f ? MaterialSolid : MaterialAir;
//        corners |= (material << i); // Set bits 0...7 on off depending on whether they are solid or not
//    }
//
//    if (corners == 0 || corners == 255)
//    {
//        // Voxel is full inside or outside the voxel
//        // Since this experiment is only looking for the surface and doesn't care about whats under it this is fine
//        delete leaf;
//        return nullptr;
//    }
//
//    const int MaxCrossings = 6;
//    vec3 averageNormal(0.0f);
//    int edgeCount = 0;
//    svd::QefSolver qef;
//
//    for (int i = 0; i < 12 && edgeCount < MaxCrossings; i++)
//    {
//        const int c1 = edgevmap[i][0];
//        const int c2 = edgevmap[i][1];
//
//        const int m1 = (corners >> c1) & 1;
//        const int m2 = (corners >> c2) & 1;
//
//        if ((m1 == MaterialAir && m2 == MaterialAir)
//         || (m1 == MaterialSolid && m2 == MaterialSolid))
//        {
//            // No zero crossing on this edge
//            continue;
//        }
//
//        const vec3 p1 = vec3(leaf->min + ChildMinOffsets[c1]);
//        const vec3 p2 = vec3(leaf->min + ChildMinOffsets[c2]);
//        const vec3 p = ApproximateZeroCrossingPosition(p1, p2);
//        const vec3 n = CalculateSurfaceNormal(p);
//        qef.add(p.x, p.y, p.z, n.x, n.y, n.z);
//
//        averageNormal += n;
//
//        edgeCount++;
//    }
//
//    svd::Vec3 qefPosition;
//    qef.solve(qefPosition, QefError, QefSweeps, QefError);
//
//    OctreeDrawInfo* drawInfo = new OctreeDrawInfo;
//    drawInfo->position = vec3(qefPosition.x, qefPosition.y, qefPosition.z);
//    drawInfo->qef = qef.getData();
//    
//    const vec3 min = vec3(leaf->min);
//    const vec3 max = vec3(leaf->min + ivec3(leaf->size));
//    if (drawInfo->position.x < min.x || drawInfo->position.x > max.x
//     || drawInfo->position.y < min.y || drawInfo->position.y > max.y
//     || drawInfo->position.z < min.z || drawInfo->position.z > max.z) // This stuff is basically black magic
//    {
//        const auto& mp = qef.getMassPoint();
//        drawInfo->position = vec3(mp.x, mp.y, mp.z);
//    }
//
//    drawInfo->averageNormal = glm::normalize(averageNormal / static_cast<float>(edgeCount));
//    drawInfo->corners = corners;
//
//    leaf->type = OctreeNodeType::Leaf;
//    leaf->drawInfo = drawInfo;
//    
//    return leaf;
//}

OctreeNode* ConstructLeaf(OctreeNode* leaf)
{
    if (!leaf || leaf->size != 1)
    {
        return nullptr;
    }

    int corners = 0;
    for (int i = 0; i < 8; i++)
    {
        const ivec3 cornerPos = leaf->min + ChildMinOffsets[i];
        const float density = DensityFunc(vec3(cornerPos));
        const int material = density < 0.f ? MaterialSolid : MaterialAir;
        corners |= (material << i);
    }

    if (corners == 0 || corners == 255)
    {
        // voxel is full inside or outside the volume
        delete leaf;
        return nullptr;
    }

    // otherwise the voxel contains the surface, so find the edge intersections
    const int MAX_CROSSINGS = 6;
    int edgeCount = 0;
    vec3 averageNormal(0.f);
    svd::QefSolver qef;

    for (int i = 0; i < 12 && edgeCount < MAX_CROSSINGS; i++)
    {
        const int c1 = edgevmap[i][0];
        const int c2 = edgevmap[i][1];

        const int m1 = (corners >> c1) & 1;
        const int m2 = (corners >> c2) & 1;

        if ((m1 == MaterialAir && m2 == MaterialAir) ||
            (m1 == MaterialSolid && m2 == MaterialSolid))
        {
            // no zero crossing on this edge
            continue;
        }

        const vec3 p1 = vec3(leaf->min + ChildMinOffsets[c1]);
        const vec3 p2 = vec3(leaf->min + ChildMinOffsets[c2]);
        const vec3 p = ApproximateZeroCrossingPosition(p1, p2);
        const vec3 n = CalculateSurfaceNormal(p);
        qef.add(p.x, p.y, p.z, n.x, n.y, n.z);

        averageNormal += n;

        edgeCount++;
    }

    svd::Vec3 qefPosition;
    qef.solve(qefPosition, QefError, QefSweeps, QefError);

    OctreeDrawInfo* drawInfo = new OctreeDrawInfo;
    drawInfo->position = vec3(qefPosition.x, qefPosition.y, qefPosition.z);
    drawInfo->qef = qef.getData();

    const vec3 min = vec3(leaf->min);
    const vec3 max = vec3(leaf->min + ivec3(leaf->size));
    if (drawInfo->position.x < min.x || drawInfo->position.x > max.x ||
        drawInfo->position.y < min.y || drawInfo->position.y > max.y ||
        drawInfo->position.z < min.z || drawInfo->position.z > max.z)
    {
        const auto& mp = qef.getMassPoint();
        drawInfo->position = vec3(mp.x, mp.y, mp.z);
    }

    drawInfo->averageNormal = glm::normalize(averageNormal / (float)edgeCount);
    drawInfo->corners = corners;

    leaf->type = OctreeNodeType::Leaf;
    leaf->drawInfo = drawInfo;

    return leaf;
}

vec3 ApproximateZeroCrossingPosition(const vec3& p0, const vec3& p1)
{
    // Approximate the zero crossing by finding the min value along the edge
    float minValue = 100000.0f;
    float t = 0.0f;
    float currentT = 0.0f;
    const int steps = 8;
    const float increment = 1.0f / static_cast<float>(steps);

    while (currentT <= 1.0f)
    {
        const vec3 p = p0 + ((p1 - p0) * currentT);
        const float density = glm::abs(DensityFunc(p));
        if (density < minValue)
        {
            minValue = density;
            t = currentT;
        }
        currentT += increment;
    }

    return p0 + ((p1 - p0)*t);
}
