#define CGLTF_WRITE_IMPLEMENTATION
#define PAR_OCTASPHERE_IMPLEMENTATION

#include "cgltf_write.h"
#include "par_octasphere.h"

#define countof(a) (sizeof(a) / sizeof(a[0]))

typedef float Vertex[3];

static par_octasphere_mesh generate_mesh(float corner_radius, int geometric_lod) {
    uint32_t num_indices;
    uint32_t num_vertices;
    par_octasphere_config config = {
        .corner_radius = corner_radius,
        .width = 1.0f,
        .height = 1.0f,
        .depth = 1.0f,
        .num_subdivisions = geometric_lod,
    };
    par_octasphere_get_counts(&config, &num_indices, &num_vertices);

    const int vbufsize = sizeof(Vertex) * num_vertices;
    const int ibufsize = sizeof(uint16_t) * num_indices;
    const int vbufsize3 = 3 * vbufsize;
    const int ibufsize3 = 3 * ibufsize;

    uint8_t* buffer = malloc(vbufsize3 + vbufsize3 + ibufsize3);
    uint8_t* posptr = buffer;
    uint8_t* nrmptr = buffer + vbufsize3;
    uint8_t* idxptr = buffer + vbufsize3 + vbufsize3;

    par_octasphere_mesh mesh = {};

    mesh.positions = (float*) posptr;
    mesh.normals = (float*) nrmptr;
    mesh.indices = (uint16_t*) idxptr;
    config.width = 3;
    par_octasphere_populate(&config, &mesh);
    config.width = 1;

    posptr += vbufsize;
    nrmptr += vbufsize;
    idxptr += ibufsize;

    mesh.positions = (float*) posptr;
    mesh.normals = (float*) nrmptr;
    mesh.indices = (uint16_t*) idxptr;
    config.height = 3;
    par_octasphere_populate(&config, &mesh);
    config.height = 1;

    posptr += vbufsize;
    nrmptr += vbufsize;
    idxptr += ibufsize;

    mesh.positions = (float*) posptr;
    mesh.normals = (float*) nrmptr;
    mesh.indices = (uint16_t*) idxptr;
    config.depth = 3;
    par_octasphere_populate(&config, &mesh);
    config.depth = 1;


    posptr = buffer;
    nrmptr = buffer + vbufsize3;
    idxptr = buffer + vbufsize3 + vbufsize3;
    mesh.positions = (float*) posptr;
    mesh.normals = (float*) nrmptr;
    mesh.indices = (uint16_t*) idxptr;
    mesh.num_indices *= 3;
    mesh.num_vertices *= 3;

    for (int i = num_indices * 1; i < num_indices * 2; i++) {
        mesh.indices[i] += num_vertices * 1;
    }
    for (int i = num_indices * 2; i < num_indices * 3; i++) {
        mesh.indices[i] += num_vertices * 2;
    }

    return mesh;
}

static void write_gltf(par_octasphere_mesh source, const char* gltf_path, const char* bin_path) {

    const int vbufsize = source.num_vertices * sizeof(Vertex);
    const int ibufsize = source.num_indices * sizeof(uint16_t);

    cgltf_buffer buffers[1] = {};

    buffers[0].uri = (char*) bin_path;
    buffers[0].size = vbufsize + vbufsize + ibufsize;

    cgltf_buffer_view buffer_views[3] = {};

    buffer_views[0].buffer = &buffers[0];
    buffer_views[0].size = vbufsize;
    buffer_views[0].offset = 0;
    buffer_views[0].type = cgltf_buffer_view_type_vertices;

    buffer_views[1].buffer = &buffers[0];
    buffer_views[1].size = vbufsize;
    buffer_views[1].offset = vbufsize;
    buffer_views[1].type = cgltf_buffer_view_type_vertices;

    buffer_views[2].buffer = &buffers[0];
    buffer_views[2].size = ibufsize;
    buffer_views[2].offset = vbufsize + vbufsize;
    buffer_views[2].type = cgltf_buffer_view_type_indices;

    cgltf_accessor accessors[3] = {};

    accessors[0].buffer_view = &buffer_views[0];
    accessors[0].component_type = cgltf_component_type_r_32f;
    accessors[0].type = cgltf_type_vec3;
    accessors[0].count = source.num_vertices;
    accessors[0].has_min = accessors[0].has_max = true;
    accessors[0].min[0] = accessors[0].min[1] = accessors[0].min[2] = -3.0f;
    accessors[0].max[0] = accessors[0].max[1] = accessors[0].max[2] = +3.0f;

    accessors[1].buffer_view = &buffer_views[1];
    accessors[1].component_type = cgltf_component_type_r_32f;
    accessors[1].type = cgltf_type_vec3;
    accessors[1].count = source.num_vertices;

    accessors[2].buffer_view = &buffer_views[2];
    accessors[2].component_type = cgltf_component_type_r_16u;
    accessors[2].type = cgltf_type_scalar;
    accessors[2].count = source.num_indices;

    cgltf_attribute attributes[2] = {};

    attributes[0].name = (char*)"POSITION";
    attributes[0].data = &accessors[0];
    attributes[0].type = cgltf_attribute_type_position;

    attributes[1].name = (char*)"NORMAL";
    attributes[1].data = &accessors[1];
    attributes[1].type = cgltf_attribute_type_normal;

    cgltf_primitive prims[1] = {};

    prims[0].type = cgltf_primitive_type_triangles;
    prims[0].attributes = attributes;
    prims[0].attributes_count = countof(attributes);
    prims[0].indices = &accessors[2];

    cgltf_mesh meshes[1] = {};

    meshes[0].name = "octasphere";
    meshes[0].primitives = prims;
    meshes[0].primitives_count = countof(prims);

    cgltf_node nodes[1] = {};

    nodes[0].name = "root";
    nodes[0].mesh = &meshes[0];

    cgltf_node* pnodes[1] = { &nodes[0] };
    cgltf_scene scenes[1] = {};

    scenes[0].nodes = pnodes;
    scenes[0].nodes_count = countof(pnodes);

    cgltf_data asset = {
        .asset.generator = "https://github.com/prideout/normal-map-test",
        .asset.version = "2.0",
        .buffers = buffers,
        .buffers_count = countof(buffers),
        .buffer_views_count = countof(buffer_views),
        .buffer_views = buffer_views,
        .accessors_count = countof(accessors),
        .accessors = accessors,
        .meshes = meshes,
        .meshes_count = countof(meshes),
        .nodes = nodes,
        .nodes_count = countof(nodes),
        .scenes = scenes,
        .scenes_count = countof(scenes)
    };

    cgltf_options options = {};
    cgltf_write_file(&options, gltf_path, &asset);

    FILE* file = fopen(bin_path, "wb");
    fwrite(source.positions, sizeof(Vertex), source.num_vertices, file);
    fwrite(source.normals, sizeof(Vertex), source.num_vertices, file);
    fwrite(source.indices, sizeof(uint16_t), source.num_indices, file);
    fclose(file);
}

int main(int argc, char** argv) {
    par_octasphere_mesh himesh = generate_mesh(0.1, 4);
    write_gltf(himesh, "test_hi.gltf", "test_hi.bin");
    printf("test_hi.gltf has %d verts\n", himesh.num_vertices);

    par_octasphere_mesh lomesh = generate_mesh(0.1, 1);
    write_gltf(lomesh, "test_lo.gltf", "test_lo.bin");
    printf("test_lo.gltf has %d verts\n", lomesh.num_vertices);
}
