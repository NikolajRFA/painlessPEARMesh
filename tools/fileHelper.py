def get_file_paths_from_folder(folder_path):
    import os
    file_paths = []
    for root, directories, files in os.walk(folder_path):
        for filename in files:
            filepath = os.path.join(root, filename)
            file_paths.append(filepath)
    return file_paths
