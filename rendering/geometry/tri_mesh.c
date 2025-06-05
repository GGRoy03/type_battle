typedef struct
{
    bool EnableNormals;
    bool EnableUVs;
    bool EnableColors;
} tri_mesh_modifiers;

typedef struct
{
    void*  Values;
    size_t Size;
} mesh_attribute;

typedef struct
{
    mesh_attribute Positions;
    mesh_attribute Normals;
    mesh_attribute UVs;
    mesh_attribute Colors;
    mesh_attribute Indices;
} tri_mesh;

typedef enum
{
    MESH_MODIFIER_NONE = 1 << 0,

    MESH_MODIFIER_NORMAL_BIT = 1 << 1,
    MESH_MODIFIER_UV_BIT     = 1 << 2,
    MESH_MODIFIER_COLOR_BIT  = 1 << 3,
} MESH_MODIFIERS;

typedef enum
{
    VERTEX_COUNT_NONE,

    VERTEX_COUNT_TRIANGLE = 3,
} VERTEX_COUNTS;

typedef struct
{
    mesh_attribute InterleavedData;
    mesh_attribute Indices;

    u32 IndexCount;

    u32 MaterialIndex;
} static_sub_mesh;

typedef struct
{
    u8  Modifiers;
    u32 Stride;

    static_sub_mesh* SubMeshes;
    u32              SubMeshCount;
} static_tri_mesh;
