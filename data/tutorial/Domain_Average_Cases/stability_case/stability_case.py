import os
import subprocess
import shutil

def run_windninja(cfg_file_path):
   

    # Command to run
    command = ["WindNinja_cli", cfg_file_path]

    try:
        # Run the command
        subprocess.run(command, check=True)
        print("WindNinja simulation completed successfully.")

    except subprocess.CalledProcessError as e:
        print("An error occurred during WindNinja simulation:", e)



def copy_files(source_directory, destination_directory, file_names):
    """
    Copies specified files from the source directory to the destination directory.
    File names will remain the same in the destination directory.

    :param source_directory: The path to the source directory
    :param destination_directory: The path to the destination directory
    :param file_names: A list of file names to copy
    """
    # Ensure destination directory exists
    if not os.path.exists(destination_directory):
        os.makedirs(destination_directory)

    # Copy each file
    for file_name in file_names:
        # Define the full file paths
        source_file = os.path.join(source_directory, file_name)
        destination_file = os.path.join(destination_directory, file_name)

        # Check if the file exists and then copy it
        if os.path.exists(source_file):
            shutil.copy(source_file, destination_file)
            print("Copied {file_name} to {destination_directory}")
        else:
            print("File not found: {source_file}")



if __name__ == "__main__":

    # Use WINDNINJA_DATA environment variable
    data_dir = os.environ.get("WINDNINJA_DATA")

    if not data_dir:
        raise ValueError("WINDNINJA_DATA environment variable not set. Please set it to the WindNinja data directory.")
    destination_dir = './'
    files_to_copy = ['big_butte.tif']

    copy_files(data_dir, destination_dir, files_to_copy)

    run_windninja("./cli_bigbutte_stability.cfg")
