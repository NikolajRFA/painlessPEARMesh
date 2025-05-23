import json
from typing import Optional
import numpy as np
import pandas as pd
import re


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


def df_from_pear_reports(pear_reports: list[PearReport], run_id: int = 1):
    df = pd.DataFrame([vars(pear_report) for pear_report in pear_reports])
    df['node_time'] = df['node_time'].replace(0, np.nan)  # Convert 0s to NaN temporarily
    df['node_time'] = df['node_time'].interpolate()

    # Add the run_id column with the supplied integer
    df["run_id"] = run_id

    return df

df = df_from_pear_reports(extract_pear_reports("data/20_4_1505_1.txt")[0])

f = open("csv.csv", "w")
f.write(df.to_csv())

