import json
import pandas as pd

def extract_data_jsons(file_path):
    data_jsons = []
    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith("DATA: "):
                data_json = line.replace("DATA: ", "").strip()
                data_jsons.append(json.loads(data_json))
    return data_jsons

def csv_from_json(json_data):


print(extract_data_jsons('20_2_1305_1.txt'))