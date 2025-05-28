def get_energy_profile_id(node_id: int) -> int:
    energy_profile_id_map = {3206773453: 0,
                             3211408993: 1,
                             3211386233: 1,
                             417643029: 2,
                             2930999053: 2,
                             2811362457: 2,
                             3211302717: 2,
                             3206780693: 3,
                             3211321933: 3,
                             3211320821: 3,
                             3101225193: 3,
                             1616231985: 4,
                             3101224829: 4,
                             3294674857: 4,
                             2883624417: 4,
                             2883613101: 5,
                             3294675137: 3,
                             2883639777: 4,
                             3294679677: 5,
                             3294675181: 3,
                             2883681453: 2,
                             2883619065: 3,
                             3294677077: 4,
                             2883632169: 4,
                             3294679889: 5,
                             2883627821: 1,
                             2883641793: 2,
                             3294680957: 1}

    return energy_profile_id_map[node_id]


def get_energy_profile_from_node(node_id: int):
    return get_energy_profile(get_energy_profile_id(node_id))


def get_energy_profile(energy_profile_id: int):
    energy_profile_map = {
        0: (999, 999),
        1: (25, 20),
        2: (20, 15),
        3: (15, 10),
        4: (10, 5),
        5: (5, 0)
    }

    return energy_profile_map[energy_profile_id]


def get_energy_profile_id_H6L13(node_id: int) -> int:
    energy_profile_id_map = {
        3211408993: 1,
        3211386233: 1,
        417643029: 1,
        3206780693: 1,
        2883627821: 1,
        3211302717: 1,
        2930999053: 2,
        2811362457: 2,
        3211321933: 2,
        3211320821: 2,
        3101225193: 2,
        1616231985: 2,
        3101224829: 2,
        3294674857: 2,
        2883624417: 2,
        2883613101: 2,
        3294675137: 2,
        2883639777: 2,
        3294679677: 2,
        3294675181: 2,
        2883681453: 2,
        2883619065: 2,
        3294677077: 2,
        2883632169: 2,
        3294679889: 2,
        2883641793: 2,
        3294680957: 2,
    }

    return energy_profile_id_map[node_id]

def get_energy_f_profile_H6L13(energy_profile_id: int):
    energy_profile_map = {
        1: (60, 45),
        2: (20, 15),
    }

    return energy_profile_map[energy_profile_id]

def get_energy_profile_from_node_H6L13(node_id: int):
    return get_energy_f_profile_H6L13(get_energy_profile_id_H6L13(node_id))