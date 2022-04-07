package main

import (
	"bufio"
	"fmt"
	"log"
	"net/http"
	"strconv"
	"strings"
)

// OGR VRT for proper reading:
/*
<OGRVRTDataSource>
    <OGRVRTLayer name="test">
        <SrcDataSource>test.csv</SrcDataSource>
        <GeometryType>wkbPolygon</GeometryType>
        <LayerSRS>WGS84</LayerSRS>
        <GeometryField encoding="WKT" field="WKT"/>
    </OGRVRTLayer>
</OGRVRTDataSource>
*/

var grids = []string{
	// Grid 212 is the NA
	"g212",
	// Grid 218 is the 12 km NAM grid for CONUS.
	"g218",
	// NAM Alaska 11.25 km
	"g242",
	// HIRES Alaska, East, West
	"alaska.5kmhrw",
	"east.5kmhrw",
	"west.5kmhrw",
}

type cell struct {
	i, j int
	x, y float64
}

func main() {
	fmt.Printf("grid,WKT\n")
	for _, g := range grids {
		log.Printf("generating domain for %s", g)
		fmt.Printf("%s,", g)
		resp, err := http.Get("http://ftp.emc.ncep.noaa.gov/mmb/mmbpll/gridlola.eta/latlon." + g)
		if err != nil {
			log.Fatal(err)
		}
		var cells []cell
		s := bufio.NewScanner(resp.Body)
		for s.Scan() {
			flds := strings.Fields(s.Text())
			if len(flds) < 4 {
				break
			}
			i, err := strconv.Atoi(flds[0])
			if err != nil {
				log.Fatal(err)
			}
			j, err := strconv.Atoi(flds[1])
			if err != nil {
				log.Fatal(err)
			}
			y, err := strconv.ParseFloat(flds[2], 64)
			if err != nil {
				log.Fatal(err)
			}
			x, err := strconv.ParseFloat(flds[3], 64)
			if err != nil {
				log.Fatal(err)
			}
			cells = append(cells, cell{i, j, -x, y})
		}
		fmt.Printf("\"POLYGON((")
		imax := cells[len(cells)-1].i
		jmax := 0
		for _, c := range cells {
			if c.j > jmax {
				jmax = c.j
			}
			if c.j == 1 {
				fmt.Printf("%.3f %.3f,", c.x, c.y)
			} else if c.i == imax {
				fmt.Printf("%.3f %.3f,", c.x, c.y)
			}
		}
		for i := len(cells) - 1; i >= 0; i-- {
			c := cells[i]
			if c.j == jmax {
				fmt.Printf("%.3f %.3f,", c.x, c.y)
			} else if i == 1 {
				fmt.Printf("%.3f %.3f,", c.x, c.y)
			}
		}
		fmt.Printf("%.3f %.3f", cells[0].x, cells[0].y)
		fmt.Printf("\"))\n")
		resp.Body.Close()
	}
}
