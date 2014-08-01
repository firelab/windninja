/******************************************************************************
*
* $Id$
*
* Project:  WindNinja Qt GUI
* Purpose:  Command line parser and model run for WindNinja
* Author:   Kyle Shannon <ksshannon@gmail.com>
*
******************************************************************************
*
* THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
* MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
* IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
* OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
* PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
* LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
* PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
* RELIABILITY, OR ANY OTHER CHARACTERISTIC.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

#include "cli.h"
#include <string>

/**
 * Function used to check that 'opt1' and 'opt2' are not specified
 * at the same time.
 * @param vm variable map
 * @param opt1 first option
 * @param opt2 second option
 */
void conflicting_options(const po::variables_map& vm, const char* opt1, const char* opt2)
{
    if (vm.count(opt1) && !vm[opt1].defaulted()
        && vm.count(opt2) && !vm[opt2].defaulted())
        throw logic_error(string("Conflicting options '")
                          + opt1 + "' and '" + opt2 + "'.");
}

/**
 * Function used to check that of 'for_what' is specified, then
 * 'required_option' is specified too.
 * @param vm variable map
 * @param for_what option with dependency
 * @param required_option required option
 */
void option_dependency(const po::variables_map& vm, const char* for_what, const char* required_option)
{
    if (vm.count(for_what) && !vm[for_what].defaulted())
        if (vm.count(required_option) == 0 || vm[required_option].defaulted())
            throw logic_error(string("Option '") + for_what
                              + "' requires option '" + required_option + "'.");
}

/**
 * Function used to verify if an option has been set (not defaulted)
 * @param vm variable map
 * @param optn required option
 */
void verify_option_set(const po::variables_map& vm, const char* optn)
{
    if(!vm.count(optn) && !vm[optn].defaulted())
    {
        throw logic_error(std::string("Option '") + optn + "' was not set.\n");
    }
}

/*
bool checkArgs(string arg1, string arg2, string arg3)
{

}
*/

// Additional command line parser which interprets '@something' as a
// option "response_file" with the value "something"
pair<string, string> at_option_parser(string const&s)
{
    if ('@' == s[0])
        return std::make_pair(string("response_file"), s.substr(1));
    else
        return pair<string, string>();
}

/** @brief This method initializes the CLI options so that both the command line and the API can use them
 *
 */
/*
void initializeOptions()
{
    try {
        // Declare a group of options that will be
        // allowed only on command line
        po::options_description generic("Generic options");
        generic.add_options()
                        ("version", "print version")
                        ("help", "produce help message")
                        ("config_file", po::value<std::string>(),
                                "configuration file ('config_file' flag not required)")
                        ("response_file", po::value<std::string>(),
                                 "response file (can be specified with '@name', also)")
                                ;

        // Declare a group of options that will be
        // allowed both on command line and in
        // config file
        po::options_description config("Simulation options");
        config.add_options()
                ("num_threads", po::value<int>()->default_value(1), "number of threads to use during simulation")
                ("elevation_file", po::value<std::string>(), "input elevation path/filename (*.asc, *.lcp, *.tif, *.img)")
                ("fetch_elevation", po::value<std::string>(), "fetch elevation file and save to path/filename")
                ("north", po::value<double>(), "north extent of elevation file bounding box")
                ("east", po::value<double>(), "east extent of elevation file bounding box")
                ("south", po::value<double>(), "south extent of elevation file bounding box")
                ("west", po::value<double>(), "west extent of elevation file bounding box")
                ("x_center", po::value<double>(), "x coordinate of center of elevation domain to be downloaded")
                ("y_center", po::value<double>(), "y coordinate of center of elevation domain to be downloaded")
                ("x_buffer", po::value<double>(), "x buffer of elevation domain to be downloaded")
                ("y_buffer", po::value<double>(), "y buffer of elevation domain to be downloaded")
                ("buffer_units", po::value<std::string>()->default_value("miles"), "units for buffer (kilometers, miles)")
                ("elevation_source", po::value<std::string>()->default_value("us_srtm"), "Source for downloading elevation data (us_srtm, world_srtm, gmted)")
                ("initialization_method", po::value<std::string>()->required(), "initialization method (domainAverageInitialization, pointInitialization, wxModelInitialization)")
                ("time_zone", po::value<std::string>(), "time zone (common choices are: America/New_York, America/Chicago, America/Denver, America/Phoenix, America/Los_Angeles, America/Anchorage; all choices are listed in date_time_zonespec.csv)")
                ("wx_model_type", po::value<std::string>(), "type of wx model to download (NCAR-NAM-12-KM, NCAR-NAM-Alaska-11-KM, NCAR-NDFD-5-KM, NCAR-RAP-13-KM)")
                ("forecast_duration", po::value<int>(), "forecast duration to download (in hours)")
                ("forecast_filename", po::value<std::string>(), "path/filename of an already downloaded wx forecast file")
                ("match_points",po::value<bool>()->default_value(true), "match simulation to points(true, false)")
                ("input_speed", po::value<double>(), "input wind speed")
                ("input_speed_units", po::value<std::string>(), "units of input wind speed (mps, mph, kph)")
                ("output_speed_units", po::value<std::string>()->default_value("mph"), "units of output wind speed (mps, mph, kph)")
                ("input_direction", po::value<double>(), "input wind direction")
                ("uni_air_temp", po::value<double>(), "surface air temperature")
                ("air_temp_units", po::value<std::string>(), "surface air temperature units (K, C, R, F)")
                ("uni_cloud_cover", po::value<double>(), "cloud cover")
                ("cloud_cover_units", po::value<std::string>(), "cloud cover units (fraction, percent, canopy_category)")
                ("wx_station_filename", po::value<std::string>(), "path/filename of input wx station file")
                ("write_wx_station_kml", po::value<bool>()->default_value(false), "write a Google Earth kml file for the input wx stations (true, false)")
                ("wx_station_kml_filename", po::value<std::string>(), "filename for the Google Earth kml wx station output file")
                ("input_wind_height", po::value<double>(), "height of input wind speed above the vegetation")
                ("units_input_wind_height", po::value<std::string>(), "units of input wind height (ft, m)")
                ("output_wind_height", po::value<double>()->required(), "height of output wind speed above the vegetation")
                ("units_output_wind_height", po::value<std::string>(), "units of output wind height (ft, m)")
                ("vegetation", po::value<std::string>(), "dominant type of vegetation (grass, brush, trees)")
                ("diurnal_winds", po::value<bool>()->default_value(false), "include diurnal winds in simulation (true, false)")
                ("year", po::value<int>(), "year of simulation")
                ("month", po::value<int>(), "month of simulation")
                ("day", po::value<int>(), "day of simulation")
                ("hour", po::value<int>(), "hour of simulation")
                ("minute", po::value<int>(), "minute of simulation")
                ("mesh_choice", po::value<std::string>(), "mesh resolution choice (coarse, medium, fine)")
                ("mesh_resolution", po::value<double>(), "mesh resolution")
                ("units_mesh_resolution", po::value<std::string>(), "mesh resolution units (ft, m)")
                ("output_buffer_clipping", po::value<double>()->default_value(0.0), "percent to clip buffer on output files")
                ("write_wx_model_goog_output", po::value<bool>()->default_value(false), "write a Google Earth kmz output file for the raw wx model forecast (true, false)")
                ("write_goog_output", po::value<bool>()->default_value(false), "write a Google Earth kmz output file (true, false)")
                ("goog_out_resolution", po::value<double>()->default_value(-1.0), "resolution of Google Earth output file (-1 to use mesh resolution)")
                ("units_goog_out_resolution", po::value<std::string>()->default_value("m"), "units of Google Earth resolution (ft, m)")
                ("write_wx_model_shapefile_output", po::value<bool>()->default_value(false), "write a shapefile output file for the raw wx model forecast (true, false)")
                ("write_shapefile_output", po::value<bool>()->default_value(false), "write a shapefile output file (true, false)")
                ("shape_out_resolution", po::value<double>()->default_value(-1.0), "resolution of shapefile output file (-1 to use mesh resolution)")
                ("units_shape_out_resolution", po::value<std::string>()->default_value("m"), "units of shapefile resolution (ft, m)")
                ("write_wx_model_ascii_output", po::value<bool>()->default_value(false), "write ascii fire behavior output files for the raw wx model forecast (true, false)")
                ("write_ascii_output", po::value<bool>()->default_value(false), "write ascii fire behavior output files (true, false)")
                ("ascii_out_resolution", po::value<double>()->default_value(-1.0), "resolution of ascii fire behavior output files (-1 to use mesh resolution)")
                ("units_ascii_out_resolution", po::value<std::string>()->default_value("m"), "units of ascii fire behavior output file resolutino (ft, m)")
                ("write_vtk_output", po::value<bool>()->default_value(false), "write VTK output file (true, false)")
                ("write_farsite_atm", po::value<bool>()->default_value(false), "write a FARSITE atm file (true, false)")
                #ifdef STABILITY
                ("non_neutral_stability", po::value<bool>()->default_value(false), "use non-neutral stability (true, false)")
                ("alpha_stability", po::value<double>(), "alpha value for atmospheric stability")
                #endif
                #ifdef EMISSIONS
                ("compute_emissions",po::value<bool>()->default_value(false), "compute dust emissions (true, false)")
                ("fire_perimeter_file", po::value<std::string>(), "input burn perimeter path/filename (*.shp)")
                ("dust_file_out", po::value<std::string>(), "name of emissions output file")
                #endif
                ;

        // Hidden options, will be allowed both on command line and
        // in config file, but will not be shown to the user.
        //po::options_description hidden("Hidden options");
        //hidden.add_options()
        //                ("input-file", po::value< vector<string> >(), "input file")
        //                ;


        po::options_description cmdline_options;
        //cmdline_options.add(generic).add(config).add(hidden);
        cmdline_options.add(generic).add(config);

        po::options_description config_file_options;
        //config_file_options.add(config).add(hidden);
        config_file_options.add(config);

        po::options_description visible("Allowed options");
        visible.add(generic).add(config);

        po::positional_options_description p;
        p.add("config_file", -1);

        po::variables_map vm;

        po::parsed_options opts_command = po::command_line_parser(argc, argv).
                        options(cmdline_options).extra_parser(at_option_parser).positional(p).run();

    initialized = true;
}
*/

/**
 * Command line implementation (CLI) of WindNinja.  Can be run using command line args or
 * from an input file.
 * @param argc Number of args
 * @param argv Arguments
 * @return zero if successful, non-zero otherwise
 */
int windNinjaCLI(int argc, char* argv[])
{
    setbuf(stdout, NULL);

    try
    {
        WindNinjaInputs inputs;
    }
    catch(std::exception &e)
    {
        fprintf(stderr, "Could not initialize WindNinja, try setting " \
                        "WINDNINJA_DATA\n");
        return 1;
    }
    //write out parsed options for debugging
    bool writeParsed = false;
    bool writeValues = false;

    //initializeOptions();

    // Moved to initializeOptions()
    try {
        // Declare a group of options that will be
        // allowed only on command line
        po::options_description generic("Generic options");
        generic.add_options()
                        ("version", "print version")
                        ("help", "produce help message")
                        ("config_file", po::value<std::string>(),
                                "configuration file ('config_file' flag not required)")
                        ("response_file", po::value<std::string>(),
                                 "response file (can be specified with '@name', also)")
                                ;

        // Declare a group of options that will be
        // allowed both on command line and in
        // config file
        po::options_description config("Simulation options");
        config.add_options()
                ("num_threads", po::value<int>()->default_value(1), "number of threads to use during simulation")
                ("elevation_file", po::value<std::string>(), "input elevation path/filename (*.asc, *.lcp, *.tif, *.img)")
                ("fetch_elevation", po::value<std::string>(), "download an elevation file from an internet server and save to path/filename")
                ("north", po::value<double>(), "north extent of elevation file bounding box to download")
                ("east", po::value<double>(), "east extent of elevation file bounding box to download")
                ("south", po::value<double>(), "south extent of elevation file bounding box to download")
                ("west", po::value<double>(), "west extent of elevation file bounding box to download")
                ("x_center", po::value<double>(), "x coordinate of center of elevation domain to download")
                ("y_center", po::value<double>(), "y coordinate of center of elevation domain to download")
                ("x_buffer", po::value<double>(), "x buffer of elevation domain to download (distance in east-west direction from center to edge of domain)")
                ("y_buffer", po::value<double>(), "y buffer of elevation domain to download (distance in north-south direction from center to edge of domain)")
                ("buffer_units", po::value<std::string>()->default_value("miles"), "units for x_buffer and y_buffer of  elevation file to download (kilometers, miles)")
#ifdef HAVE_GMTED
                ("elevation_source", po::value<std::string>()->default_value("us_srtm"), "source for downloading elevation data (us_srtm, world_srtm, gmted)")
#else
                ("elevation_source", po::value<std::string>()->default_value("us_srtm"), "source for downloading elevation data (us_srtm, world_srtm)")
#endif
                ("initialization_method", po::value<std::string>()/*->required()*/, "initialization method (domainAverageInitialization, pointInitialization, wxModelInitialization)")
                ("time_zone", po::value<std::string>(), "time zone (common choices are: America/New_York, America/Chicago, America/Denver, America/Phoenix, America/Los_Angeles, America/Anchorage; use 'auto-detect' to try and find the time zone for the dem.  All choices are listed in date_time_zonespec.csv)")
                ("wx_model_type", po::value<std::string>(), "type of wx model to download (NCAR-NAM-12-KM, NCAR-NAM-Alaska-11-KM, NCAR-NDFD-5-KM, NCAR-RAP-13-KM, NCAR-GFS-GLOBAL-0.5-deg, )")
                ("forecast_duration", po::value<int>(), "forecast duration to download (in hours)")
                ("forecast_filename", po::value<std::string>(), "path/filename of an already downloaded wx forecast file")
                ("match_points",po::value<bool>()->default_value(true), "match simulation to points(true, false)")
                ("input_speed", po::value<double>(), "input wind speed")
                ("input_speed_units", po::value<std::string>(), "units of input wind speed (mps, mph, kph)")
                ("output_speed_units", po::value<std::string>()->default_value("mph"), "units of output wind speed (mps, mph, kph)")
                ("input_direction", po::value<double>(), "input wind direction")
                ("uni_air_temp", po::value<double>(), "surface air temperature")
                ("air_temp_units", po::value<std::string>(), "surface air temperature units (K, C, R, F)")
                ("uni_cloud_cover", po::value<double>(), "cloud cover")
                ("cloud_cover_units", po::value<std::string>(), "cloud cover units (fraction, percent, canopy_category)")
                ("wx_station_filename", po::value<std::string>(), "path/filename of input wx station file")
                ("write_wx_station_kml", po::value<bool>()->default_value(false), "write a Google Earth kml file for the input wx stations (true, false)")
                ("wx_station_kml_filename", po::value<std::string>(), "filename for the Google Earth kml wx station output file")
                ("input_wind_height", po::value<double>(), "height of input wind speed above the vegetation")
                ("units_input_wind_height", po::value<std::string>(), "units of input wind height (ft, m)")
                ("output_wind_height", po::value<double>()/*->required()*/, "height of output wind speed above the vegetation")
                ("units_output_wind_height", po::value<std::string>(), "units of output wind height (ft, m)")
                ("vegetation", po::value<std::string>(), "dominant type of vegetation (grass, brush, trees)")
                ("diurnal_winds", po::value<bool>()->default_value(false), "include diurnal winds in simulation (true, false)")
                ("year", po::value<int>(), "year of simulation")
                ("month", po::value<int>(), "month of simulation")
                ("day", po::value<int>(), "day of simulation")
                ("hour", po::value<int>(), "hour of simulation")
                ("minute", po::value<int>(), "minute of simulation")
                ("mesh_choice", po::value<std::string>(), "mesh resolution choice (coarse, medium, fine)")
                ("mesh_resolution", po::value<double>(), "mesh resolution")
                ("units_mesh_resolution", po::value<std::string>(), "mesh resolution units (ft, m)")
                ("output_buffer_clipping", po::value<double>()->default_value(0.0), "percent to clip buffer on output files")
                ("write_wx_model_goog_output", po::value<bool>()->default_value(false), "write a Google Earth kmz output file for the raw wx model forecast (true, false)")
                ("write_goog_output", po::value<bool>()->default_value(false), "write a Google Earth kmz output file (true, false)")
                ("goog_out_resolution", po::value<double>()->default_value(-1.0), "resolution of Google Earth output file (-1 to use mesh resolution)")
                ("units_goog_out_resolution", po::value<std::string>()->default_value("m"), "units of Google Earth resolution (ft, m)")
                ("write_wx_model_shapefile_output", po::value<bool>()->default_value(false), "write a shapefile output file for the raw wx model forecast (true, false)")
                ("write_shapefile_output", po::value<bool>()->default_value(false), "write a shapefile output file (true, false)")
                ("shape_out_resolution", po::value<double>()->default_value(-1.0), "resolution of shapefile output file (-1 to use mesh resolution)")
                ("units_shape_out_resolution", po::value<std::string>()->default_value("m"), "units of shapefile resolution (ft, m)")
                ("write_wx_model_ascii_output", po::value<bool>()->default_value(false), "write ascii fire behavior output files for the raw wx model forecast (true, false)")
                ("write_ascii_output", po::value<bool>()->default_value(false), "write ascii fire behavior output files (true, false)")
                ("ascii_out_resolution", po::value<double>()->default_value(-1.0), "resolution of ascii fire behavior output files (-1 to use mesh resolution)")
                ("units_ascii_out_resolution", po::value<std::string>()->default_value("m"), "units of ascii fire behavior output file resolution (ft, m)")
                ("write_vtk_output", po::value<bool>()->default_value(false), "write VTK output file (true, false)")
                ("write_farsite_atm", po::value<bool>()->default_value(false), "write a FARSITE atm file (true, false)")
                #ifdef STABILITY
                ("non_neutral_stability", po::value<bool>()->default_value(false), "use non-neutral stability (true, false)")
                ("alpha_stability", po::value<double>(), "alpha value for atmospheric stability")
                #endif
                #ifdef FRICTION_VELOCITY
                ("compute_friction_velocity",po::value<bool>()->default_value(false), "compute friction velocity (true, false)")
                ("friction_velocity_calculation_method", po::value<std::string>()->default_value("logProfile"), "friction velocity calculation method (logProfile, shearStress)")
                #endif
                #ifdef EMISSIONS
                ("compute_emissions",po::value<bool>()->default_value(false), "compute dust emissions (true, false)")
                ("fire_perimeter_file", po::value<std::string>(), "input burn perimeter path/filename (*.shp)")
                #endif
                #ifdef SCALAR
                ("compute_scalar_transport", po::value<bool>()->default_value(false), "compute scalar transport (true, false)")
                ("scalar_source_strength", po::value<double>()->default_value(-1.0), "source strength in g/s")
                ("scalar_source_xlocation", po::value<double>()->default_value(-1.0), "longitude of source in decimal degrees")
                ("scalar_source_ylocation", po::value<double>()->default_value(-1.0), "latitude of source in decimal degrees")
                #endif
                ("input_points_file", po::value<std::string>(), "input file containing lat,long,z for requested output points (z in m above ground)")
                ("output_points_file", po::value<std::string>(), "file to write containing output for requested points")
                #ifdef NINJAFOAM
                ("momentum_flag", po::value<bool>()->default_value(false), "use momentum solver (true, false)")
                ("number_of_iterations", po::value<int>()->default_value(2000), "number of iterations for momentum solver (must be a multiple of 10)") 
                ("mesh_count", po::value<int>()->default_value(1000000), "number of cells in the mesh") 
                ("non_equilibrium_boundary_conditions", po::value<bool>()->default_value(false), "use non-equilibrium boundary conditions for a momentum solver run (ture, false)") 
                #endif
                ;

        // Hidden options, will be allowed both on command line and
        // in config file, but will not be shown to the user.
        //po::options_description hidden("Hidden options");
        //hidden.add_options()
        //                ("input-file", po::value< vector<string> >(), "input file")
        //                ;


        po::options_description cmdline_options;
        //cmdline_options.add(generic).add(config).add(hidden);
        cmdline_options.add(generic).add(config);

        po::options_description config_file_options;
        //config_file_options.add(config).add(hidden);
        config_file_options.add(config);

        po::options_description visible("Allowed options");
        visible.add(generic).add(config);

        po::positional_options_description p;
        p.add("config_file", -1);

        po::variables_map vm;

        po::parsed_options opts_command = po::command_line_parser(argc, argv).
                        options(cmdline_options).extra_parser(at_option_parser).positional(p).run();
    //

        //write out parsed options for debugging
        if(writeParsed)
        {
            typedef std::vector< po::basic_option<char> > vec_opt;
            cout << "\n\nParsed command line options:" << endl;
            for(vec_opt::iterator l_itrOpt = opts_command.options.begin();
                    l_itrOpt != opts_command.options.end();
                    ++l_itrOpt)
            {
                po::basic_option<char>& l_option = *l_itrOpt;
                cout << "\t" << l_option.string_key << ": ";
                typedef std::vector< std::basic_string<char> > vec_string;
                for(vec_string::iterator l_itrString = l_option.value.begin();
                        l_itrString != l_option.value.end();
                        ++l_itrString)
                {
                    cout << *l_itrString;
                }
                cout << endl;
            }
        }
        store(opts_command, vm);
        //notify(vm);

        if (vm.count("config_file")) {
            ifstream ifs(vm["config_file"].as<std::string>().c_str());
            if (!ifs)
            {
                cout << "can not open config file: " << vm["config_file"].as<std::string>() << "\n";
                return -1;
            }
            else
            {
                po::parsed_options opts_config = parse_config_file(ifs, config_file_options);

                //write out parsed options for debugging
                if(writeParsed)
                {
                    typedef std::vector< po::basic_option<char> > vec_opt;
                    cout << "Parsed configure file options:" << endl;
                    for(vec_opt::iterator l_itrOpt = opts_config.options.begin();
                            l_itrOpt != opts_config.options.end();
                            ++l_itrOpt)
                    {
                        po::basic_option<char>& l_option = *l_itrOpt;
                        cout << "\t" << l_option.string_key << ": ";
                        typedef std::vector< std::basic_string<char> > vec_string;
                        for(vec_string::iterator l_itrString = l_option.value.begin();
                                l_itrString != l_option.value.end();
                                ++l_itrString)
                        {
                            cout << *l_itrString;
                        }
                        cout << endl;
                    }
                }

                store(opts_config, vm);
                //store(parse_config_file(ifs, config_file_options), vm);
                //notify(vm);
            }
        }

        if (vm.count("help")) {
            cout << visible << "\n";
            return 0;
        }

        if (vm.count("version")) {
            cout << "WindNinja version: " << VERSION << "\n";
            cout << "SVN version: " << SVN_VERSION << "\n";
            cout << "Release date: " << RELEASE_DATE << "\n";
            return 0;
        }

        if (vm.count("response_file")) {
            // Load the file and tokenize it
            ifstream ifs(vm["response_file"].as<string>().c_str());
            if (!ifs) {
                cout << "Could not open the response file\n";
                return -1;
            }
            // Read the whole file into a string
            stringstream ss;
            ss << ifs.rdbuf();
            // Split the file content
            boost::char_separator<char> sep(" \n\r");
            string sstr = ss.str();
            boost::tokenizer<boost::char_separator<char> > tok(sstr, sep);
            vector<string> args;
            copy(tok.begin(), tok.end(), back_inserter(args));

            for(unsigned int i=0; i< args.size(); i++)
                std::cout << args[i] << std::endl;

            po::parsed_options opts_resp = po::command_line_parser(args).options(cmdline_options).run();

            //write out parsed options for debugging
            if(writeParsed)
            {
                typedef std::vector< po::basic_option<char> > vec_opt;
                cout << "Parsed configure file options:" << endl;
                for(vec_opt::iterator l_itrOpt = opts_resp.options.begin();
                        l_itrOpt != opts_resp.options.end();
                        ++l_itrOpt)
                {
                    po::basic_option<char>& l_option = *l_itrOpt;
                    cout << "\t" << l_option.string_key << ": ";
                    typedef std::vector< std::basic_string<char> > vec_string;
                    for(vec_string::iterator l_itrString = l_option.value.begin();
                            l_itrString != l_option.value.end();
                            ++l_itrString)
                    {
                        cout << *l_itrString;
                    }
                    cout << endl;
                }
            }

            // Parse the file and store the options
            store(opts_resp, vm);
            //store(po::command_line_parser(args).options(cmdline_options).run(), vm);
        }

        notify(vm);


        //write out values in vm for debugging
        if(writeValues)
        {
            std::cout << "\n\nValues of variables map vm:\n";
            for ( po::variables_map::iterator i = vm.begin() ; i != vm.end() ; ++ i )
            {
                 const po::variable_value& v = i->second ;
                 if ( ! v.empty() )
                 {
                     const ::std::type_info& type = v.value().type() ;
                     if ( type == typeid( ::std::string ) )
                     {
                         const ::std::string& val = v.as< ::std::string >() ;
                         std::cout << i->first << "\t" << val << "\n";
                     }
                     else if ( type == typeid( int ) )
                     {
                         int val = v.as< int >() ;
                         std::cout << i->first << "\t" << val << "\n";
                     }
                     else if ( type == typeid( double ) )
                     {
                         double val = v.as< double >() ;
                         std::cout << i->first << "\t" << val << "\n";
                     }
                     else if ( type == typeid( bool ) )
                     {
                         bool val = v.as< bool >() ;
                         std::cout << i->first << "\t" << val << "\n";
                     }

                 }
            }
        }

        #ifdef NINJAFOAM
        ninjaArmy windsim(1, vm["momentum_flag"].as<bool>()); //-Moved to header file
        #endif

        #ifndef NINJAFOAM
        ninjaArmy windsim(1); //-Moved to header file
        #endif

        /* Do we have to fetch an elevation file */

        if(vm.count("north") || vm.count("south") ||
               vm.count("east") || vm.count("west"))
        {
            option_dependency(vm, "north", "fetch_elevation");
            option_dependency(vm, "east", "fetch_elevation");
            option_dependency(vm, "south", "fetch_elevation");
            option_dependency(vm, "west", "fetch_elevation");
        }

        if(vm.count("fetch_elevation"))
        {
            std::cout << "Downloading elevation file..." << std::endl;
            std::string new_elev = vm["fetch_elevation"].as<std::string>();

            int nSrtmError;
            conflicting_options(vm, "elevation_file", "fetch_elevation");
            option_dependency(vm, "fetch_elevation", "elevation_source");
            std::string source = vm["elevation_source"].as<std::string>();

            SurfaceFetch *fetch = FetchFactory::GetSurfaceFetch( source );
            if( NULL == fetch )
            {
                fprintf(stderr, "Invalid DEM Source\n");
                exit(1);
            }

            if(vm.count("north") || vm.count("south") ||
               vm.count("east") || vm.count("west"))
            {
                option_dependency(vm, "fetch_elevation", "north");
                option_dependency(vm, "fetch_elevation", "east");
                option_dependency(vm, "fetch_elevation", "south");
                option_dependency(vm, "fetch_elevation", "west");

                double north, east, south, west;
                north = vm["north"].as<double>();
                east = vm["east"].as<double>();
                south = vm["south"].as<double>();
                west = vm["west"].as<double>();

                if(south >= north || west >= east)
                {
                    cerr << "Invalid bounding box\n";
                    exit(1);
                }

                double bbox[4];
                bbox[0] = north;
                bbox[1] = east;
                bbox[2] = south;
                bbox[3] = west;

                nSrtmError = fetch->FetchBoundingBox(bbox, 30.0,
                                                     new_elev.c_str(), NULL);

            }
            else
            {
                option_dependency(vm, "fetch_elevation", "x_center");
                option_dependency(vm, "fetch_elevation", "y_center");
                option_dependency(vm, "fetch_elevation", "x_buffer");
                option_dependency(vm, "fetch_elevation", "y_buffer");
                option_dependency(vm, "fetch_elevation", "buffer_units");

                double x, y, x_buf, y_buf;
                std::string b_units;
                x = vm["x_center"].as<double>();
                y = vm["y_center"].as<double>();
                x_buf = vm["x_buffer"].as<double>();
                y_buf = vm["y_buffer"].as<double>();
                b_units = vm["buffer_units"].as<std::string>();

                if(x > 180 || x < -180 || y > 90 || y < -90 || x_buf < 0 ||
                   y_buf < 0 )
                {
                    cerr << "Invalid coordinates for dem\n";
                    exit(1);
                }
                if(b_units != "miles" && b_units != "kilometers")
                {
                    cerr << "Invalid units for buffer for dem\n";
                    exit(1);
                }

                double center[2];
                double buffer[2];
                center[0] = x;
                center[1] = y;
                buffer[0] = x_buf;
                buffer[1] = y_buf;
                lengthUnits::eLengthUnits buffer_units;
                if(b_units == "miles")
                    buffer_units = lengthUnits::miles;
                else
                    buffer_units = lengthUnits::kilometers;

                nSrtmError = fetch->FetchPoint(center, buffer, buffer_units,
                                               30.0, new_elev.c_str(), NULL);
            }

            if(nSrtmError < 0)
            {
                cerr << "Failed to download elevation data\n";
                VSIUnlink(new_elev.c_str());
                exit(1);
            }
            vm.insert(std::make_pair("elevation_file", po::variable_value(vm["fetch_elevation"])));
            po::notify(vm);
            //std::cout << "Elevation file download complete." << std::endl;
        }

        /* Fill no data? */
        int bFillNoData =
            CSLTestBoolean( CPLGetConfigOption( "NINJA_FILL_DEM_NO_DATA",
                                                "NO" ) );
        /* If we downloaded from our fetcher, we fill */
        if( vm.count(" fetch_elevation" ) )
            bFillNoData = TRUE;
        if( bFillNoData )
        {
            GDALDataset *poDS;
            poDS = (GDALDataset*)GDALOpen(vm["elevation_file"].as<std::string>().c_str(), GA_Update);
            if(poDS == NULL)
            {
                throw std::runtime_error("Could not open DEM for reading");
            }
            int nNoDataValues = 0;
            if(GDALHasNoData(poDS, 1))
            {
                nNoDataValues = GDALFillBandNoData(poDS, 1, 100);
            }
            GDALClose((GDALDatasetH)poDS);
            if(nNoDataValues > 0)
            {
                std::cerr << "Could not download valid elevation file, " <<
                            "it contains no data values" << std::endl;
                return 1;
            }
        }

        /* Do we need to detect the timezone? */
        std::string osTimeZone;
        if(vm.count("time_zone"))
        {
            osTimeZone = vm["time_zone"].as<std::string>();
            if(osTimeZone =="auto-detect")
            {
                double p[2];
                GDALDataset *poDS = (GDALDataset*)GDALOpen(vm["elevation_file"].as<std::string>().c_str(), GA_ReadOnly);
                if(poDS == NULL)
                {
                    fprintf(stderr, "Unable to open input DEM\n");
                    return 1;
                }
                GDALGetCenter(poDS, p);
                GDALClose((GDALDatasetH)poDS);
                std::string tz = FetchTimeZone(p[0], p[1], NULL);
                if(tz == "")
                {
                    fprintf(stderr, "Could not detect timezone\n");
                    return 1;
                }
                else{
                    osTimeZone = tz;
                }
            }
        }

        if(vm["initialization_method"].as<std::string>()!=string("domainAverageInitialization") &&
                vm["initialization_method"].as<std::string>() != string("pointInitialization") &&
                vm["initialization_method"].as<std::string>() != string("wxModelInitialization"))
        {
            cout << "'initialization_method' is not a known type.\n";
            cout << "Choices are domainAverageInitialization, pointInitialization, or wxModelInitialization.\n";
            return -1;
        }
        
        //---------------------------------------------------------------------
        //  only some options are possible with momentum solver
        //---------------------------------------------------------------------
        #ifdef NINJAFOAM 
        if(vm["initialization_method"].as<std::string>()!=string("domainAverageInitialization") && 
           vm["momentum_flag"].as<bool>())
        {
            cout << "'initialization_method' must be 'domainAverageInitialization' if the momentum solver is enabled.\n";
            return -1;
        }
        if(vm["momentum_flag"].as<bool>() && vm["diurnal_winds"].as<bool>())
        {
            cout << "Dirunal slope winds option not supported if the momentum solver is enabled.\n";
            return -1;
        }
        #ifdef STABILITY
        if(vm["momentum_flag"].as<bool>() && vm["non_neutral_stability"].as<bool>())
        {
            cout << "Non-neutral stability option not supported if the momentum solver is enabled.\n";
            return -1;
        }
        #endif
        #ifdef FRICTION_VELOCITY
        if(vm["momentum_flag"].as<bool>() && vm["compute_friction_velocity"].as<bool>())
        {
            cout << "Friction velocity calculations not supported if the momentum solver is enabled.\n";
            return -1;
        }
        #endif
        #ifdef EMISSIONS
        if(vm["momentum_flag"].as<bool>() && vm["compute_emissions"].as<bool>())
        {
            cout << "Emission calculations not supported if the momentum solver is enabled.\n";
            return -1;
        }
        #endif
        #ifdef SCALAR
        if(vm["momentum_flag"].as<bool>() && vm["compute_scalar_transport"].as<bool>())
        {
            cout << "Scalar transport calculations not supported if the momentum solver is enabled.\n";
            return -1;
        }
        #endif
        if(vm["momentum_flag"].as<bool>() && vm.count("input_points_file"))
        {
            cout << "Scalar transport calculations not supported if the momentum solver is enabled.\n";
            return -1;
        }
        #endif //NINJAFOAM
        
        if(vm["initialization_method"].as<std::string>() == string("wxModelInitialization"))
        {
            conflicting_options(vm, "wx_model_type", "forecast_filename");
            option_dependency(vm, "wx_model_type", "forecast_duration");
            option_dependency(vm, "wx_model_type", "time_zone");

            if(vm.count("wx_model_type"))   //download forecast and make appropriate size ninjaArmy
            {
                std::string model_type = vm["wx_model_type"].as<std::string>();
                wxModelInitialization *model;
                try
                {
                    model = wxModelInitializationFactory::makeWxInitializationFromId( model_type );
                    windsim.makeArmy( model->fetchForecast( vm["elevation_file"].as<std::string>(),
                                                            vm["forecast_duration"].as<int>() ),
                                                            osTimeZone );
                }
                catch(... )
                {
                    cout << "'wx_model_type' is not valid" << "\n";
                    return -1;
                }
                delete model;
            }
            option_dependency(vm, "forecast_filename", "time_zone");
            if(vm.count("forecast_filename"))   //if a forecast file already exists
            {
                windsim.makeArmy(vm["forecast_filename"].as<std::string>(), osTimeZone);
            }
        }

        /*
        windsim.Com = new ninjaCLIComHandler();
        int r = -1;
        windsim.Com->runNumber = &r;
        char msg[1024];
        windsim.Com->lastMsg = msg;
        */

        //For loop over all ninjas (just 1 ninja unless it's a weather model run)--------------------

        for(int i_ = 0; i_ < windsim.getSize(); i_++)
        {
            //Set ninja communication----------------------------------------------------------
            windsim.setNinjaCommunication(i_, i_, ninjaComClass::ninjaCLICom );

            windsim.setNumberCPUs( i_, vm["num_threads"].as<int>() );

            //windsim.ninjas[i_].readInputFile(vm["elevation_file"].as<std::string>());
            windsim.setDEM( i_, vm["elevation_file"].as<std::string>() );
            windsim.setPosition( i_ );    //get position from DEM file

            //stuff for requested output locations
            if( vm.count("input_points_file") )
            { //optional name for input file
                windsim.setInputPointsFilename( i_,
                        vm["input_points_file"].as<std::string>() );
            }
            if( vm.count("output_points_file") )
            { //optional name for output file
                windsim.setOutputPointsFilename( i_,
                        vm["output_points_file"].as<std::string>() );
            }

            #ifdef FRICTION_VELOCITY
            if(vm["compute_friction_velocity"].as<bool>())
            {
                windsim.setFrictionVelocityFlag( i_, true );
            }
            else
            {
                windsim.setFrictionVelocityFlag( i_, false );
            }
            if( vm.count("friction_velocity_calculation_method") ){
                windsim.setFrictionVelocityCalculationMethod( i_,
                        vm["friction_velocity_calculation_method"].as<std::string>() );
            }
            else{
                windsim.setFrictionVelocityCalculationMethod( i_,
                        "logProfile" );
            }
            #endif

            #ifdef EMISSIONS
            if(vm["compute_emissions"].as<bool>())
            {
                //verify_option_set(vm, "compute_friction_velocity");
                //verify_option_set(vm, "fire_perimeter_file");

                option_dependency(vm, "compute_emissions", "compute_friction_velocity");
                option_dependency(vm, "compute_emissions", "fire_perimeter_file");

                windsim.setDustFlag( i_, true );
                windsim.setDustFilename( i_, vm["fire_perimeter_file"].as<std::string>() );
            }
            else
            {
                windsim.setDustFlag( i_, false );  //default false doensn't work ??
            }
            #endif
                    
            #ifdef NINJAFOAM
            if(vm["momentum_flag"].as<bool>()){
                if(vm.count("number_of_iterations")){
                    windsim.setNumberOfIterations( i_,
                        vm["number_of_iterations"].as<int>() );
                }
                if(vm.count("mesh_count")){
                    windsim.setMeshCount( i_,
                        vm["mesh_count"].as<int>() );
                }
                if(vm["non_equilibrium_boundary_conditions"].as<bool>()){
                    windsim.setNonEqBc( i_,
                        vm["non_equilibrium_boundary_conditions"].as<bool>() );
                }
            }
            #endif

            if(vm["initialization_method"].as<std::string>() == string("domainAverageInitialization"))
            {
                verify_option_set(vm, "input_speed");
                option_dependency(vm, "input_speed", "input_speed_units");
                verify_option_set(vm, "input_direction");
                verify_option_set(vm, "input_wind_height");
                option_dependency(vm, "input_wind_height", "units_input_wind_height");
                option_dependency(vm, "output_wind_height", "units_output_wind_height");

                windsim.setInitializationMethod( i_,
                        WindNinjaInputs::domainAverageInitializationFlag);

                windsim.setInputSpeed( i_, vm["input_speed"].as<double>(),
                        velocityUnits::getUnit( vm["input_speed_units"].as<std::string>() ) );

                windsim.setInputDirection( i_, vm["input_direction"].as<double>() );

                windsim.setInputWindHeight( i_, vm["input_wind_height"].as<double>(),
                        lengthUnits::getUnit(vm["units_input_wind_height"].as<std::string>() ) );

                windsim.setOutputWindHeight( i_, vm["output_wind_height"].as<double>(),
                        lengthUnits::getUnit(vm["units_output_wind_height"].as<std::string>()));

                if(vm["diurnal_winds"].as<bool>())
                {
                    option_dependency(vm, "diurnal_winds", "uni_air_temp");
                    option_dependency(vm, "uni_air_temp", "air_temp_units");
                    option_dependency(vm, "diurnal_winds", "uni_cloud_cover");
                    option_dependency(vm, "uni_cloud_cover", "cloud_cover_units");
                    option_dependency(vm, "diurnal_winds", "year");
                    option_dependency(vm, "diurnal_winds", "month");
                    option_dependency(vm, "diurnal_winds", "day");
                    option_dependency(vm, "diurnal_winds", "hour");
                    option_dependency(vm, "diurnal_winds", "minute");
                    option_dependency(vm, "diurnal_winds", "time_zone");

                    windsim.setDiurnalWinds( i_, true);
                    windsim.setUniAirTemp( i_, vm["uni_air_temp"].as<double>(),
                            temperatureUnits::getUnit(vm["air_temp_units"].as<std::string>())); //for average speed and direction initialization
                    windsim.setUniCloudCover( i_, vm["uni_cloud_cover"].as<double>(),
                            coverUnits::getUnit(vm["cloud_cover_units"].as<std::string>()));
                    windsim.setDateTime( i_, vm["year"].as<int>(), vm["month"].as<int>(),
                                        vm["day"].as<int>(), vm["hour"].as<int>(),
                                        vm["minute"].as<int>(), 0.0,
                                        osTimeZone);
                }
                #ifdef SCALAR
                //scalar transport stuff
                if(vm["compute_scalar_transport"].as<bool>())
                {
                    windsim.setScalarTransportFlag(i_, true);
                    windsim.setScalarSourceStrength(i_, vm["scalar_source_strength"].as<double>());
                    windsim.setScalarXcoord(i_, vm["scalar_source_xlocation"].as<double>());
                    windsim.setScalarYcoord(i_, vm["scalar_source_ylocation"].as<double>());
                }
                #endif //SCALAR

                #ifdef STABILITY
                //Atmospheric stability selections
                if(vm["non_neutral_stability"].as<bool>())
                {
                    if(vm.count("alpha_stability")) //if alpha is specified directly, use that; else, get the info we need to calculate it
                    {
                        if (vm["alpha_stability"].as<double>() > 0 && vm["alpha_stability"].as<double>() <= 5)
                        {
                            windsim.setAlphaStability( i_, vm["alpha_stability"].as<double>());
                            windsim.setStabilityFlag( i_, true);
                        }
                        else
                        {
                            cout << "alpha_stability = " << vm["alpha_stability"].as<double>() << " is not valid.\n";
                            cout << "Valid range for alpha is: 0 < alpha_stability <= 5\n";
                            return -1;
                        }
                    }
                    else{
                        verify_option_set(vm, "uni_cloud_cover");
                        option_dependency(vm, "uni_cloud_cover", "cloud_cover_units");
                        option_dependency(vm, "uni_cloud_cover", "year");
                        option_dependency(vm, "uni_cloud_cover", "month");
                        option_dependency(vm, "uni_cloud_cover", "day");
                        option_dependency(vm, "uni_cloud_cover", "hour");
                        option_dependency(vm, "uni_cloud_cover", "minute");
                        option_dependency(vm, "uni_cloud_cover", "time_zone");

                        windsim.setStabilityFlag( i_, true);
                        windsim.setUniCloudCover( i_, vm["uni_cloud_cover"].as<double>(),
                                                               coverUnits::getUnit(vm["cloud_cover_units"].as<std::string>()));
                        windsim.setDateTime( i_, vm["year"].as<int>(),
                                                               vm["month"].as<int>(),
                                                               vm["day"].as<int>(),
                                                               vm["hour"].as<int>(),
                                                               vm["minute"].as<int>(),
                                                               0.0,
                                                               osTimeZone);
                    }
                }
                #endif //STABILITY
            }else if(vm["initialization_method"].as<std::string>() == string("pointInitialization"))
            {
                verify_option_set(vm, "wx_station_filename");
                option_dependency(vm, "write_wx_station_kml", "wx_station_kml_filename");
                option_dependency(vm, "output_wind_height", "units_output_wind_height");
                windsim.setInitializationMethod( i_,
                        WindNinjaInputs::pointInitializationFlag,
                        vm["match_points"].as<bool>() );
                windsim.setWxStationFilename( i_, vm["wx_station_filename"].as<std::string>() );
                if(vm["write_wx_station_kml"].as<bool>() == true)
                    wxStation::writeKmlFile(windsim.getWxStations( i_ ),
                                            vm["wx_station_kml_filename"].as<std::string>());

                windsim.setOutputWindHeight( i_, vm["output_wind_height"].as<double>(),
                        lengthUnits::getUnit(vm["units_output_wind_height"].as<std::string>()));

                if(vm["diurnal_winds"].as<bool>())
                {
                    option_dependency(vm, "diurnal_winds", "year");
                    option_dependency(vm, "diurnal_winds", "month");
                    option_dependency(vm, "diurnal_winds", "day");
                    option_dependency(vm, "diurnal_winds", "hour");
                    option_dependency(vm, "diurnal_winds", "minute");
                    option_dependency(vm, "diurnal_winds", "time_zone");

                    windsim.setDiurnalWinds( i_, true);
                    windsim.setDateTime( i_, vm["year"].as<int>(), vm["month"].as<int>(),
                                             vm["day"].as<int>(), vm["hour"].as<int>(),
                                             vm["minute"].as<int>(), 0.0,
                                             osTimeZone);
                }
                #ifdef STABILITY
                //Atmospheric stability selections
                if(vm["non_neutral_stability"].as<bool>())
                {
                    if(vm.count("alpha_stability")) //if alpha is specified directly, use that; else, get the info we need to calculate it
                    {
                        if (vm["alpha_stability"].as<double>() > 0 && vm["alpha_stability"].as<double>() <= 5)
                        {
                            windsim.setAlphaStability( i_, vm["alpha_stability"].as<double>());
                            windsim.setStabilityFlag( i_, true);
                        }
                        else
                        {
                            cout << "alpha_stability = " << vm["alpha_stability"].as<double>() << " is not valid.\n";
                            cout << "Valid range for alpha is: 0 < alpha_stability <= 5\n";
                            return -1;
                        }
                    }
                    else
                    {
                        option_dependency(vm, "non_neutral_stability", "year");
                        option_dependency(vm, "non_neutral_stability", "month");
                        option_dependency(vm, "non_neutral_stability", "day");
                        option_dependency(vm, "non_neutral_stability", "hour");
                        option_dependency(vm, "non_neutral_stability", "minute");
                        option_dependency(vm, "non_neutral_stability", "time_zone");

                        windsim.setStabilityFlag( i_, true);
                        windsim.setDateTime( i_, vm["year"].as<int>(), vm["month"].as<int>(),
                                                           vm["day"].as<int>(), vm["hour"].as<int>(),
                                                           vm["minute"].as<int>(), 0.0,
                                                           osTimeZone);
                    }
                }
                #endif //STABILITY

            }else if(vm["initialization_method"].as<std::string>() == string("wxModelInitialization"))
            {
                option_dependency(vm, "output_wind_height", "units_output_wind_height");

                windsim.setOutputWindHeight( i_, vm["output_wind_height"].as<double>(),
                        lengthUnits::getUnit(vm["units_output_wind_height"].as<std::string>()));

                if(vm["diurnal_winds"].as<bool>())
                {
                    windsim.setDiurnalWinds( i_, true );
                }
                #ifdef STABILITY
                if(vm["non_neutral_stability"].as<bool>())
                {
                    //windsim.ninjas[i_].set_stabilityFlag(true);
                    bool wxModel_3d = true;

                    if(vm.count("alpha_stability")) //if alpha is specified directly, use that; else, get the info we need to calculate it
                    {
                        if (vm["alpha_stability"].as<double>() > 0 && vm["alpha_stability"].as<double>() <= 5)
                        {
                            windsim.setStabilityFlag( i_, true );
                            windsim.setAlphaStability( i_, vm["alpha_stability"].as<double>() );
                        }
                        else
                        {
                            cout << "alpha_stability = " << vm["alpha_stability"].as<double>() << " is not valid.\n";
                            cout << "Valid range for alpha is: 0 < alpha_stability <= 5\n";
                            return -1;
                        }
                    }
                    else if(wxModel_3d == true){ //use 3d temperature data from wx model if available
                        windsim.setStabilityFlag( i_, true );
                    }
                }
                #endif
            }

            //check if lcp to determine if surface veg needs to be set or not
            bool isLcp;
            GDALDataset *poDS = (GDALDataset*)GDALOpen(vm["elevation_file"].as<std::string>().c_str(),
                    GA_ReadOnly);
            if(poDS == NULL) {
                printf("Cannot open %s for reading, exiting...", vm["elevation_file"].as<std::string>().c_str());
            }
            const char *pszWkt = poDS->GetProjectionRef();
            if( pszWkt == NULL || EQUAL( pszWkt, "" ) )
            {
                cerr << "DEM does not contain spatial reference information, "
                        "it cannot be used in WindNinja\n";
                return -1;
            }
            OGRSpatialReference oSRS;
            oSRS.importFromWkt((char**)&pszWkt);
            if(oSRS.IsGeographic())
            {
                cerr << "Invalid DEM spatial reference, it is Geographic.\n";
                return -1;
            }

            if(EQUAL(poDS->GetDriver()->GetDescription(), "LCP"))
                isLcp = true;
            else
                isLcp = false;

            GDALClose((GDALDatasetH)poDS);

            if(!isLcp)  //if not an lcp file, we need surface stuff
            {
                verify_option_set(vm, "vegetation");
                if(vm["vegetation"].as<std::string>()!=std::string("grass") && vm["vegetation"].as<std::string>()!=std::string("brush") && vm["vegetation"].as<std::string>()!=std::string("trees"))
                {
                    cout << "'vegetation' type of '" << vm["vegetation"].as<std::string>() << " is not valid.\n";
                    cout << "Choices are: grass, brush, or trees.\n";
                    return -1;
                }
                windsim.setUniVegetation( i_,
                        ninja::get_eVegetationType(vm["vegetation"].as<std::string>()));
            }

            //Mesh resolution selections
            conflicting_options(vm, "mesh_choice", "mesh_resolution");
            if(vm.count("mesh_choice"))
            {
                if( windsim.setMeshResolutionChoice( i_,
                            vm["mesh_choice"].as<std::string>() ) != 0 )
                {
                    cout << "'mesh_choice' of " << vm["mesh_choice"].as<std::string>()
                         << " is not valid.\n" \
                         << "Choices are: coarse, medium, or fine.\n";
                    return -1;
                }
            }
            else if( vm.count("mesh_resolution") )
            {
                option_dependency(vm, "mesh_resolution", "units_mesh_resolution");
                windsim.setMeshResolution( i_, vm["mesh_resolution"].as<double>(), lengthUnits::getUnit(vm["units_mesh_resolution"].as<std::string>()));
            }else{
                cout << "Mesh resolution has not been set.\nUse either 'mesh_choice' or 'mesh_resolution'.\n";
                return -1;
            }
            windsim.setNumVertLayers( i_, 20);

            //output------------------------------------------------------------------------------
            windsim.setOutputBufferClipping( i_, vm["output_buffer_clipping"].as<double>());

            if(!vm.count("write_wx_model_goog_output") &&
                    !vm.count("write_wx_model_shapefile_output") &&
                    !vm.count("write_wx_model_ascii_output") &&
                    !vm.count("write_goog_output") &&
                    !vm.count("write_shapefile_output") &&
                    !vm.count("write_ascii_output") &&
                    !vm.count("write_vtk_output"))
            {
                cout << "No outputs selected.\n";
                return -1;
            }

            windsim.setOutputSpeedUnits( i_,
                    velocityUnits::getUnit(vm["output_speed_units"].as<std::string>()));

            windsim.setWxModelGoogOutFlag( i_, vm["write_wx_model_goog_output"].as<bool>());
            windsim.setWxModelShpOutFlag( i_, vm["write_wx_model_shapefile_output"].as<bool>());
            windsim.setWxModelAsciiOutFlag( i_, vm["write_wx_model_ascii_output"].as<bool>());

            if(vm["write_goog_output"].as<bool>())
            {
                windsim.setGoogOutFlag( i_, true);
                option_dependency(vm, "goog_out_resolution", "units_goog_out_resolution");
                windsim.setGoogResolution( i_, vm["goog_out_resolution"].as<double>(),
                        lengthUnits::getUnit(vm["units_goog_out_resolution"].as<std::string>()));
            }
            if(vm["write_shapefile_output"].as<bool>())
            {
                windsim.setShpOutFlag( i_, true );
                option_dependency(vm, "shape_out_resolution", "units_shape_out_resolution");
                windsim.setShpResolution( i_, vm["shape_out_resolution"].as<double>(),
                        lengthUnits::getUnit(vm["units_shape_out_resolution"].as<std::string>()));
            }
            if(vm["write_ascii_output"].as<bool>())
            {
                windsim.setAsciiOutFlag( i_, true );
                option_dependency(vm, "ascii_out_resolution", "units_ascii_out_resolution");
                windsim.setAsciiResolution( i_, vm["ascii_out_resolution"].as<double>(),
                        lengthUnits::getUnit(vm["units_ascii_out_resolution"].as<std::string>()));
            }
            if(vm["write_vtk_output"].as<bool>())
            {
                windsim.setVtkOutFlag( i_, true );
            }

        }   //end for loop over ninjas

        if(vm["write_farsite_atm"].as<bool>())
        {
            option_dependency(vm, "write_farsite_atm", "write_ascii_output");
            windsim.set_writeFarsiteAtmFile(true);
        }

        //run the simulations
        if(!windsim.startRuns(vm["num_threads"].as<int>()))
        {
            cout << "ERROR: The simulations returned a bad value.\n";
            return -1;
        }
    }
    catch (badForecastFile& e) {   //catch a badForecastFile
        cout << "Exception badForecastFile caught: " << e.what() << "\n";
        cout << "There was a problem downloading the forecast file or it had bad data.\n";
        return -1;
    }catch (bad_alloc& e)
    {
        cout << "Exception bad_alloc caught: " << e.what() << endl;
        cout << "WindNinja appears to have run out of memory." << endl;
        return -1;
    }catch (exception& e)
    {
        cout << "Exception caught: " << e.what() << endl;
        return -1;
    }catch (...)
    {
        cout << "Exception caught: Cannot determine exception type." << endl;
        return -1;
    }

    return 0;
}

/*
//Windninja API Methods

//@brief Sets whether run should write a VTK output file
//@param bool Determine whether to write VTK
//
//@TODO Write combined ascii output method

//@brief Sets whether run should write a VTK output file
//@param bool Determine whether to write VTK
//
void Ninja_WriteVTKOutput(bool choice)
{
    if(initialized!)
    {
        initializeOptions();
    }

    if(choice)
    {
        vm["write_vtk_output"] = true;
        windsim.ninjas[i_].set_vtkOutFlag(true);
    }
}

//@brief Sets whether to make/use? an ATM file
//  @TODO Figure out what this is supposed to do
Ninja_SetFarsiteATMFile(bool choice)
{

}


//@brief Sets whether run should write a Farsite ATM file
// @param bool Determine whether to write ATM

void Ninja_WriteFarsiteATMFile(bool choice)
{
    if(initialized!)
    {
        initializeOptions();
    }

    if(choice)
    {
        vm["write_farsite_atm"] = true;
        option_dependency(vm, "write_farsite_atm", "write_ascii_output");
        windsim.set_writeFarsiteATMFile(true);
    }
}

//@brief Begins a run using number of processors specified in parameter
// @param int Number of processors to use

void Ninja_StartRuns(int numProcessors)
{
    if(initialized!)
    {
        initializeOptions();
    }

    if(!windsim.startRuns(numProcessors);)
    {
        cout << "ERROR: The simulations returned a bad value.\n";
    }
}

//@brief Begins a run using number of processors specified in the virtual map
//
void Ninja_StartRuns()
{
    if(initialized!)
    {
        initializeOptions();
    }

    if(!windsim.startRuns(vm["num_threads"].as<int>()))
    {
        cout << "ERROR: The simulations returned a bad value.\n";
    }
}

//@brief Determines whether to write ASCIIOutput and what units to use and resolution
//@param
//@param
//@param
void Ninja_WriteASCIIOutput(bool choice,char units = 'm', double resolution = 200)
{
    if(initialized!)
    {
        initializeOptions();
    }

    if(initialized)
    {
         windsim.ninjas[i_].set_asciiOutFlag(true);
         option_dependency(vm, "ascii_out_resolution", "units_ascii_out_resolution");
         windsim.ninjas[i_].set_asciiResolution(resolution, units);
    }
}

//@brief Determines whether to write Shapefile output and what units to use and resolution
//@param
//@param
//@param
void Ninja_WriteShapefileOutput(bool choice,char units = 'm', double resolution = 200)
{
    if(initialized!)
    {
        initializeOptions();
    }

    if(initialized)
    {
         windsim.ninjas[i_].set_shpOutFlag(true);
         option_dependency(vm, "shape_out_resolution", "units_shape_out_resolution");
         windsim.ninjas[i_].set_shpResolution(resolution, units);
    }
}

//@brief Determines whether to write Google output and what units to use and resolution
//@param
//@param
//@param
void Ninja_WriteGoogleOutput(bool choice,char units = 'm', double resolution = 200)
{
    if(initialized!)
    {
        initializeOptions();
    }

    if(initialized)
    {
         windsim.ninjas[i_].set_googOutFlag(true);
         option_dependency(vm, "goog_out_resolution", "units_goog_out_resolution");
         windsim.ninjas[i_].set_googResolution(resolution, units);
    }
}

//@brief Determines whether to write weather model Google output
//@param
void Ninja_WriteWXGoogleOutput(bool choice)
{
    if(initialized!)
    {
        initializeOptions();
    }

    if(initialized)
    {
         windsim.ninjas[i_].set_wxModelGoogOutFlag(true);
    }
}

//@brief Determines whether to write weather model Shapefile output
//@param
void Ninja_WriteWXShapefileOutput(bool choice)
{
    if(initialized!)
    {
        initializeOptions();
    }

    if(initialized)
    {
        windsim.ninjas[i_].set_wxModelShpOutFlag(true);
    }
}

//@brief Determines whether to write weather model Shapefile output
//@param
void Ninja_WriteWXASCIIOutput(bool choice)
{
    if(initialized!)
    {
        initializeOptions();
    }

    if(initialized)
    {
        windsim.ninjas[i_].set_wxModelAsciiOutFlag(true);
    }
}
  */



