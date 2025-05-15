import json
from typing import Optional
import pandas as pd
import re


class PearReport:
    from_node: int
    tx_period: Optional[int]
    rx_period: Optional[int]
    parent_candidates: list[int]

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


def df_from_pear_reports(pear_reports: list[PearReport]):
    cols: dict[str, list[Optional[int]]] = {}

    # Populate the dictionary
    for pear_report in pear_reports:
        if pear_report.from_node == 3206773453: continue
        tx_key = f"{pear_report.from_node}tx"
        rx_key = f"{pear_report.from_node}rx"

        if tx_key not in cols:
            cols[tx_key] = []
            cols[rx_key] = []

        cols[tx_key].append(pear_report.tx_period)
        cols[rx_key].append(pear_report.rx_period)

    # Find the maximum length of any list
    max_length = max(len(lst) for lst in cols.values())

    # Pad lists with zeros to match the maximum length
    for key in cols:
        while len(cols[key]) < max_length:
            cols[key].append(0)

    df = pd.DataFrame(data=cols)
    df = df.astype(float)
    return df


df = df_from_pear_reports(extract_pear_reports("20_1_1405_1.txt"))

print(df.dtypes)

f = open("csv.csv", "w")
f.write(df.to_csv())
