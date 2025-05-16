import json
from typing import Optional
import numpy as np
import pandas as pd
import re
from matplotlib import pyplot as plt


class PearReport:
    from_node: int
    tx_period: Optional[int]
    rx_period: Optional[int]
    parent_candidates: list[int]
    node_time: int


    def __init__(self, from_node: int, tx_period: Optional[int], rx_period: Optional[int],
                 parent_candidates: list[int], node_time: int):
        self.from_node = from_node
        self.tx_period = tx_period
        self.rx_period = rx_period
        self.parent_candidates = parent_candidates
        self.node_time = node_time

    def __str__(self):
        return f"From: {self.from_node}, Tx: {self.tx_period}, Rx: {self.rx_period}, Parents: {self.parent_candidates}, Time: {self.node_time}"


def extract_pear_reports(file_path):
    nodes: list[int] = []
    pear_reports = []
    pear_stable_time: Optional[int] = None
    with open(file_path, 'r', encoding="utf-16") as file:
        collect = False
        last_line = ""
        for line in file:
            if line.startswith("Number of nodes in mesh: 20"):
                collect = True
            if collect and last_line.startswith("TOPOLOGY: "):
                matches = re.findall(r'(?<=nodeId":)\d+', last_line)
                for match in matches:
                    nodes.append(int(match))
                break
            last_line = line

        assert len(nodes) == 20

    with open(file_path, 'r', encoding="utf-16") as file:
        for line in file:
            if line.startswith("DATA: "):
                line_without_data = line.replace("DATA: ", "").strip()
                split_line = line_without_data.split(";")
                from_node = split_line[0].strip()
                time_stamp = int(split_line[1].strip())
                json_data = json.loads(split_line[2].strip())
                pear_report = PearReport(int(from_node), json_data['txPeriod'], json_data['rxPeriod'],
                                         json_data['parentCandidates'], time_stamp)
                pear_reports.append(pear_report)

            if line.startswith("Time since first pear run:"):
                pear_stable_time = int(line.split(":")[1].strip())


    return pear_reports, pear_stable_time


def df_from_pear_reports(pear_reports: list[PearReport]):
    df = pd.DataFrame([vars(pear_report) for pear_report in pear_reports])
    df['node_time'] = df['node_time'].replace(0, np.nan)  # Convert 0s to NaN temporarily
    df['node_time'] = df['node_time'].interpolate()
    #df = df.astype(float)
    return df

pear_reports, pear_stable_time = extract_pear_reports("data/20_4_1505_1.txt")

df = df_from_pear_reports(pear_reports)

sorted_df = df.sort_values(by=['from_node', 'node_time'])

unique_node_ids = df['from_node'].unique()

for node_id in unique_node_ids:
    filtered_df = sorted_df[sorted_df['from_node'] == node_id]

    filtered_df = df[df['from_node'] == node_id].copy()  # Ensures it's a separate copy
    filtered_df['node_time'] = filtered_df['node_time'] / 1000

    plt.plot(filtered_df['node_time'], filtered_df['tx_period'], marker='o', linestyle='-')
    plt.axvline(x=360000, color='red', linestyle='--', label="PEAR first run")
    plt.axvline(x=pear_stable_time, color='green', linestyle='--', label="PEAR stability reached")
    plt.xticks(filtered_df['node_time'], labels=filtered_df['node_time'].round(0), rotation=45)

    plt.title(f"Node {node_id}")
    plt.xlabel("Time (ms)")
    plt.ylabel("Transmissions last 60 seconds")

    plt.grid()
    plt.legend()
    plt.tight_layout()
    plt.show()

#f = open("csv.csv", "w")
#f.write(df.to_csv())
