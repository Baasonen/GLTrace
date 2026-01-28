from scene_writer import *

gold = add_material(
    1.0, 0.8, 0.6, #Color
    0.0, #Invisibility
    0.2, #Roughness
    1.0, #Metallic
    0.0, #Emission
    1.0  #Opacity
)

mirror = add_material(
    1.0, 1.0, 1.0,
    0.0,
    0.0,
    1.0,
    0.0,
    1.0
)

matte = add_material(
    0.2, 0.2, 0.25,
    0.0,
    0.9,
    0.0,
    0.0,
    1.0
)

emissiveR = add_material(
    1.0, 0.0, 0.0,
    0.0,
    0.0,
    0.0,
    12.0,
    1.0
)


emissiveG = add_material(
    0.0, 1.0, 0.0,
    0.0,
    0.0,
    0.0,
    12.0,
    1.0
)

emissiveB = add_material(
    0.0, 0.0, 1.0,
    0.0,
    0.0,
    0.0,
    12.0,
    1.0
)

emissiveW = add_material(
    1.0, 1.0, 1.0,
    0.0,
    0.0,
    0.0,
    20.0,
    1.0
)

emissiveSoftW = add_material(
    1.0, 1.0, 1.0,
    0.0,
    0.0,
    0.0,
    2.0,
    1.0
)

matteW = add_material(
    1.0, 1.0, 1.0,
    0.0,
    0.9,
    0.0,
    0.0,
    1.0
)

room = add_source("city.obj")

add_instance(
    pos=(0, 0, 0),
    scale=(4, 4, 4),
    rotation=(0, 0, 0),
    material_index=matteW,
    source_index=room
)



write_scene("city_ns.scene")
