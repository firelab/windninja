package thredds

import (
	"fmt"
	"io"
	"net/http"
	"net/url"
	"time"
)

const (
	NAM_CONUS  = "UCAR-NAM-CONUS"
	NAM_ALASKA = "UCAR-NAM-ALASKA"
	GFS        = "UCAR-GFS"
)

type model struct {
	path      string
	variables []string
}

var threddsModels = map[string]model{
	NAM_CONUS: model{
		path: "/thredds/ncss/grib/NCEP/NAM/CONUS_12km/best",
		variables: []string{
			"Temperature_height_above_ground",
			"Total_cloud_cover_entire_atmosphere_single_layer",
			"u-component_of_wind_height_above_ground",
			"v-component_of_wind_height_above_ground",
		},
	},
	NAM_ALASKA: model{
		path: "/thredds/ncss/grib/NCEP/NAM/Alaska_11km/best",
		variables: []string{
			"Temperature_height_above_ground",
			"v-component_of_wind_height_above_ground",
			"u-component_of_wind_height_above_ground",
			"Total_cloud_cover_entire_atmosphere_single_layer",
		},
	},
	GFS: model{
		path: "/thredds/ncss/grib/NCEP/GFS/Global_0p5deg/best",
		variables: []string{
			"Temperature_height_above_ground",
			"v-component_of_wind_height_above_ground",
			"u-component_of_wind_height_above_ground",
			"Total_cloud_cover_convective_cloud",
		},
	},
}

var errInvalidModel = fmt.Errorf("invalid wx model")

func Download(w io.Writer, name string, minX, maxX, minY, maxY float64, d time.Duration) error {
	u := url.URL{
		Scheme: "http",
		Host:   "thredds.ucar.edu",
	}
	m, ok := threddsModels[name]
	if !ok {
		return errInvalidModel
	}
	u.Path = m.path
	q := url.Values{}
	q.Set("north", fmt.Sprintf("%f", maxY))
	q.Set("west", fmt.Sprintf("%f", minX))
	q.Set("south", fmt.Sprintf("%f", minY))
	q.Set("east", fmt.Sprintf("%f", maxX))
	q.Set("time_start", "present")
	q.Set("time_duration", fmt.Sprintf("PT%dH", int(d.Hours())))
	q.Set("accept", "netcdf")
	for _, v := range m.variables {
		q.Add("var", v)
	}
	u.RawQuery = q.Encode()
	resp, err := http.Get(u.String())
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	if resp.StatusCode != 200 {
		return fmt.Errorf("failed with %d", resp.StatusCode)
	}
	_, err = io.Copy(w, resp.Body)
	return err
}
