from tools import data_parser
from tools.fileHelper import get_file_paths_from_folder, get_short_node_id
from tools.data_parser_2 import extract_pear_reports, df_from_pear_reports, PearReport
# from tools.energy_profile import get_energy_profile_from_node, get_energy_profile_id, get_energy_profile, \
#    get_energy_profile_from_node_H6L13, get_energy_profile_id_H6L13
import re
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


# run_data: list[tuple[list[PearReport], int]] = [extract_pear_reports("data/7th set/20_7_1705_3.txt"), extract_pear_reports("data/7th set/20_7_1705_4.txt"), #extract_pear_reports("data/7th set/20_7_1705_6.txt"), extract_pear_reports("data/7th set/20_7_1705_7.txt"), extract_pear_reports("data/7th set/20_7_1705_8.txt")]


def chart_setup(data_path: str, energy_profile_id_map: dict, energy_profile_map: dict, chart_title: str = "", use_old_data_parser = False):
    # Get all file paths from the folder
    file_paths = get_file_paths_from_folder(data_path)

    # List to store DataFrames
    dfs = []
    pear_stable_time = None

    # Iterate over each file
    for file_path in file_paths:
        # Extract the last integer from the file name using regex
        match = re.search(r"(\d+)(?=\..*$)", file_path)  # Finds the last number before the file extension
        if match:
            run_id = int(match.group(1))  # Convert to integer
        else:
            continue  # Skip files without a valid number

        # Extract pear reports
        if use_old_data_parser:
            pear_reports = data_parser.extract_pear_reports(file_path)
            df = data_parser.df_from_pear_reports(pear_reports, run_id)
        else:
            pear_reports = extract_pear_reports(file_path)[0]
            df = df_from_pear_reports(pear_reports, run_id)

        # Convert to DataFrame and add run_id column


        # Append to list
        dfs.append(df)

    # Combine all DataFrames
    df = pd.concat(dfs, ignore_index=True)
    # df['from_node'] = df["from_node"].apply(get_short_node_id)

    # Sort by node_time and from_node for consistency
    df = df.sort_values(by=["from_node", "node_time"])

    sorted_df = df.sort_values(by=['from_node', 'run_id', 'node_time'])

    unique_node_ids = df['from_node'].unique()

    # Get all file paths from the folder
    no_pear_file_paths = get_file_paths_from_folder("data/no pear")

    # List to store DataFrames
    no_pear_dfs = []
    pear_stable_time = None

    # Iterate over each file
    for file_path in no_pear_file_paths:
        # Extract the last integer from the file name using regex
        match = re.search(r"(\d+)(?=\..*$)", file_path)  # Finds the last number before the file extension
        if match:
            run_id = int(match.group(1))  # Convert to integer
        else:
            continue  # Skip files without a valid number

        # Extract pear reports
        pear_reports, pear_stable_time1 = extract_pear_reports(file_path)

        # Convert to DataFrame and add run_id column
        no_pear_df = df_from_pear_reports(pear_reports, run_id)

        # Append to list
        no_pear_dfs.append(no_pear_df)

    # Combine all DataFrames
    no_pear_df = pd.concat(no_pear_dfs, ignore_index=True)

    # Sort by node_time and from_node for consistency
    no_pear_df = no_pear_df.sort_values(by=["from_node", "node_time"])

    no_pear_2min_mean = no_pear_df['tx_period'].mean() * 19 * 2

    # Make a copy to work on
    temp_df = df.copy()

    # Convert node_time to milliseconds if necessary
    temp_df["node_time"] = temp_df["node_time"] / 1000

    # Sort data to ensure correct forward-filling
    temp_df.sort_values(["from_node", "node_time", "run_id"], inplace=True)

    # Create a new dataframe to store adjusted rows
    new_rows = []

    # Iterate over each node's data to identify missing reports
    for (node, run_id), group in temp_df.groupby(["from_node", "run_id"]):
        last_time = None

        for _, row in group.iterrows():
            if last_time is not None and row["node_time"] > last_time + 70000:
                while last_time + 60000 < row["node_time"]:
                    new_row = row.copy()
                    new_row["node_time"] = last_time + 60000
                    new_rows.append(new_row)
                    last_time += 60000

            last_time = row["node_time"]

    #print(f'new_rows: {len(new_rows)}')
    # Append new rows to original dataframe
    temp_df = pd.concat([temp_df, pd.DataFrame(new_rows)], ignore_index=True)

    # Resort dataframe
    temp_df.sort_values(["from_node", "node_time", "run_id"], inplace=True)

    # **Remove all rows where node_time is below 240000**
    temp_df = temp_df[temp_df["node_time"] >= 240000].copy()

    # **Recalculate time_bin AFTER filtering**
    temp_df["time_bin"] = temp_df["node_time"] // 120000  # Convert milliseconds into 2-minute bins

    # Aggregate total transmissions for the whole network over two-minute intervals, grouped by run_id
    df_agg = temp_df.groupby(["time_bin", "run_id"])["tx_period"].sum().reset_index()

    df_agg = df_agg[df_agg["time_bin"] != df_agg.groupby("run_id")["time_bin"].transform("max")]

    # Convert time_bin to minutes
    df_agg["time_bin_min"] = (df_agg["time_bin"] * 120000) / (60 * 1000)  # Convert microseconds to minutes

    plt.figure(figsize=(10, 6))

    # Define start and end for vertical lines in minutes
    # start_time_min = start_time / (60 * 1000)
    # end_time_min = end_time / (60 * 1000)

    # Draw vertical lines every 2 minutes starting at start_time

    plt.axvline(x=6, color="red", linestyle="--", linewidth=1, label="Start PEAR algorithm")

    # for x in range(int(start_time_min), int(end_time_min) + 1, 2):
    # plt.axvline(x=x, color="red", linestyle="--", linewidth=1, label="PEAR algorithm runs" if x == start_time_min else "")

    # Plot a separate curve for each run_id
    for run_id, run_group in df_agg.groupby("run_id"):
        plt.plot(run_group["time_bin_min"], run_group["tx_period"], marker="", linestyle="-", label=f"Run {run_id}")

        # Get the last data point for each run_id
        last_x = run_group["time_bin_min"].iloc[-1]
        last_y = run_group["tx_period"].iloc[-1]

        # Plot only the last data point with marker "o"
        plt.plot(last_x, last_y, marker="o", markersize=6, color="maroon")

    # Add reference horizontal line for the average transmissions without PEAR
    plt.axhline(y=no_pear_2min_mean, color="green", linestyle="--", linewidth=1, label="Average packages w/o PEAR")

    # Adjust x-ticks for better readability
    plt.xticks(df_agg["time_bin_min"], labels=df_agg["time_bin_min"].round(1), rotation=45)

    plt.xlabel("Time (minutes)")
    plt.ylabel("Total Packages Transmitted (2-minute window)")
    plt.title(f'{chart_title + " " if chart_title != "" else ""}Packages Transmitted Over Time (Grouped by Run ID)')
    plt.legend(loc="upper right", fontsize="small")
    plt.grid()
    plt.tight_layout()
    plt.show()

    # Ensure each row retains its run_id before filtering
    success_rate_df = temp_df.groupby(["from_node", "run_id"]).tail(1).copy()

    # Extract tx_threshold and rx_threshold separately
    success_rate_df[["tx_threshold", "rx_threshold"]] = success_rate_df["from_node"].apply(
        lambda this_node: pd.Series(energy_profile_map[energy_profile_id_map[this_node]])
    )

    # Compute energy profile
    success_rate_df["energy_profile"] = success_rate_df["tx_threshold"] + success_rate_df["rx_threshold"]

    # Compute success condition
    success_rate_df["success"] = (success_rate_df["tx_period"] < success_rate_df["tx_threshold"]) | (
            success_rate_df["rx_period"] < success_rate_df["rx_threshold"])

    # Group by tx_threshold, rx_threshold, and run_id instead of energy_profile
    grouped_success_rate_df = success_rate_df.groupby(["tx_threshold", "rx_threshold", "run_id"]).agg(
        success_rate=("success", lambda x: (x.sum() / len(x)) * 100),  # Percentage of successful devices
        overconsuming_devices=("success", lambda x: (len(x) - x.sum()))  # Count of unsuccessful devices
    ).reset_index()

    # Compute the average overconsuming devices per tx_threshold and rx_threshold across all run_ids
    average_overconsuming_df = grouped_success_rate_df.groupby(["tx_threshold", "rx_threshold"]).agg(
        avg_overconsuming_devices=("overconsuming_devices", "mean"),  # Average count of unsuccessful devices
        avg_success_rate=("success_rate", "mean")  # Average percentage of successful devices
    ).reset_index()

    # Convert success rate into a failure rate (percentage of unsuccessful devices)
    average_overconsuming_df["failure_rate"] = 100 - average_overconsuming_df["avg_success_rate"]

    # Display the results
    # average_overconsuming_df.head()

    # Make a copy to work on
    temp_df = df.copy()

    # Convert node_time to milliseconds if necessary
    temp_df["node_time"] = temp_df["node_time"] / 1000

    # Sort data to ensure correct forward-filling
    temp_df.sort_values(["from_node", "node_time", "run_id"], inplace=True)

    # Create a new dataframe to store adjusted rows
    new_rows = []

    # Iterate over each node's data to identify missing reports
    for (node, run_id), group in temp_df.groupby(["from_node", "run_id"]):
        last_time = None

        for _, row in group.iterrows():
            if last_time is not None and row["node_time"] > last_time + 70000:
                while last_time + 60000 < row["node_time"]:
                    new_row = row.copy()
                    new_row["node_time"] = last_time + 60000
                    new_rows.append(new_row)
                    last_time += 60000

            last_time = row["node_time"]

    # Append new rows to original dataframe
    temp_df = pd.concat([temp_df, pd.DataFrame(new_rows)], ignore_index=True)

    # Resort dataframe
    temp_df.sort_values(["from_node", "node_time", "run_id"], inplace=True)

    # **Remove all rows where node_time is below 240000**
    temp_df = temp_df[temp_df["node_time"] >= 240000].copy()

    # **Recalculate time_bin AFTER filtering**
    temp_df["time_bin"] = temp_df["node_time"] // 120000  # Convert milliseconds into 2-minute bins

    # Aggregate total transmissions for each from_node over two-minute intervals, grouped by run_id
    df_binned = temp_df.groupby(["time_bin", "from_node", "run_id"]).agg(
        tx_sum=("tx_period", "sum")  # Sum of transmissions for each from_node in the time_bin
    ).reset_index()

    df_binned = df_binned[df_binned["time_bin"] != df_binned.groupby("run_id")["time_bin"].transform("max")]

    # Get all unique run_ids
    run_ids = df_binned["run_id"].unique()

    # Count occurrences of each time_bin across all run_ids
    time_bin_counts = df_binned.groupby("time_bin")["run_id"].nunique()

    # Find the minimum and maximum common time bins
    min_common_time_bin = time_bin_counts[time_bin_counts == len(run_ids)].index.min()
    max_common_time_bin = time_bin_counts[time_bin_counts == len(run_ids)].index.max()

    # Aggregate total transmissions for each from_node over two-minute intervals, averaging across runs
    df_avg_binned = df_binned.groupby(["time_bin", "from_node"]).agg(
        avg_tx_sum=("tx_sum", "mean")  # Compute the average tx_sum over all runs
    ).reset_index()

    df_avg_binned_min = df_avg_binned[df_avg_binned['time_bin'] == min_common_time_bin]
    df_avg_binned_max = df_avg_binned[df_avg_binned['time_bin'] == max_common_time_bin]

    combined_df = pd.concat([df_avg_binned_min, df_avg_binned_max], ignore_index=True)

    # Pivot dataframe to structure data for plotting
    df_pivoted = combined_df.pivot(index="from_node", columns="time_bin", values="avg_tx_sum")

    df_pivoted['energy_profile_id'] = df_pivoted.index.to_series().astype(int).apply(energy_profile_id_map.get)

    # Ensure nodes are sorted correctly
    df_pivoted = df_pivoted.sort_index()

    # Get x-axis positions
    x = np.arange(len(df_pivoted))

    # Define bar width
    bar_width = 0.4

    for energy_profile_id in sorted(df_pivoted["energy_profile_id"].unique()):
        if energy_profile_id == 0: continue
        tx_threshold, rx_threshold = energy_profile_map[int(energy_profile_id)]
        tx_threshold *= 2
        rx_threshold *= 2

        # Filter only nodes with the current energy profile
        this_pivot_df = df_pivoted[df_pivoted["energy_profile_id"] == energy_profile_id]

        # Generate x-axis positions dynamically
        x = np.arange(len(this_pivot_df))  # Match size of filtered nodes

        # Create plot
        plt.figure(figsize=(12, 6))
        bars_pre = plt.bar(x, this_pivot_df.iloc[:, 0], width=bar_width, label="Pre PEAR",
                           color="blue")  # First time_bin column
        bars_post = plt.bar(x + bar_width, this_pivot_df.iloc[:, 1], width=bar_width, label="Post PEAR",
                            color="orange")  # Second time_bin column

        plt.axhline(y=tx_threshold, color="red", linestyle="--", linewidth=1, label="Tx Threshold")

        # Add values on top of bars
        for bar in bars_pre:
            plt.text(bar.get_x() + bar.get_width() / 2, bar.get_height(), f"{bar.get_height():.1f}",
                     ha="center", va="bottom", fontsize=10, color="black")
        for bar in bars_post:
            plt.text(bar.get_x() + bar.get_width() / 2, bar.get_height(), f"{bar.get_height():.1f}",
                     ha="center", va="bottom", fontsize=10, color="black")

        # Formatting
        plt.xticks(x + bar_width / 2, labels=this_pivot_df.index.map(lambda node_id: get_short_node_id(node_id)))
        plt.xlabel("From Node")
        plt.ylabel("2 Minute Average Tx Sum")
        plt.title(f"{chart_title + " " if chart_title != "" else ""}2 Minute Average Tx per Node (Energy Profile {energy_profile_id})")
        plt.legend()
        plt.grid(axis="y", linestyle="--")

        plt.tight_layout()
        plt.show()

        max_message_before = this_pivot_df[min_common_time_bin].max()
        max_message_after = this_pivot_df[max_common_time_bin].max()

        print(f'Difference between max before and after PEAR {max_message_before - max_message_after}')
        print(100 - (max_message_after / max_message_before * 100))


"""energy_profile_id_map = {
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

energy_profile_map = {
    1: (60, 45),
    2: (20, 15),
}

chart_setup("data/H6L13", energy_profile_id_map, energy_profile_map)"""
