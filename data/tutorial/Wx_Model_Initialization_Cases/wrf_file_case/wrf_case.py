import urllib.request
import os
import ssl
import subprocess
import shutil

def download_file(url, local_filename):
    try:
        ssl_context = ssl._create_unverified_context()

        with urllib.request.urlopen(url, context=ssl_context) as response, open(local_filename, 'wb') as out_file:
            data = response.read()
            out_file.write(data)


            # Write the data to a local file
            with open(local_filename, 'wb') as file:
                file.write(data)

    except Exception as e:
        print("error occured", e)




def run_windninja(cfg_file_path):
    

    # Command to run
    command = ["WindNinja_cli", cfg_file_path]

    try:
        # Run the command
        subprocess.run(command, check=True)
        print("WindNinja simulation completed successfully.")

    except subprocess.CalledProcessError as e:
        print(f"An error occurred during WindNinja simulation:", e)




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
            print(f"Copied {file_name} to {destination_directory}")
        else:
            print(f"File not found: {source_file}")



if __name__ == "__main__":
    # URL of the file to be downloaded
    file_url = "https://ninjastorm.firelab.org/data/wrfout_d04_2010-09-05_120000.nc"

    # Local filename (in the same directory as the script)
    script_dir = os.path.dirname(os.path.realpath(__file__))
    local_filename = os.path.join(script_dir, "wrfout_d04_2010-09-05_120000.nc")

    # Download the file
    download_file(file_url, local_filename)

    data_dir = os.environ.get("WINDNINJA_DATA")

    if not data_dir:
        raise ValueError("WINDNINJA_DATA environment variable not set. Please set it to the WindNinja data directory.")
    destination_dir = './'
    
    files_to_copy = ['big_butte.tif']

    copy_files(data_dir, destination_dir, files_to_copy)

    run_windninja("./bigbutte_wrf_initialization.cfg")
