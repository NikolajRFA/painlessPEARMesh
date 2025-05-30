import pandas as pd


def get_file_paths_from_folder(folder_path):
    import os
    file_paths = []
    for root, directories, files in os.walk(folder_path):
        for filename in files:
            filepath = os.path.join(root, filename)
            file_paths.append(filepath)
    return file_paths

def get_short_node_id(node_id: int) -> int:
    df = pd.read_excel('data/ESP32 MAC addresses.xlsx')
    esp_map = dict(zip(df['NodeId'], df['ESP']))

    return esp_map[node_id]

#print(get_short_node_id(3294680957))