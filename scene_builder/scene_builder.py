from scene_writer import *

gold = add_material(
    1.0, 0.8, 0.6,
    1.0,
    0.2,
    1.0,
    0.0,
    1.0
)

matte = add_material(
    0.2, 0.2, 0.25,
    1.0,
    0.9,
    0.0,
    0.0,
    1.0
)

dragon = add_source("dragon.obj")

add_instance(
    pos=(0, 0, 0),
    scale=(1, 1, 1),
    rotation=(0, 0, 0),
    material_index=gold,
    source_index=dragon
)

add_instance(
    pos=(2, 0, 0),
    scale=(1, 1, 1),
    rotation=(0, 45, 0),
    material_index=matte,
    source_index=dragon
)

write_scene("test.scene")
