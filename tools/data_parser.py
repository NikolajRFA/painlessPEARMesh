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
                 parent_candidates: list[int]):
        self.from_node = from_node
        self.tx_period = tx_period
        self.rx_period = rx_period
        self.parent_candidates = parent_candidates

    def __str__(self):
        return f"From: {self.from_node}, Tx: {self.tx_period}, Rx: {self.rx_period}, Parents: {self.parent_candidates}"


def extract_pear_reports(file_path):
    nodes: list[int] = []
    pear_reports = []
    first_connected_node = 0
    seen_nodes = []
    ROOT_NODE_ID = 3206773453
    with open(file_path, 'r', encoding="utf-16") as file:
        collect = False
        for line in file:
            if line.startswith("Number of nodes in mesh: 20"):
                collect = True
            if collect and line.startswith("TOPOLOGY: "):
                matches = re.findall(r'(?<=nodeId":)\d+', line)
                for match in matches:
                    if match == ROOT_NODE_ID:
                        continue
                    nodes.append(int(match))
                break
        assert len(nodes) == 20

    with open(file_path, 'r', encoding="utf-16") as file:
        for line in file:
            if first_connected_node == 0 and line.startswith("--> startHere: New Connection, nodeId = "):
                first_connected_node = int(line.split("=")[1].strip())

            if line.startswith("DATA: "):
                line_without_data = line.replace("DATA: ", "").strip()
                from_node = line_without_data.split(";")[0].strip()
                json_data = json.loads(line_without_data.split(";")[1].strip())
                pear_report = PearReport(int(from_node), json_data['txPeriod'], json_data['rxPeriod'],
                                         json_data['parentCandidates'])
                pear_reports.append(pear_report)
                if pear_report.from_node == first_connected_node and seen_nodes.__len__() != 0:
                    # impute nulls on unseen nodes
                    for node in nodes:
                        if not seen_nodes.__contains__(node):
                            pear_reports.append(PearReport(node, None, None, []))

                    seen_nodes.clear()
                    seen_nodes.append(pear_report.from_node)

                if not seen_nodes.__contains__(pear_report.from_node):
                    seen_nodes.append(pear_report.from_node)

    return pear_reports


def df_from_pear_reports(pear_reports: list[PearReport], run_id: int = 1):
    df = pd.DataFrame([vars(pear_report) for pear_report in pear_reports])

    # Assuming `df` is your DataFrame containing PearReport data
    df["node_time"] = 0  # Initialize node_time column

    # Dictionary to track the last seen time for each `from_node`
    node_timestamps = {}

    # Iterate through the DataFrame and assign node_time values
    for index, row in df.iterrows():
        from_node = row["from_node"]

        if from_node in node_timestamps:  # Seen before, increment time
            node_timestamps[from_node] += 60 * 1_000_000 + 1000  # Add 60 seconds in microseconds
        else:  # First occurrence, set time to 0
            node_timestamps[from_node] = 1000

        df.at[index, "node_time"] = node_timestamps[from_node]  # Assign interpolated node_time

    # Display the updated DataFrame
    #print(df.head())

    df['node_time'] = df['node_time'].replace(0, np.nan)  # Convert 0s to NaN temporarily
    df['node_time'] = df['node_time'].interpolate()

    # Add the run_id column with the supplied integer
    df["run_id"] = run_id

    return df


#df = df_from_pear_reports(extract_pear_reports("data/1st set/20_1_1405_1.txt"), 1)

#print(df.info())
