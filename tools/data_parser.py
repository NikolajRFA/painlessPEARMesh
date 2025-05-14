import json
import pandas as pd


class PearReport:
    from_node: int
    tx_period: int
    rx_period: int
    parent_candidates: list[int]

    def __init__(self, from_node: int, tx_period: int, rx_period: int, parent_candidates: list[int]):
        self.from_node = from_node
        self.tx_period = tx_period
        self.rx_period = rx_period
        self.parent_candidates = parent_candidates

    def __str__(self):
        return f"From: {self.from_node}, Tx: {self.tx_period}, Rx: {self.rx_period}, Parents: {self.parent_candidates}"


def extract_pear_reports(file_path):
    pear_reports = []
    collecting = False
    with open(file_path, 'r', encoding="utf-16") as file:
        for line in file:
            if line.startswith("Number of nodes in mesh: 20"):
                collecting = True
            if collecting:
                if line.startswith("DATA: "):
                    line_without_data = line.replace("DATA: ", "").strip()
                    from_node = line_without_data.split(";")[0].strip()
                    json_data = json.loads(line_without_data.split(";")[1].strip())
                    pear_report = PearReport(int(from_node), json_data['txPeriod'], json_data['rxPeriod'],
                                             json_data['parentCandidates'])
                    pear_reports.append(pear_report)

    return pear_reports


def csv_from_pear_reports(pear_reports: list[PearReport]):
    cols: dict[str, list[int]] = {}

    # Populate the dictionary
    for pear_report in pear_reports:
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

    return pd.DataFrame(data=cols)


print(csv_from_pear_reports(extract_pear_reports("20_1_1405_1.txt")).to_csv())
