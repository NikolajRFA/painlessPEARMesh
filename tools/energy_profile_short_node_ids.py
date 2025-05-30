def get_energy_profile_id(node_id: int) -> int:
    energy_profile_id_map = {1: 0,
                             3: 1,
                             4: 1,
                             5: 2,
                             6: 2,
                             7: 2,
                             8: 2,
                             9: 3,
                             10: 3,
                             11: 3,
                             12: 3,
                             13: 4,
                             14: 4,
                             15: 4,
                             16: 4,
                             17: 5,
                             18: 3,
                             19: 4,
                             20: 5,
                             21: 3,
                             22: 2,
                             23: 3,
                             24: 4,
                             25: 4,
                             26: 5,
                             27: 1,
                             28: 2,
                             29: 1}

    return energy_profile_id_map[node_id]


def get_energy_profile_from_node(node_id: int):
    return get_energy_profile(get_energy_profile_id(node_id))


def get_energy_profile(energy_profile_id: int):
    energy_profile_map = {
        0: (999, 999),
        1: (35, 30),
        2: (30, 25),
        3: (25, 20),
        4: (20, 15),
        5: (15, 10)
    }

    return energy_profile_map[energy_profile_id]


def get_energy_profile_id_H6L13(node_id: int) -> int:
    energy_profile_id_map = {
        3: 1,
        4: 1,
        5: 1,
        9: 1,
        27: 1,
        8: 1,
        6: 2,
        7: 2,
        10: 2,
        11: 2,
        12: 2,
        13: 2,
        14: 2,
        15: 2,
        16: 2,
        17: 2,
        18: 2,
        19: 2,
        20: 2,
        21: 2,
        22: 2,
        23: 2,
        24: 2,
        25: 2,
        26: 2,
        28: 2,
        29: 2,
    }

    return energy_profile_id_map[node_id]

def get_energy_profile_H6L13(energy_profile_id: int):
    energy_profile_map = {
        1: (60, 45),
        2: (20, 15),
    }

    return energy_profile_map[energy_profile_id]

def get_energy_profile_from_node_H6L13(node_id: int):
    return get_energy_profile_H6L13(get_energy_profile_id_H6L13(node_id))