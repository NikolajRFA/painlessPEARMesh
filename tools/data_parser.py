import json
import pandas as pd

class PearReport:
    from_node: int
    tx_period: int
    rx_period: int
    parent_candidates: list[int]

def extract_pear_reports(file_path):
    data_jsons = []
    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith("DATA: "):
                data_json = line.replace("DATA: ", "").strip()
                data_jsons.append(json.loads(data_json))
    return map(lambda json: json, data_jsons)

def csv_from_json(json_data):
    df = pd.DataFrame(json_data)
    return df

print(csv_from_json(extract_data_jsons("20_2_1305_1.txt")).head())