import os

SCENEBUILDER_DIR = os.path.dirname(os.path.abspath(__file__))

PROJECT_ROOT = os.path.dirname(SCENEBUILDER_DIR)

MODELS_DIR = os.path.join(PROJECT_ROOT, "models")
SCENES_DIR = os.path.join(PROJECT_ROOT, "scenes")

materials = []
sources = []
instances = []
spheres = []

def add_material(r, g, b, visibility, roughness, metallic, emission, opacity):
    if visibility > 0.5:
        visibility = 1.0
    else:
        visibility = 0.0

    materials.append((r, g, b, visibility, roughness, metallic, emission, opacity))

    return len(materials) - 1

def add_source(model_filename):
    full_path = os.path.join(MODELS_DIR, model_filename)

    if not os.path.isfile(full_path):
        raise FileNotFoundError(f"Model not found: {full_path}")

    relative_path = os.path.relpath(full_path, PROJECT_ROOT)
    relative_path = relative_path.replace("\\", "/")

    if relative_path not in sources:
        sources.append(relative_path)

    return sources.index(relative_path)

def add_instance(pos, scale, rotation, material_index, source_index):
    instances.append((pos, scale, rotation, material_index, source_index))

def add_sphere(pos, radius, material_index):
    spheres.append((pos, radius, material_index))

def write_scene(scene_name):
    os.makedirs(SCENES_DIR, exist_ok=True)
    output_path = os.path.join(SCENES_DIR, scene_name)

    with open(output_path, "w") as f:
        f.write(f"{len(materials)}\n")
        for m in materials:
            f.write(" ".join(f"{v:.6f}" for v in m) + "\n")

        f.write("\n")

        f.write(f"{len(sources)}\n")
        for s in sources:
            f.write(s + "\n")

        f.write("\n")

        f.write(f"{len(instances)}\n")
        for inst in instances:
            pos, scale, rot, mat_idx, src_idx = inst
            f.write(
                f"{pos[0]} {pos[1]} {pos[2]}   "
                f"{scale[0]} {scale[1]} {scale[2]}   "
                f"{rot[0]} {rot[1]} {rot[2]}   "
                f"{mat_idx} {src_idx}\n"
            )
        f.write("\n")

        f.write(f"{len(spheres)}\n")
        for s in spheres:
            pos, rad, mat_idx = s
            f.write(
                f"{pos[0]} {pos[1]} {pos[2]} "
                f"{rad} {mat_idx}\n"
            )

    print(f"Scene written to: {output_path}")