# WINDNINJA TUTORIALS SET UP INSTRUCTIONS


## Step 1: SETTING THE WINDNINJA_DATA ENVIRONMENTAL VARIABLE

The WINDNINJA_DATA environment variable must be set and must point to the location of WindNinja's data files in order to run the tutorial cases. Here's how to set it:

- Determine the path to the WindNinja data files. This will depend on where WindNinja was installed on your system.

- Set the WINDNINJA_DATA environment variable. For example, if your WindNinja data files are located at `/path/to/windninja_data`, you would set the variable like this:
   - `export WINDNINJA_DATA=/path/to/windninja_data`

## STEP 2: SETTING UP YOUR TUTORIAL DIRECTORY 

To avoid contamination of the source tree, it's best to run tutorial cases in a separate directory. You can create and use a directory like `/home/user/src/windninja/build/TUTORIAL`. Feel free to choose a directory that suits your setup. Here's how to do it:

- Navigate to your preferred directory (`windninja/build` is a good place to start):
- Create a new directory for your tutorials:
  - `mkdir TUTORIAL`
- Move into the newly created directory:
  - `cd TUTORIAL`

- Copy the tutorial cases from the WINDNINJA_DATA directory:
  - `cp -r $WINDNINJA_DATA/tutorial*`

## Step 3: RUNNING THE TUTORIAL CASES
 After setting up the environment and copying the tutorial cases, you can run them as follows:

- Navigate to the directory containing the tutorial case you want to run.

- Run the case's python file with the following command:
    `python your_case.py`  (replace your_case.py with the corect python file)

**NOTE**: The tutorial cases require python version 3 or newer.
