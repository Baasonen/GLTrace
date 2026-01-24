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
    1.0,
    0.0,
    0.0,
    10.0,
    1.0
)


emissiveG = add_material(
    0.0, 1.0, 0.0,
    1.0,
    0.0,
    0.0,
    10.0,
    1.0
)

emissiveB = add_material(
    0.0, 0.0, 1.0,
    1.0,
    0.0,
    0.0,
    10.0,
    1.0
)

emissiveSoftW = add_material(
    1.0, 1.0, 1.0,
    1.0,
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

dragon = add_source("dragon.obj")
alfa = add_source("alfa147.obj")

add_instance(
    pos=(0, -40, -400),
    scale=(2, 2, 2),
    rotation=(0, 0, 0),
    material_index=gold,
    source_index=alfa
)

add_instance(
    pos=(0, 0, 0),
    scale=(1, 1, 1),
    rotation=(0, 0, 0),
    material_index=gold,
    source_index=dragon
)

add_instance(
    pos=(220, 0, 0),
    scale=(1, 1, 1),
    rotation=(0, 45, 0),
    material_index=matte,
    source_index=dragon
)

add_instance(
    pos=(-220, 0, 0),
    scale=(0.5, 0.5, 0.5),
    rotation=(0, 315, 0),
    material_index=emissiveSoftW,
    source_index=dragon
)

add_sphere(
    pos=(0.0, 100.0, 0.0),
    radius=(20.0),
    material_index=emissiveR
)

add_sphere(
    pos=(140.0, 30.0, 0.0),
    radius=(15.0),
    material_index=emissiveG
)

add_sphere(
    pos=(50.0, 100.0, -120.0),
    radius=(15.0),
    material_index=emissiveB
)

add_sphere(
    pos=(0.0, -10040.0, 0.0),
    radius=(10000.0),
    material_index=matteW
)

add_sphere(
    pos=(-40., 0.0, 300.0),
    radius=(100.0),
    material_index=mirror
)


write_scene("1.scene")
