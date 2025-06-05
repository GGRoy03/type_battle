#define STATIC_ASSET_MAGIC_BYTE       0xAC
#define EXPECTED_STATIC_ASSET_VERSION 1.0f
#define MATERIAL_NAME_LENGTH          64
#define STATIC_ASSET_MAGIC_BYTE       0xAC
#define ASSET_MAGIC_BYTE              0x56
#define ASSET_VERSION                 1.0f

#pragma pack(push,1)
typedef struct
{
    u8  MagicByte;
    f32 Version;
    u64 AttributesSize;
    u64 AttributesOffset;
    u64 IndicesSize;
    u64 IndicesOffset;
    u8  HasNormal;
    u8  HasTexture;

    u64            MaterialOffset;
    LIGHTNING_TYPE LightningType;
} static_asset_header;
#pragma pack(pop)

typedef struct
{
    char           MaterialName[MATERIAL_NAME_LENGTH];
    LIGHTNING_TYPE Type;

    // Constants
    f32 Shininess;
    f32 Dissolve;
    f32 OpticalDensity;

    // Colors
    f32 Ambient[3];
    f32 Diffuse[3];
    f32 Specular[3];
    f32 Emissive[3];
} master_material;

internal u32
LoadStaticMeshFromDisk(char* RelativeDirectory,
                       platform_context* Platform,
                       transient_allocator* Allocator)
{
    if(!RelativeDirectory) return 0;

    char Directory[MAX_PATH] = {};
    snprintf(Directory, MAX_PATH, "%s/assets/%s", Platform->Root, RelativeDirectory);

    char* FileList = NULL;
    u32   FileCount = 0;
    Platform->ListDirectoryFiles(Directory, ".asset", Allocator, &FileList, &FileCount);

    static_tri_mesh Mesh = {};
    Mesh.SubMeshCount    = FileCount;
    Mesh.SubMeshes       = (static_sub_mesh*)AllocateTransient(Allocator, FileCount * sizeof(static_sub_mesh));

    u32  CommonStride = 0;
    u8   CommonMod    = MESH_MODIFIER_NONE;

    for (u32 i = 0; i < FileCount; ++i)
    {
        char* Path = FileList + (i * MAX_PATH);
        platform_read_result FB = Platform->ReadFile(Path, Allocator);
        if (!FB.Content) continue;

        static_asset_header* H = (static_asset_header*)FB.Content;
        if (H->MagicByte != ASSET_MAGIC_BYTE ||
            H->Version != ASSET_VERSION)
        {
            FreeTransient(Allocator, FB.Content, FB.ContentSize);
            continue;
        }

        if (CommonStride == 0)
        {
            CommonStride = 3 * sizeof(f32);
            if (H->HasNormal)
            {
                CommonStride += 3 * sizeof(f32);
                CommonMod |= MESH_MODIFIER_NORMAL_BIT;
            }
            if (H->HasTexture)
            {
                CommonStride += 2 * sizeof(f32);
                CommonMod |= MESH_MODIFIER_UV_BIT;
            }

            Mesh.Stride = CommonStride;
            Mesh.Modifiers = CommonMod;
        }

        static_sub_mesh* D = Mesh.SubMeshes + i;

        D->IndexCount = (u32)(H->IndicesSize / sizeof(u32));

        D->InterleavedData.Size   = (u32)H->AttributesSize;
        D->InterleavedData.Values = AllocateTransient(Allocator, H->AttributesSize);
        memcpy(D->InterleavedData.Values, (u8*)FB.Content + H->AttributesOffset,
               H->AttributesSize);

        D->Indices.Size = (u32)H->IndicesSize;
        D->Indices.Values = AllocateTransient(Allocator, H->IndicesSize);
        memcpy(D->Indices.Values, (u8*)FB.Content + H->IndicesOffset, H->IndicesSize);

        master_material* MatSrc = (master_material*)((u8*)FB.Content + H->MaterialOffset);

        switch (H->LightningType)
        {

        case LIGHTNING_COLOR_AMBIENT:
        {
            material_color_ambient Material = {0};

            Material.Shininess = MatSrc->Shininess;
            Material.Dissolve  = MatSrc->Dissolve;

            memcpy(Material.Ambient, MatSrc->Ambient, sizeof(MatSrc->Ambient));
            memcpy(Material.Diffuse, MatSrc->Diffuse, sizeof(MatSrc->Diffuse));

            D->MaterialIndex = CreateNewMaterial(&Material, sizeof(material_color_ambient));
        } break;

        case LIGHTNING_COLOR_AMBIENT_SPECULAR:
        {
            material_color_ambient_specular Material = {0};

            Material.Shininess = MatSrc->Shininess;
            Material.Dissolve  = MatSrc->Dissolve;

            memcpy(Material.Ambient , MatSrc->Ambient , sizeof(MatSrc->Ambient ));
            memcpy(Material.Diffuse , MatSrc->Diffuse , sizeof(MatSrc->Diffuse ));
            memcpy(Material.Specular, MatSrc->Specular, sizeof(MatSrc->Specular));

            D->MaterialIndex = CreateNewMaterial(&Material, sizeof(material_color_ambient_specular));
        } break;

        }

        FreeTransient(Allocator, FB.Content, FB.ContentSize);
    }

    Platform->FreeFileList(FileList, FileCount, Allocator);

    u32 MeshIndex = PushNewStaticMesh(Mesh);
    return MeshIndex;
}
